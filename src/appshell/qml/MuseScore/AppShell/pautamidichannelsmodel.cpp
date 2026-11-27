/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: MIDI port and channel of each instrument (see pautamidichannelsmodel.h).
 */
#include "pautamidichannelsmodel.h"

#include "engraving/dom/instrument.h"
#include "engraving/dom/masterscore.h"
#include "engraving/dom/part.h"

using namespace mu::appshell;
using namespace mu::engraving;

PautaMidiChannelsModel::PautaMidiChannelsModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void PautaMidiChannelsModel::load()
{
    m_rows.clear();
    auto master = globalContext()->currentMasterNotation();
    MasterScore* ms = master ? master->masterScore() : nullptr;
    if (ms) {
        for (Part* part : ms->parts()) {
            for (const auto& pair : part->instruments()) {
                const Instrument* instr = pair.second;
                const bool several = instr->channel().size() > 1;
                for (const InstrChannel* ch : instr->channel()) {
                    const int idx = ch->channel();
                    if (idx < 0 || idx >= int(ms->midiMapping().size())) {
                        continue;
                    }
                    QVariantMap row;
                    row["index"] = idx;
                    row["name"] = several ? part->partName().toQString() + " (" + ch->name().toQString() + ")"
                                  : part->partName().toQString();
                    row["port"] = ms->midiPort(idx) + 1;
                    row["channel"] = ms->midiChannel(idx) + 1;
                    m_rows.push_back(row);
                }
            }
        }
    }
    emit rowsChanged();
}

void PautaMidiChannelsModel::setPortAndChannel(int index, int port, int channel)
{
    auto master = globalContext()->currentMasterNotation();
    MasterScore* ms = master ? master->masterScore() : nullptr;
    if (!ms || index < 0 || index >= int(ms->midiMapping().size()) || port < 1 || channel < 1 || channel > 16) {
        return;
    }
    MidiMapping* mm = ms->midiMapping(index);
    if (mm->port() == port - 1 && mm->channel() == channel - 1) {
        return;
    }
    ms->updateMidiMapping(mm->articulation(), nullptr, port - 1, channel - 1);
    int maxPort = 0;
    for (const MidiMapping& m : ms->midiMapping()) {
        maxPort = std::max(maxPort, int(m.port()));
    }
    ms->setMidiPortCount(maxPort + 1);
    ms->checkMidiMapping(); // a custom mapping is saved in the file and used by the MIDI export
    if (auto project = globalContext()->currentProject()) {
        project->markAsUnsaved();
    }
}
