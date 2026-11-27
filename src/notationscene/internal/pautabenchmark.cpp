/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: speed test (see pautabenchmark.h).
 */
#include "pautabenchmark.h"

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>

#include <QElapsedTimer>
#include <QFile>
#include <QGuiApplication>
#include <QQuickWindow>
#include <QTextStream>
#include <QTimer>
#include <QDebug>

#include "engraving/dom/chord.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/score.h"
#include "engraving/dom/segment.h"
#include "engraving/dom/page.h"
#include "engraving/dom/accidental.h"
#include "engraving/dom/textbase.h"

using namespace mu::engraving;
using namespace muse::actions;

namespace mu::notation {
namespace {
struct Step {
    QString name;
    std::function<void()> prepare;   // not timed
    ActionCode action;               // timed
    std::function<void()> cleanup;   // not timed
};

struct Bench : public std::enable_shared_from_this<Bench> {
    INotationPtr notation;
    IActionsDispatcher* dispatcher = nullptr;
    QString outPath;
    std::vector<Step> steps;
    int rounds = 5;
    size_t stepIdx = 0;
    int round = 0;
    QStringList lines;
    std::vector<double> syncMs, frameMs;

    QElapsedTimer clock;
    std::atomic<qint64> lastFrameNs { 0 };
    QQuickWindow* window = nullptr;

    Score* score() const { return notation->elements()->msScore(); }

    Measure* measureAt(int idx) const
    {
        Measure* m = score()->firstMeasure();
        for (int i = 0; m && i < idx; ++i) {
            m = m->nextMeasure();
        }
        return m;
    }

    int measureCount() const
    {
        int n = 0;
        for (Measure* m = score()->firstMeasure(); m; m = m->nextMeasure()) {
            ++n;
        }
        return n;
    }

    Note* noteAt(int measureIdx, staff_idx_t staff) const
    {
        Measure* m = measureAt(measureIdx);
        if (!m) {
            return nullptr;
        }
        for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
            EngravingItem* e = s->element(staff * VOICES);
            if (e && e->isChord()) {
                return toChord(e)->upNote();
            }
        }
        return nullptr;
    }

    void selectNote(int measureIdx, staff_idx_t staff)
    {
        if (Note* n = noteAt(measureIdx, staff)) {
            notation->interaction()->select({ n }, SelectType::SINGLE, 0);
        }
    }

    void selectMeasures(int first, int last)
    {
        Measure* a = measureAt(first);
        Measure* b = measureAt(last);
        if (!a || !b) {
            return;
        }
        notation->interaction()->clearSelection();
        notation->interaction()->select({ a }, SelectType::RANGE, 0);
        notation->interaction()->select({ b }, SelectType::RANGE, score()->nstaves() - 1);
    }

    void run(const ActionCode& code) { dispatcher->dispatch(code); }

    void findWindow()
    {
        int best = 0;
        for (QWindow* w : QGuiApplication::topLevelWindows()) {
            auto* qw = qobject_cast<QQuickWindow*>(w);
            if (qw && qw->isVisible() && qw->width() * qw->height() > best) {
                best = qw->width() * qw->height();
                window = qw;
            }
        }
        if (window) {
            std::weak_ptr<Bench> weak = shared_from_this();
            QObject::connect(window, &QQuickWindow::frameSwapped, window, [weak]() {
                if (auto self = weak.lock()) {
                    self->lastFrameNs = self->clock.nsecsElapsed();
                }
            }, Qt::DirectConnection);
        }
    }

    void next(int delayMs)
    {
        std::weak_ptr<Bench> weak = shared_from_this();
        QTimer::singleShot(delayMs, [weak]() {
            if (auto self = weak.lock()) {
                self->doRun();
            }
        });
    }

    void doRun()
    {
        qWarning() << "PAUTA_BENCH step" << stepIdx << "round" << round;
        if (stepIdx >= steps.size()) {
            finish();
            return;
        }
        Step& st = steps[stepIdx];
        if (st.prepare) {
            st.prepare();
        }
        // let the selection settle and be drawn before timing
        std::weak_ptr<Bench> weak = shared_from_this();
        QTimer::singleShot(250, [weak]() {
            if (auto self = weak.lock()) {
                self->timeOne();
            }
        });
    }

    void timeOne()
    {
        Step& st = steps[stepIdx];
        const qint64 t0 = clock.nsecsElapsed();
        lastFrameNs = 0;
        run(st.action);
        const qint64 t1 = clock.nsecsElapsed();
        syncMs.push_back((t1 - t0) / 1e6);

        // wait for the next frame on screen (or give up after 3 s, e.g. when the screen is locked)
        auto self = shared_from_this();
        auto poll = std::make_shared<QTimer>();
        poll->setInterval(1);
        QObject::connect(poll.get(), &QTimer::timeout, [self, poll, t0, t1]() {
            const qint64 f = self->lastFrameNs.load();
            const qint64 now = self->clock.nsecsElapsed();
            if (f > t1 || now - t0 > 3000000000LL) {
                poll->stop();
                self->frameMs.push_back(f > t1 ? (f - t0) / 1e6 : -1.0);
                self->afterOne();
            }
        });
        poll->start();
    }

    static double median(std::vector<double> v)
    {
        if (v.empty()) {
            return -1;
        }
        std::sort(v.begin(), v.end());
        return v[v.size() / 2];
    }

    void afterOne()
    {
        Step& st = steps[stepIdx];
        if (st.cleanup) {
            st.cleanup();
        }
        if (++round >= rounds) {
            const auto mx = *std::max_element(syncMs.begin(), syncMs.end());
            lines << QString("%1\t%2\t%3\t%4").arg(st.name)
                .arg(median(syncMs), 0, 'f', 1).arg(mx, 0, 'f', 1).arg(median(frameMs), 0, 'f', 1);
            QFile f(outPath);
            if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                QTextStream(&f) << "acao\tmediana_ms\tpior_ms\tate_aparecer_ms\n" << lines.join(u'\n') << "\n";
            }
            syncMs.clear();
            frameMs.clear();
            round = 0;
            ++stepIdx;
        }
        next(400);
    }

    // PAUTA_DEBUG_BENCH_DUMP=<file>: a series of edits that are kept, then every drawn item of every page is written
    // (type, position, size, pitch, text). Two runs (with and without an optimization) must give the same file.
    void editsAndDump(const QString& path)
    {
        const int nm = measureCount();
        const int mid = nm / 2;
        const staff_idx_t st = score()->nstaves() / 2;
        selectMeasures(nm / 3, nm / 3);
        run("insert-measure");
        selectMeasures(mid, mid + 3);
        run("transpose-up");
        selectNote(mid + 10, st);
        run("pitch-up");
        selectMeasures(mid, mid + 3);
        run("action://notation/copy");
        selectMeasures(mid + 20, mid + 20);
        run("action://notation/paste");
        selectMeasures(mid + 30, mid + 30);
        run("action://notation/delete");
        selectMeasures(nm / 4, nm / 4 + 1);
        run("time-delete");
        selectNote(nm - 5, 0);
        run("pitch-down");
        run("action://notation/undo");
        run("action://notation/redo");
        notation->interaction()->clearSelection();

        QFile f(path);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return;
        }
        QTextStream out(&f);
        int pageNo = 0;
        for (Page* page : score()->pages()) {
            out << "PAGE " << pageNo++ << "\n";
            page->scanElements([&out](EngravingItem* e) {
                const RectF r = e->pageBoundingRect();
                out << e->typeName() << ' ' << QString::number(r.x(), 'f', 2) << ' ' << QString::number(r.y(), 'f', 2) << ' '
                    << QString::number(r.width(), 'f', 2) << ' ' << QString::number(r.height(), 'f', 2);
                if (e->isNote()) {
                    out << " p" << toNote(e)->pitch() << " t" << toNote(e)->tpc();
                } else if (e->isAccidental()) {
                    out << " a" << int(toAccidental(e)->accidentalType());
                } else if (e->isTextBase()) {
                    out << " '" << toTextBase(e)->plainText().toQString() << "'";
                }
                out << "\n";
            });
        }
    }

    void finish()
    {
        if (qEnvironmentVariableIsSet("PAUTA_DEBUG_BENCH_DUMP")) {
            editsAndDump(qEnvironmentVariable("PAUTA_DEBUG_BENCH_DUMP"));
        }
        // PAUTA_DEBUG_BENCH_SHOW=1: end with the middle note a half step up (it gets a sharp) and that note on screen (to look at the layout)
        if (qEnvironmentVariableIsSet("PAUTA_DEBUG_BENCH_SHOW")) {
            const int mid = measureCount() / 2;
            const staff_idx_t st = score()->nstaves() / 2;
            selectNote(mid, st);
            run("pitch-up");
            if (Note* n = noteAt(mid, st)) {
                notation->interaction()->showItem(n);
            }
        }
        QFile f(outPath);
        if (f.open(QIODevice::Append)) {
            QTextStream(&f) << "FIM\n";
        }
        if (qEnvironmentVariableIsSet("PAUTA_DEBUG_BENCH_QUIT")) {
            QTimer::singleShot(500, []() { QCoreApplication::exit(0); });
        }
    }
};
}

void pautaRunBenchmark(INotationPtr notation, IActionsDispatcher* dispatcher, const QString& outPath)
{
    qWarning() << "PAUTA_BENCH start" << outPath;
    auto b = std::make_shared<Bench>();
    b->notation = notation;
    b->dispatcher = dispatcher;
    b->outPath = outPath;
    b->clock.start();
    if (qEnvironmentVariableIsSet("PAUTA_DEBUG_BENCH_ROUNDS")) {
        b->rounds = std::max(1, qEnvironmentVariableIntValue("PAUTA_DEBUG_BENCH_ROUNDS"));
    }

    const int nm = b->measureCount();
    const int mid = nm / 2;
    const staff_idx_t st = b->score()->nstaves() / 2;
    Bench* p = b.get();
    auto undo = [p]() { p->run("action://notation/undo"); };
    auto cancel = [p]() { p->run("action://notation/cancel"); };

    b->steps = {
        { "mudar altura de 1 nota", [p, mid, st]() { p->selectNote(mid, st); }, "pitch-up", undo },
        { "desfazer", [p, mid, st]() { p->selectNote(mid, st); p->run("pitch-up"); }, "action://notation/undo", nullptr },
        { "refazer", [p, mid, st]() { p->selectNote(mid, st); p->run("pitch-up"); p->run("action://notation/undo"); },
          "action://notation/redo", undo },
        { "apagar 1 nota", [p, mid, st]() { p->selectNote(mid, st); }, "action://notation/delete", undo },
        { "escrever 1 nota", [p, mid, st]() {
                p->selectNote(mid, st);
                p->notation->interaction()->noteInput()->startNoteInput(NoteInputMethod::BY_NOTE_NAME, false);
            }, "note-c",
          [p, undo]() { p->notation->interaction()->noteInput()->endNoteInput(); undo(); } },
        { "apagar 1 compasso inteiro", [p, mid]() { p->selectMeasures(mid, mid); }, "action://notation/delete", undo },
        { "transpor 4 compassos", [p, mid]() { p->selectMeasures(mid, mid + 3); }, "transpose-up", undo },
        { "colar 4 compassos", [p, mid]() {
                p->selectMeasures(mid, mid + 3);
                p->run("action://notation/copy");
                p->selectMeasures(mid + 8, mid + 8);
            }, "action://notation/paste", undo },
        { "inserir compasso", [p, mid]() { p->selectMeasures(mid, mid); }, "insert-measure", undo },
        { "selecionar tudo", nullptr, "notation-select-all", cancel },
        { "primeira nota (inicio)", [p]() { p->selectNote(0, 0); }, "pitch-up", undo },
        { "ultima nota (fim)", [p, nm]() { p->selectNote(nm - 1, 0); }, "pitch-up", undo },
    };

    static std::shared_ptr<Bench> keepAlive; // the timers only hold weak references
    keepAlive = b;
    // PAUTA_DEBUG_BENCH_ONLY=<text>: run only the steps whose name contains it (to profile one of them)
    if (qEnvironmentVariableIsSet("PAUTA_DEBUG_BENCH_ONLY")) {
        const QString only = qEnvironmentVariable("PAUTA_DEBUG_BENCH_ONLY");
        b->steps.erase(std::remove_if(b->steps.begin(), b->steps.end(),
                                      [&only](const Step& st) { return !st.name.contains(only); }), b->steps.end());
    }
    // PAUTA_DEBUG_BENCH_VIEW=<view action> (e.g. view-mode-continuous): switch the view before timing
    if (qEnvironmentVariableIsSet("PAUTA_DEBUG_BENCH_VIEW")) {
        dispatcher->dispatch(qEnvironmentVariable("PAUTA_DEBUG_BENCH_VIEW").toStdString());
    }
    b->findWindow();
    b->lines << QString("# compassos=%1 pautas=%2").arg(nm).arg(b->score()->nstaves());
    b->next(500);
}
}
