/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: the top toolbar of the score page, in boxes by subject (see pautadeckmodel.h).
 */
#include "pautadeckmodel.h"

#include "ui/view/iconcodes.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::uicomponents;
namespace IconCode = muse::ui::IconCode;

PautaDeckModel::PautaDeckModel(QObject* parent)
    : AbstractToolBarModel(parent)
{
}

void PautaDeckModel::load()
{
    struct Tool {
        const char* code;
        IconCode::Code icon;
    };
    struct Box {
        const char* title;
        std::vector<Tool> tools;
    };

    // Everything Pauta can do with the score, one click away
    static const std::vector<Box> boxes {
        { "SCORE", {
              { "parts", IconCode::Code::PAGE },
              { "toggle-instruments", IconCode::Code::SCORE },
              { "toggle-mixer", IconCode::Code::MIXER },
              { "toggle-piano-keyboard", IconCode::Code::MUSIC_NOTES },
              { "pauta-command-palette", IconCode::Code::SEARCH },
          } },
        { "KEY", {
              { "pauta-change-key", IconCode::Code::KEY_SIGNATURE },
              { "transpose", IconCode::Code::KEY_SIGNATURE_1_SHARP },
              { "concert-pitch", IconCode::Code::TUNING_FORK },
              { "pauta-chord-from-notes", IconCode::Code::CHORD_SYMBOL },
          } },
        { "REPEATS", {
              { "pauta-repeat-size-large", IconCode::Code::REPEAT_START },
              { "pauta-repeat-wings", IconCode::Code::BARLINE_WINGED },
              { "pauta-play-passes-2", IconCode::Code::VOLTA },
              { "unroll-repeats", IconCode::Code::PLAY_REPEATS },
          } },
        { "SOUND", {
              { "pauta-sound-generaluser", IconCode::Code::AUDIO },
              { "pauta-toggle-reverb", IconCode::Code::WAVEFORM },
              { "metronome", IconCode::Code::METRONOME },
              { "countin", IconCode::Code::COUNT_IN },
              { "pauta-midi-channels", IconCode::Code::MIDI_INPUT },
          } },
        { "LAYOUT", {
              { "edit-style", IconCode::Code::BRUSH },
              { "page-settings", IconCode::Code::ORIENTATION_PORTRAIT },
              { "system-break", IconCode::Code::LINE_BREAK },
              { "page-break", IconCode::Code::PAGE_BREAK },
              { "toggle-system-lock", IconCode::Code::SYSTEM_LOCK },
              { "pauta-copy-layout-to-parts", IconCode::Code::COPY },
          } },
        { "SHARE", {
              { "pauta-capture-copy", IconCode::Code::CAMERA },
              { "pauta-export-to-program", IconCode::Code::SHARE_FILE },
              { "file-export", IconCode::Code::VIDEO },
              { "print", IconCode::Code::PRINT },
          } },
    };

    ToolBarItemList items;
    m_categories.clear();
    for (const Box& box : boxes) {
        const int start = items.size();
        for (const Tool& t : box.tools) {
            ToolBarItem* item = makeItem(t.code);
            if (!item) {
                continue;
            }
            ui::UiAction action = item->action();
            action.iconCode = t.icon;
            item->setAction(action);
            items << item;
        }
        QVariantMap cat;
        cat["title"] = muse::qtrc("appshell", box.title);
        cat["start"] = start;
        cat["count"] = int(items.size()) - start;
        m_categories << cat;
    }

    setItems(items);
    emit categoriesChanged();

    if (!m_loaded) {
        globalContext()->currentNotationChanged().onNotify(this, [this]() {
            AbstractToolBarModel::load();
        });
        m_loaded = true;
    }

    AbstractToolBarModel::load();
}

void PautaDeckModel::run(const QString& action)
{
    dispatcher()->dispatch(action.toStdString());
}
