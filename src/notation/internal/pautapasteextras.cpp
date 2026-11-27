/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: "complete" copy and paste (see pautapasteextras.h).
 */
#include "pautapasteextras.h"

#include <climits>

#include <QJsonArray>
#include <QJsonDocument>
#include <QSettings>

#include "engraving/dom/factory.h"
#include "engraving/dom/key.h"
#include "engraving/dom/keylist.h"
#include "engraving/dom/keysig.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/select.h"
#include "engraving/dom/spanner.h"
#include "engraving/dom/staff.h"
#include "engraving/dom/timesig.h"
#include "engraving/dom/volta.h"
#include "engraving/dom/chord.h"
#include "engraving/dom/rest.h"
#include "engraving/dom/selectionfilter.h"
#include "engraving/rw/xmlreader.h"
#include "engraving/types/types.h"

#include "log.h"

using namespace mu::engraving;

namespace mu::notation {
static QString optionKey(const char* key)
{
    return QStringLiteral("pauta/pasteWith/") + QString::fromLatin1(key);
}

bool pautaPasteOption(const char* key)
{
    return QSettings().value(optionKey(key), true).toBool();
}

void pautaTogglePasteOption(const char* key)
{
    QSettings s;
    s.setValue(optionKey(key), !pautaPasteOption(key));
    pautaPasteOptionChanged().send(key);
}

muse::async::Channel<std::string>& pautaHintRequested()
{
    static muse::async::Channel<std::string> ch;
    return ch;
}

void pautaHint(const char* key)
{
    pautaHintRequested().send(key);
}

bool pautaFlag(const char* key, bool defaultValue)
{
    return QSettings().value(QStringLiteral("pauta/") + QString::fromLatin1(key), defaultValue).toBool();
}

void pautaToggleFlag(const char* key, bool defaultValue)
{
    QSettings().setValue(QStringLiteral("pauta/") + QString::fromLatin1(key), !pautaFlag(key, defaultValue));
    pautaPasteOptionChanged().send(key);
}

muse::async::Channel<std::string>& pautaPasteOptionChanged()
{
    static muse::async::Channel<std::string> ch;
    return ch;
}

static Measure* measureStartingAt(Score* score, const Fraction& tick)
{
    Measure* m = score->tick2measure(tick);
    return (m && m->tick() == tick) ? m : nullptr;
}

static QString itemData(const EngravingItem* e)
{
    return QString::fromLatin1(e->mimeData().toQByteArray().toBase64());
}

static EngravingItem* itemFromData(Score* score, const QJsonValue& v)
{
    const QByteArray raw = QByteArray::fromBase64(v.toString().toLatin1());
    if (raw.isEmpty()) {
        return nullptr;
    }
    PointF off;
    Fraction dur;
    return EngravingItem::readMimeData(score, muse::ByteArray::fromQByteArray(raw), &off, &dur);
}

//---------------------------------------------------------
//   copy
//---------------------------------------------------------

QByteArray pautaCapturePasteExtras(Score* score)
{
    if (!score || !score->selection().isRange()) {
        return {};
    }
    const Selection& sel = score->selection();

    const Fraction start = sel.tickStart();
    const Fraction end = sel.tickEnd();
    Measure* first = measureStartingAt(score, start);
    if (!first || end <= start) {
        return {}; // the copy does not start at a barline: a normal paste is all we can do
    }
    Measure* lastM = score->tick2measure(end - Fraction::fromTicks(1));
    if (!lastM || lastM->endTick() != end) {
        return {}; // does not end at a barline
    }

    QJsonObject obj;
    obj["v"] = 1;
    obj["len"] = (end - start).ticks();
    obj["staffStart"] = int(sel.staffStart());

    QJsonArray measures;
    QJsonArray items;
    bool irregular = false;
    for (Measure* m = first; m && m->tick() < end; m = m->nextMeasure()) {
        const int rel = (m->tick() - start).ticks();
        QJsonObject jm;
        jm["t"] = rel;
        jm["n"] = m->timesig().numerator();
        jm["d"] = m->timesig().denominator();
        jm["rs"] = m->repeatStart();
        jm["re"] = m->repeatEnd();
        jm["rc"] = m->repeatCount();
        measures.append(jm);
        if (m->ticks() != m->timesig()) {
            irregular = true; // pickup or shortened measure: we don't rewrite time signatures then
        }

        for (EngravingItem* e : m->el()) {
            if (e && (e->isJump() || e->isMarker()) && !e->generated() && e->track() == 0) {
                items.append(QJsonObject { { "kind", "measure" }, { "t", rel }, { "data", itemData(e) } });
            }
        }
        // tempo marks: the normal copy leaves them out (a tempo already at the destination is kept)
        {
            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                for (EngravingItem* e : s->annotations()) {
                    if (e && e->isTempoText() && e->track() == 0 && !e->generated()) {
                        items.append(QJsonObject { { "kind", "tempo" }, { "t", (s->tick() - start).ticks() }, { "data", itemData(e) } });
                    }
                }
            }
        }
    }
    obj["measures"] = measures;
    obj["items"] = items;
    obj["irregular"] = irregular;

    // key signatures per copied staff: the key at the start and every change inside
    QJsonArray keys;
    for (staff_idx_t si = sel.staffStart(); si < sel.staffEnd(); ++si) {
        Staff* st = score->staff(si);
        QJsonArray ks;
        auto addKey = [&](int rel, const KeySigEvent& ev) {
            if (ev.custom() || !ev.isValid()) {
                return;
            }
            ks.append(QJsonObject { { "t", rel }, { "k", int(ev.concertKey()) }, { "m", int(ev.mode()) } });
        };
        addKey(0, st->keySigEvent(start));
        for (const auto& [tick, ev] : *st->keyList()) {
            if (tick > start.ticks() && tick < end.ticks()) {
                addKey(tick - start.ticks(), ev);
            }
        }
        keys.append(ks);
    }
    obj["keys"] = keys;

    // voltas (the normal copy leaves them out)
    QJsonArray voltas;
    auto sl = score->spannerMap().findOverlapping(start.ticks(), end.ticks());
    for (auto i : sl) {
        Spanner* s = i.value;
        if (!s->isVolta() || s->track() != 0 || s->generated() || s->tick() < start || s->tick2() > end) {
            continue;
        }
        Volta* v = toVolta(s);
        QJsonArray en;
        for (int n : v->endings()) {
            en.append(n);
        }
        voltas.append(QJsonObject { { "t", (v->tick() - start).ticks() }, { "t2", (v->tick2() - start).ticks() },
                          { "e", en }, { "txt", v->beginText().toQString() },
                          { "open", v->voltaType() == Volta::Type::OPEN } });
    }
    obj["voltas"] = voltas;

    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

bool PautaPasteExtras::load(const QByteArray& data)
{
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject() || doc.object().value("v").toInt() != 1) {
        return false;
    }
    m_obj = doc.object();
    return true;
}

static bool sameKey(const KeySigEvent& ev, int k, int m)
{
    return !ev.custom() && int(ev.concertKey()) == k && int(ev.mode()) == m;
}

// the destination measure that comes right after the N measures the passage replaces (its tick; or the score end)
static Fraction destEndTick(Score* score, const Fraction& destTick, int count)
{
    Measure* m = measureStartingAt(score, destTick);
    for (int i = 0; m && i < count; ++i) {
        m = m->nextMeasure();
    }
    return m ? m->tick() : score->endTick();
}

void PautaPasteExtras::prepare(Score* score, const Fraction& destTick, staff_idx_t dstStaff)
{
    m_destEndKeys.clear();
    const int count = int(m_obj.value("measures").toArray().size());
    const Fraction endTick = destEndTick(score, destTick, count);
    const QJsonArray keys = m_obj.value("keys").toArray();
    for (int i = 0; i < keys.size(); ++i) {
        const staff_idx_t si = dstStaff + i;
        if (si >= score->nstaves() || endTick >= score->endTick()) {
            m_destEndKeys.push_back({ INT_MIN, 0 });
            continue;
        }
        const KeySigEvent ev = score->staff(si)->keySigEvent(endTick);
        if (ev.custom() || !ev.isValid()) {
            m_destEndKeys.push_back({ INT_MIN, 0 });
        } else {
            m_destEndKeys.push_back({ int(ev.concertKey()), int(ev.mode()) });
        }
    }
}

static TimeSig* timeSigAt(Measure* m)
{
    Segment* seg = m->findSegment(SegmentType::TimeSig, m->tick());
    EngravingItem* e = seg ? seg->element(0) : nullptr;
    return (e && e->isTimeSig() && !e->generated()) ? toTimeSig(e) : nullptr;
}

static void addTimeSig(Score* score, Measure* m, const Fraction& sig)
{
    TimeSig* ts = Factory::createTimeSig(score->dummy()->segment());
    ts->setSig(sig, TimeSigType::NORMAL);
    score->cmdAddTimeSig(m, 0, ts, false); // takes ownership
}

// The copied passage has N measures; it replaces the N destination measures, measure by measure.
// When their time signatures differ, the destination measures are rebuilt with the passage's ones, and the
// measure after them keeps its own time signature, so nothing after the paste moves across a barline.
bool PautaPasteExtras::applyBefore(Score* score, const Fraction& destTick)
{
    if (!pautaPasteOption("timeSig") || m_obj.value("irregular").toBool()) {
        return false;
    }
    const QJsonArray measures = m_obj.value("measures").toArray();
    const int count = int(measures.size());
    if (count == 0 || !measureStartingAt(score, destTick)) {
        return false;
    }
    std::vector<Fraction> sigs;
    for (const QJsonValue& v : measures) {
        sigs.push_back(Fraction(v.toObject().value("n").toInt(), v.toObject().value("d").toInt()));
    }

    auto measureAt = [&](int k) -> Measure* {
        Measure* m = measureStartingAt(score, destTick);
        for (int i = 0; m && i < k; ++i) {
            m = m->nextMeasure();
        }
        return m;
    };

    // enough destination measures
    bool changed = false;
    while (!measureAt(count - 1)) {
        score->appendMeasures(1);
        changed = true;
    }

    // nothing more to do if every measure already has the right time signature
    bool same = true;
    for (int k = 0; k < count; ++k) {
        Measure* m = measureAt(k);
        same = same && m->timesig() == sigs[k] && m->ticks() == sigs[k];
    }
    if (same) {
        return changed;
    }

    // the measure after the passage gets (or keeps) a time signature: rewriting always stops there.
    // Rewrites never touch that measure, so its TimeSig stays the same object and marks the spot.
    Measure* after = measureAt(count - 1)->nextMeasure();
    const Fraction afterSig = after ? after->timesig() : Fraction();
    bool barrierAdded = false;
    TimeSig* barrierTs = nullptr;
    if (after) {
        const Fraction afterTick = after->tick();
        if (!timeSigAt(after)) {
            addTimeSig(score, after, afterSig);
            barrierAdded = true;
        }
        Measure* b = measureStartingAt(score, afterTick);
        barrierTs = b ? timeSigAt(b) : nullptr;
        if (!barrierTs) {
            return true;
        }
    }
    auto isBarrier = [&](Measure* m) { return barrierTs && timeSigAt(m) == barrierTs; };

    // time signature changes inside the destination measures go away first
    for (int guard = 0; guard < 1000; ++guard) {
        TimeSig* inner = nullptr;
        Measure* first = measureStartingAt(score, destTick);
        for (Measure* m = first ? first->nextMeasure() : nullptr; m && !isBarrier(m); m = m->nextMeasure()) {
            if ((inner = timeSigAt(m))) {
                break;
            }
        }
        if (!inner) {
            break;
        }
        score->cmdRemoveTimeSig(inner);
    }

    // rebuild: each change rewrites from that measure up to the barrier
    Fraction tick = destTick;
    for (int k = 0; k < count; ++k) {
        Measure* m = measureStartingAt(score, tick);
        if (!m || isBarrier(m)) {
            if (m) {
                score->insertMeasure(ElementType::MEASURE, m);
            } else {
                score->appendMeasures(1);
            }
            m = measureStartingAt(score, tick);
            if (!m) {
                LOGW() << "Pauta paste: could not add a measure at " << tick.ticks();
                return true;
            }
        }
        if (m->timesig() != sigs[k] || m->ticks() != sigs[k]) {
            addTimeSig(score, m, sigs[k]);
        }
        tick += sigs[k];
    }

    // measures left over between the passage and the barrier (only rests from the rewrite) go away
    Measure* firstLeft = measureStartingAt(score, tick);
    if (firstLeft && !isBarrier(firstLeft)) {
        Measure* lastLeft = firstLeft;
        while (lastLeft->nextMeasure() && !isBarrier(lastLeft->nextMeasure())) {
            lastLeft = lastLeft->nextMeasure();
        }
        score->deleteMeasures(firstLeft, lastLeft);
    }

    // a time signature we only added to hold the barline, and that now repeats the one before it
    if (barrierAdded && barrierTs && sigs.back() == afterSig) {
        Measure* b = measureStartingAt(score, tick);
        if (b && isBarrier(b)) {
            score->cmdRemoveTimeSig(barrierTs);
        }
    }
    return true;
}

void PautaPasteExtras::applyAfter(Score* score, const Fraction& destTick, staff_idx_t dstStaff)
{
    const Fraction len = Fraction::fromTicks(m_obj.value("len").toInt());
    const Fraction endTick = destTick + len;
    if (!measureStartingAt(score, destTick)) {
        return;
    }

    // key signatures
    if (pautaPasteOption("keySig")) {
        const QJsonArray keys = m_obj.value("keys").toArray();
        for (int i = 0; i < keys.size(); ++i) {
            const staff_idx_t si = dstStaff + i;
            if (si >= score->nstaves()) {
                break;
            }
            Staff* st = score->staff(si);
            if (st->isDrumStaff(destTick)) {
                continue;
            }
            const QJsonArray ks = keys.at(i).toArray();

            // key changes inside the destination that the copied passage doesn't have go away
            for (Measure* m = score->tick2measure(destTick); m && m->tick() < endTick; m = m->nextMeasure()) {
                if (m->tick() == destTick) {
                    continue;
                }
                bool inSource = false;
                for (const QJsonValue& kv : ks) {
                    inSource |= destTick + Fraction::fromTicks(kv.toObject().value("t").toInt()) == m->tick();
                }
                Segment* seg = m->findSegment(SegmentType::KeySig, m->tick());
                EngravingItem* e = seg ? seg->element(si * VOICES) : nullptr;
                if (!inSource && e && e->isKeySig() && !e->generated()) {
                    score->undoRemoveElement(e);
                }
            }

            for (const QJsonValue& kv : ks) {
                const QJsonObject jk = kv.toObject();
                const Fraction tick = destTick + Fraction::fromTicks(jk.value("t").toInt());
                if (!measureStartingAt(score, tick)) {
                    continue;
                }
                const int k = jk.value("k").toInt();
                const int mode = jk.value("m").toInt();
                if (!sameKey(st->keySigEvent(tick), k, mode)) {
                    KeySigEvent ev;
                    ev.setConcertKey(Key(k));
                    ev.setMode(KeyMode(mode));
                    score->undoChangeKeySig(st, tick, ev);
                }
            }
            // the destination's key comes back after the passage
            if (i < int(m_destEndKeys.size()) && m_destEndKeys[i].first != INT_MIN && measureStartingAt(score, endTick)) {
                const auto [k, mode] = m_destEndKeys[i];
                if (!sameKey(st->keySigEvent(endTick), k, mode)) {
                    KeySigEvent ev;
                    ev.setConcertKey(Key(k));
                    ev.setMode(KeyMode(mode));
                    score->undoChangeKeySig(st, endTick, ev);
                }
            }
        }
    }

    // repeat barlines, voltas, jumps and markers: the copied measures replace the destination's
    if (pautaPasteOption("repeats")) {
        for (const QJsonValue& v : m_obj.value("measures").toArray()) {
            const QJsonObject jm = v.toObject();
            Measure* m = measureStartingAt(score, destTick + Fraction::fromTicks(jm.value("t").toInt()));
            if (!m || m->tick() >= endTick) {
                continue;
            }
            if (m->repeatStart() != jm.value("rs").toBool()) {
                m->undoChangeProperty(Pid::REPEAT_START, jm.value("rs").toBool());
            }
            if (m->repeatEnd() != jm.value("re").toBool()) {
                m->undoChangeProperty(Pid::REPEAT_END, jm.value("re").toBool());
            }
            if (m->repeatCount() != jm.value("rc").toInt()) {
                m->undoChangeProperty(Pid::REPEAT_COUNT, jm.value("rc").toInt());
            }
        }

        // old jumps/markers and voltas in the destination go away
        for (Measure* m = score->tick2measure(destTick); m && m->tick() < endTick; m = m->nextMeasure()) {
            std::vector<EngravingItem*> old;
            for (EngravingItem* e : m->el()) {
                if (e && (e->isJump() || e->isMarker()) && !e->generated() && e->track() == 0) {
                    old.push_back(e);
                }
            }
            for (EngravingItem* e : old) {
                score->undoRemoveElement(e);
            }
        }
        std::vector<Spanner*> oldVoltas;
        for (auto i : score->spannerMap().findContained(destTick.ticks(), endTick.ticks())) {
            if (i.value->isVolta() && i.value->track() == 0 && !i.value->generated()) {
                oldVoltas.push_back(i.value);
            }
        }
        for (Spanner* s : oldVoltas) {
            score->undoRemoveElement(s);
        }

        for (const QJsonValue& v : m_obj.value("items").toArray()) {
            const QJsonObject ji = v.toObject();
            if (ji.value("kind").toString() != "measure") {
                continue;
            }
            const Fraction tick = destTick + Fraction::fromTicks(ji.value("t").toInt());
            Measure* m = measureStartingAt(score, tick);
            EngravingItem* e = m ? itemFromData(score, ji.value("data")) : nullptr;
            if (!e) {
                continue;
            }
            e->setParent(m);
            e->setTrack(0);
            score->undoAddElement(e);
        }

        for (const QJsonValue& v : m_obj.value("voltas").toArray()) {
            const QJsonObject jv = v.toObject();
            Volta* volta = Factory::createVolta(score->dummy());
            volta->setTrack(0);
            volta->setTick(destTick + Fraction::fromTicks(jv.value("t").toInt()));
            volta->setTick2(destTick + Fraction::fromTicks(jv.value("t2").toInt()));
            std::vector<int> en;
            for (const QJsonValue& n : jv.value("e").toArray()) {
                en.push_back(n.toInt());
            }
            volta->setEndings(en);
            volta->setText(String::fromQString(jv.value("txt").toString()));
            volta->setVoltaType(jv.value("open").toBool() ? Volta::Type::OPEN : Volta::Type::CLOSED);
            score->undoAddElement(volta);
        }
    }

    // tempo marks (only those the normal copy left out)
    if (pautaPasteOption("tempo")) {
        for (const QJsonValue& v : m_obj.value("items").toArray()) {
            const QJsonObject ji = v.toObject();
            if (ji.value("kind").toString() != "tempo") {
                continue;
            }
            const Fraction tick = destTick + Fraction::fromTicks(ji.value("t").toInt());
            Measure* m = score->tick2measure(tick);
            if (!m || tick >= endTick) {
                continue;
            }
            Segment* s = m->undoGetSegment(SegmentType::ChordRest, tick);
            bool hasTempo = false;
            for (EngravingItem* a : s->annotations()) {
                hasTempo |= a->isTempoText();
            }
            if (hasTempo) {
                continue;
            }
            EngravingItem* e = itemFromData(score, ji.value("data"));
            if (!e) {
                continue;
            }
            e->setParent(s);
            e->setTrack(0);
            score->undoAddElement(e);
        }
    }
}

//---------------------------------------------------------
//   duration change that pushes the rest of the voice
//---------------------------------------------------------

static void selectVoiceOnly(Score* score, voice_idx_t voice)
{
    for (voice_idx_t v = 0; v < VOICES; ++v) {
        score->selectionFilter().setFiltered(static_cast<VoicesSelectionFilterTypes>(1u << v), v == voice);
    }
}

static Segment* crSegmentAt(Score* score, const Fraction& tick)
{
    Measure* m = score->tick2measure(tick);
    return m ? m->undoGetSegment(SegmentType::ChordRest, tick) : nullptr;
}

static bool selectVoiceRange(Score* score, const Fraction& from, const Fraction& to, staff_idx_t staff)
{
    // setRangeTicks also replaces the range a paste leaves planned (setRange would be overridden by it)
    score->deselectAll();
    score->selection().setRangeTicks(from, to, staff, staff + 1);
    score->selection().updateSelectedElements();
    const Segment* s1 = score->selection().startSegment();
    return score->selection().isRange() && s1 && s1->tick() == from;
}

bool pautaRippleDuration(Score* score, ChordRest* cr, const TDuration& newDuration)
{
    if (!score || !cr || cr->tuplet() || cr->isMeasureRepeat() || cr->isMMRest() || !newDuration.isValid()) {
        return false;
    }
    if (cr->isRest() && toRest(cr)->isFullMeasureRest()) {
        return false;
    }
    const staff_idx_t staff = cr->staffIdx();
    if (cr->staff()->timeStretch(cr->tick()) != Fraction(1, 1)) {
        return false;
    }
    const track_idx_t track = cr->track();
    const voice_idx_t voice = cr->voice();
    const Fraction start = cr->tick();
    const Fraction oldLen = cr->ticks();
    const Fraction newLen = newDuration.fraction();
    if (oldLen == newLen) {
        return true;
    }
    const Fraction delta = newLen - oldLen;
    const Fraction oldEnd = start + oldLen;
    const Fraction newEnd = start + newLen;
    const Fraction last = score->lastMeasure()->endTick();

    // what follows in this voice moves by delta; when it grows, the end of the score makes room if notes would fall off
    Fraction moveEnd = last;
    if (delta > Fraction(0, 1)) {
        bool notesAtEnd = false;
        for (Segment* s = score->tick2segment(last - delta, true, SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
            EngravingItem* e = s->element(track);
            if (e && e->isChord()) {
                notesAtEnd = true;
                break;
            }
        }
        // a chord that starts before the cut but lasts into it also needs room
        if (ChordRest* across = score->findCR(last - delta, track)) {
            notesAtEnd |= across->isChord() && across->tick() + across->ticks() > last - delta;
        }
        if (notesAtEnd) {
            while (score->lastMeasure()->endTick() < last + delta) {
                score->appendMeasures(1);
            }
        } else {
            moveEnd = last - delta;
        }
    }

    const SelectionFilter savedFilter = score->selectionFilter();
    QByteArray clip;
    if (oldEnd < moveEnd) {
        selectVoiceOnly(score, voice);
        const bool ok = selectVoiceRange(score, oldEnd, moveEnd, staff) && score->selection().canCopy();
        if (ok) {
            clip = score->selection().mimeData().toQByteArray(); // a range gives the staff list
        }
        score->selectionFilter() = savedFilter;
        score->deselectAll();
        if (!ok) {
            return false; // a tuplet or something else that can't be cut here
        }
    }

    score->changeCRlen(cr, newDuration);

    if (!clip.isEmpty()) {
        Segment* dst = crSegmentAt(score, newEnd);
        if (!dst) {
            return true;
        }
        XmlReader e(muse::ByteArray::fromQByteArray(clip));
        score->pasteStaff(e, dst, staff, Fraction(1, 1));
    }

    // it got shorter: the old ending of this voice, now repeated at the end, becomes rest
    if (delta < Fraction(0, 1)) {
        const Fraction from = last + delta;
        if (from > newEnd) {
            selectVoiceOnly(score, voice);
            if (selectVoiceRange(score, from, last, staff)) {
                score->cmdDeleteSelection();
            }
            score->selectionFilter() = savedFilter;
        }
    }

    score->deselectAll();
    if (ChordRest* changed = score->findCR(start, track)) {
        score->select(changed->isChord() ? static_cast<EngravingItem*>(toChord(changed)->upNote()) : changed, SelectType::SINGLE, 0);
    }
    return true;
}
}
