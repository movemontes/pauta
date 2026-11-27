/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: "complete" copy and paste. Copying whole measures also stores the time signatures, key signatures,
 * repeat barlines, voltas, jumps/markers and tempo marks of the passage; pasting puts them back at the
 * destination (each kind can be switched off in Edit > Paste brings along). After the pasted passage the
 * destination's own time and key signatures come back, so the rest of the score is not changed.
 */
#pragma once

#include <QByteArray>
#include <QJsonObject>
#include <string>
#include <utility>
#include <vector>

#include "async/channel.h"
#include "engraving/types/fraction.h"
#include "engraving/types/types.h"

namespace mu::engraving {
class Score;
class ChordRest;
class TDuration;
}

namespace mu::notation {
inline constexpr const char* PAUTA_PASTE_EXTRAS_MIME = "application/x-pauta-paste-extras";

// option keys: "timeSig", "keySig", "repeats", "tempo" (all on by default)
bool pautaPasteOption(const char* key);
void pautaTogglePasteOption(const char* key);
muse::async::Channel<std::string>& pautaPasteOptionChanged();

// other Pauta on/off options (stored under "pauta/<key>"); changes are announced on pautaPasteOptionChanged()
bool pautaFlag(const char* key, bool defaultValue);
void pautaToggleFlag(const char* key, bool defaultValue);

// "pauta-paste-with-timesig" -> "timeSig", etc.
inline const char* pautaPasteOptionKey(const std::string& actionCode)
{
    if (actionCode == "pauta-paste-with-keysig") {
        return "keySig";
    }
    if (actionCode == "pauta-paste-with-repeats") {
        return "repeats";
    }
    if (actionCode == "pauta-paste-with-tempo") {
        return "tempo";
    }
    return "timeSig";
}

// Pauta option "rippleDuration": a new duration for one chord/rest pushes (or pulls) the rest of its voice,
// across barlines, instead of writing over the next notes. Returns false when it can't (tuplets, etc.):
// then the normal change is used.
bool pautaRippleDuration(mu::engraving::Score* score, mu::engraving::ChordRest* cr, const mu::engraving::TDuration& newDuration);

// Called on copy: the extras of the current range selection (empty if there is nothing to add).
QByteArray pautaCapturePasteExtras(mu::engraving::Score* score);

class PautaPasteExtras
{
public:
    bool load(const QByteArray& data);

    // 1) before anything changes: remember the destination's key after the passage
    void prepare(mu::engraving::Score* score, const mu::engraving::Fraction& destTick, mu::engraving::staff_idx_t dstStaff);
    // 2) before the notes: time signatures (they move the barlines). Returns true if the score changed.
    bool applyBefore(mu::engraving::Score* score, const mu::engraving::Fraction& destTick);
    // 3) after the notes: key signatures, repeats, voltas, jumps/markers, tempo
    void applyAfter(mu::engraving::Score* score, const mu::engraving::Fraction& destTick, mu::engraving::staff_idx_t dstStaff);

private:
    QJsonObject m_obj;
    std::vector<std::pair<int, int> > m_destEndKeys; // per pasted staff: concert key, mode (INT_MIN = none)
};
}
