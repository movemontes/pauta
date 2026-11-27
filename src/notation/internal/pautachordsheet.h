/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: "chord sheet" export. The lyrics of the score with the chord symbols written above the syllable where
 * each chord starts, one pair of lines per line of music, like the chord sheets people share for guitar.
 */
#pragma once

#include <QString>
#include <vector>

namespace mu::engraving {
class Score;
}

namespace mu::notation {
struct PautaChordSheetLine {
    QString chords; // may be empty
    QString lyrics; // may be empty (an instrumental line)
};

struct PautaChordSheet {
    QString title;
    std::vector<PautaChordSheetLine> lines;
    std::vector<QString> otherVerses; // verses 2, 3…: the words only
    bool empty() const { return lines.empty(); }
    QString toText() const;
};

PautaChordSheet pautaMakeChordSheet(mu::engraving::Score* score);
bool pautaWriteChordSheetPdf(const PautaChordSheet& sheet, const QString& path);
}
