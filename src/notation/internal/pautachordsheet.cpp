/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: "chord sheet" export (see pautachordsheet.h).
 */
#include "pautachordsheet.h"

#include <map>

#include <QFontDatabase>
#include <QPainter>
#include <QPdfWriter>
#include <QPageSize>
#include <QRegularExpression>

#include "engraving/dom/chordrest.h"
#include "engraving/dom/harmony.h"
#include "engraving/dom/lyrics.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/system.h"

using namespace mu::engraving;
using namespace Qt::StringLiterals;

namespace mu::notation {
namespace {
QString harmonyText(const Harmony* h)
{
    QString s = h->plainText().toQString().trimmed();
    if (s.isEmpty()) {
        s = h->harmonyName().toQString().trimmed();
    }
    return s;
}

// the track (staff + voice) with the most first-verse syllables
track_idx_t findLyricsTrack(Score* score, bool& found)
{
    std::map<track_idx_t, int> count;
    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            for (track_idx_t t = 0; t < score->ntracks(); ++t) {
                const EngravingItem* e = s->element(t);
                if (!e || !e->isChordRest()) {
                    continue;
                }
                for (const Lyrics* l : toChordRest(e)->lyrics()) {
                    if (l->verse() == 0 && !l->plainText().isEmpty()) {
                        ++count[t];
                    }
                }
            }
        }
    }
    found = !count.empty();
    track_idx_t best = 0;
    int bestCount = -1;
    for (const auto& [t, c] : count) {
        if (c > bestCount) {
            best = t;
            bestCount = c;
        }
    }
    return best;
}

const Lyrics* lyricOf(const ChordRest* cr, int verse)
{
    for (const Lyrics* l : cr->lyrics()) {
        if (l->verse() == verse) {
            return l;
        }
    }
    return nullptr;
}

void appendSyllable(QString& line, const Lyrics* l)
{
    QString text = l->plainText().toQString();
    line += text;
    const LyricsSyllabic syl = l->syllabic();
    if (syl == LyricsSyllabic::SINGLE || syl == LyricsSyllabic::END) {
        line += u' ';
    }
}

// chords of one segment (all staves; the same chord on several staves is written once)
QStringList chordsAt(const Segment* s)
{
    QStringList out;
    for (const EngravingItem* e : s->annotations()) {
        if (e && e->isHarmony()) {
            const QString t = harmonyText(toHarmony(e));
            if (!t.isEmpty() && !out.contains(t)) {
                out << t;
            }
        }
    }
    return out;
}

void putChord(QString& chordLine, int col, const QString& chord)
{
    if (!chordLine.isEmpty() && col <= chordLine.size()) {
        col = chordLine.size() + 1; // never glue two chords together
    }
    if (chordLine.size() < col) {
        chordLine += QString(col - chordLine.size(), u' ');
    }
    chordLine += chord;
}
}

QString PautaChordSheet::toText() const
{
    QString out;
    if (!title.isEmpty()) {
        out += title + u"\n\n"_s;
    }
    for (const PautaChordSheetLine& l : lines) {
        if (!l.chords.isEmpty()) {
            out += l.chords + u'\n';
        }
        if (!l.lyrics.isEmpty()) {
            out += l.lyrics + u'\n';
        }
        out += u'\n';
    }
    for (const QString& v : otherVerses) {
        out += v + u"\n\n"_s;
    }
    return out.trimmed() + u'\n';
}

PautaChordSheet pautaMakeChordSheet(Score* score)
{
    PautaChordSheet sheet;
    if (!score) {
        return sheet;
    }

    sheet.title = score->metaTag(u"workTitle").toQString().trimmed();
    if (sheet.title.isEmpty()) {
        sheet.title = score->name().toQString();
    }

    bool hasLyrics = false;
    const track_idx_t lyricTrack = findLyricsTrack(score, hasLyrics);

    int maxVerse = 0;
    std::map<int, QStringList> verseLines;

    // one line of the sheet per line of music; very long lines (continuous view) are cut every 4 measures
    std::vector<std::vector<const Measure*> > groups;
    for (const System* sys : score->systems()) {
        std::vector<const Measure*> ms;
        for (const MeasureBase* mb : sys->measures()) {
            if (mb->isMeasure()) {
                ms.push_back(toMeasure(mb));
            }
        }
        if (ms.size() > 8) {
            for (size_t i = 0; i < ms.size(); i += 4) {
                groups.emplace_back(ms.begin() + i, ms.begin() + std::min(ms.size(), i + 4));
            }
        } else if (!ms.empty()) {
            groups.push_back(ms);
        }
    }

    for (const std::vector<const Measure*>& group : groups) {
        PautaChordSheetLine line;
        std::map<int, QString> otherVerseLine;
        bool anyMeasure = false;

        for (const Measure* m : group) {
            anyMeasure = true;

            if (!hasLyrics) {
                // no words: one bar per measure, chords in between
                QStringList inMeasure;
                for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                    inMeasure << chordsAt(s);
                }
                line.chords += line.chords.isEmpty() ? u"| "_s : u" "_s;
                line.chords += inMeasure.isEmpty() ? u"%"_s : inMeasure.join(u"  "_s);
                line.chords += u" |"_s;
                continue;
            }

            for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                const int col = line.lyrics.size();
                for (const QString& c : chordsAt(s)) {
                    putChord(line.chords, col, c);
                }
                const EngravingItem* e = s->element(lyricTrack);
                if (!e || !e->isChordRest()) {
                    continue;
                }
                const ChordRest* cr = toChordRest(e);
                if (const Lyrics* l = lyricOf(cr, 0)) {
                    appendSyllable(line.lyrics, l);
                }
                for (const Lyrics* l : cr->lyrics()) {
                    if (l->verse() > 0) {
                        maxVerse = std::max(maxVerse, l->verse());
                        appendSyllable(otherVerseLine[l->verse()], l);
                    }
                }
            }
        }

        if (!anyMeasure) {
            continue;
        }
        static const QRegularExpression trailingSpace(u"\\s+$"_s);
        line.chords = line.chords.trimmed().isEmpty() ? QString() : QString(line.chords).remove(trailingSpace);
        line.lyrics = line.lyrics.trimmed().isEmpty() ? QString() : QString(line.lyrics).remove(trailingSpace);
        if (!line.chords.isEmpty() || !line.lyrics.isEmpty()) {
            sheet.lines.push_back(line);
        }
        for (int v = 1; v <= maxVerse; ++v) {
            const QString t = otherVerseLine[v].trimmed();
            if (!t.isEmpty()) {
                verseLines[v] << t;
            }
        }
    }

    for (int v = 1; v <= maxVerse; ++v) {
        if (!verseLines[v].isEmpty()) {
            sheet.otherVerses.push_back(QString::number(v + 1) + u".\n"_s + verseLines[v].join(u'\n'));
        }
    }
    return sheet;
}

bool pautaWriteChordSheetPdf(const PautaChordSheet& sheet, const QString& path)
{
    QPdfWriter pdf(path);
    pdf.setPageSize(QPageSize(QPageSize::A4));
    pdf.setResolution(300);
    pdf.setPageMargins(QMarginsF(18, 18, 18, 18), QPageLayout::Millimeter);
    pdf.setTitle(sheet.title);
    pdf.setCreator(u"Pauta"_s);

    QPainter p;
    if (!p.begin(&pdf)) {
        return false;
    }

    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setPointSizeF(11);
    QFont chordFont = mono;
    chordFont.setBold(true);
    QFont titleFont(u"Helvetica"_s);
    titleFont.setPointSizeF(20);
    titleFont.setBold(true);

    const QColor chordColor(0xD9, 0x4F, 0x1E); // Pauta orange, dark enough to print
    const QRect page = p.viewport();
    const int pageBottom = page.height();
    int y = 0;

    auto lineHeight = [&p](const QFont& f) {
        return QFontMetrics(f, p.device()).lineSpacing();
    };
    auto ensureRoom = [&](int needed) {
        if (y + needed > pageBottom) {
            pdf.newPage();
            y = 0;
        }
    };
    auto drawLine = [&](const QString& text, const QFont& f, const QColor& c) {
        const int h = lineHeight(f);
        p.setFont(f);
        p.setPen(c);
        p.drawText(QRect(0, y, page.width(), h), Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip, text);
        y += h;
    };

    if (!sheet.title.isEmpty()) {
        drawLine(sheet.title, titleFont, Qt::black);
        y += lineHeight(mono);
    }

    const int gap = lineHeight(mono) / 2;
    for (const PautaChordSheetLine& l : sheet.lines) {
        const int needed = (l.chords.isEmpty() ? 0 : lineHeight(chordFont)) + (l.lyrics.isEmpty() ? 0 : lineHeight(mono));
        ensureRoom(needed);
        if (!l.chords.isEmpty()) {
            drawLine(l.chords, chordFont, chordColor);
        }
        if (!l.lyrics.isEmpty()) {
            drawLine(l.lyrics, mono, Qt::black);
        }
        y += gap;
    }

    for (const QString& v : sheet.otherVerses) {
        y += gap;
        for (const QString& t : v.split(u'\n')) {
            ensureRoom(lineHeight(mono));
            drawLine(t, mono, Qt::black);
        }
    }

    p.end();
    return true;
}
}
