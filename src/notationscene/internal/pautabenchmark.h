/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: speed test. With PAUTA_DEBUG_BENCH=<file.tsv> the app opens the score, does common edits by itself
 * (change a pitch, write notes, delete, undo, paste, add a measure…) and writes how long each one took.
 * Used to find and fix slow spots; it does nothing unless that variable is set.
 */
#pragma once

#include <QString>

#include "notation/inotation.h"
#include "actions/iactionsdispatcher.h"

namespace mu::notation {
void pautaRunBenchmark(INotationPtr notation, muse::actions::IActionsDispatcher* dispatcher, const QString& outPath);
}
