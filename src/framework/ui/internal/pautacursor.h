/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta (owner standard): the app's own mouse pointer, in the Treco family style.
 */
#pragma once

#include <QObject>

namespace muse::ui {
class PautaCursor : public QObject
{
public:
    static void install();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
};
}
