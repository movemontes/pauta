/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: MIDI port and channel of each instrument (used when exporting MIDI), editable again.
 */
#pragma once

#include <QObject>
#include <QVariantList>
#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

namespace mu::appshell {
class PautaMidiChannelsModel : public QObject, public muse::Contextable
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList rows READ rows NOTIFY rowsChanged)

    muse::ContextInject<context::IGlobalContext> globalContext = { this };

public:
    explicit PautaMidiChannelsModel(QObject* parent = nullptr);

    QVariantList rows() const { return m_rows; }

    Q_INVOKABLE void load();
    // port and channel start at 1, as people count them
    Q_INVOKABLE void setPortAndChannel(int index, int port, int channel);

signals:
    void rowsChanged();

private:
    QVariantList m_rows;
};
}
