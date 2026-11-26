/*
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "pautatoolsmodel.h"

#include "ui/view/iconcodes.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::uicomponents;
namespace IconCode = muse::ui::IconCode;

PautaToolsModel::PautaToolsModel(QObject* parent)
    : AbstractToolBarModel(parent)
{
}

void PautaToolsModel::load()
{
    struct Tool {
        const char* code;
        IconCode::Code icon;
        const char* title;
    };

    // The most used tools, all visible at once (music icons from the app's own icon font)
    static const std::vector<Tool> tools {
        { "note-input", IconCode::Code::EDIT, "Write" },
        { "lyrics", IconCode::Code::LYRICS, "Lyrics" },
        { "chord-text", IconCode::Code::CHORD_SYMBOL, "Chord" },
        { "add-slur", IconCode::Code::NOTE_SLUR, "Slur" },
        { "add-hairpin", IconCode::Code::HAIRPIN, "Crescendo" },
        { "staff-text", IconCode::Code::TEXT_ABOVE_STAFF, "Text" },
        { "tempo", IconCode::Code::TEMPO_CHANGE, "Tempo" },
        { "pauta-change-key", IconCode::Code::KEY_SIGNATURE, "Change key" },
        { "pauta-chord-from-notes", IconCode::Code::CHORD_SYMBOL, "Chords from notes" },
        { "play", IconCode::Code::PLAY, "Play" },
        { "metronome", IconCode::Code::METRONOME, "Metronome" },
        { "zoomin", IconCode::Code::ZOOM_IN, "Zoom in" },
        { "zoomout", IconCode::Code::ZOOM_OUT, "Zoom out" },
        { "pauta-command-palette", IconCode::Code::SEARCH, "Find" },
    };

    ToolBarItemList items;
    for (const Tool& t : tools) {
        ToolBarItem* item = makeItem(t.code, TranslatableString("appshell", t.title));
        if (!item) {
            continue;
        }
        if (t.icon != IconCode::Code::NONE) {
            ui::UiAction action = item->action();
            action.iconCode = t.icon;
            item->setAction(action);
            item->setTitle(TranslatableString("appshell", t.title));
        }
        item->setShowTitle(true);
        items << item;
    }

    setItems(items);

    if (!m_loaded) {
        globalContext()->currentNotationChanged().onNotify(this, [this]() {
            AbstractToolBarModel::load();
        });
        m_loaded = true;
    }

    AbstractToolBarModel::load();
}
