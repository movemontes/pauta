/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: Colcheia tells about an on/off option the first 3 times something related happens.
 */
#pragma once

#include <QObject>
#include <QPointF>
#include <QQuickItem>
#include <qqmlintegration.h>

#include "async/asyncable.h"

namespace mu::appshell {
class PautaHintModel : public QObject, public muse::async::Asyncable
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString text READ text NOTIFY changed)
    Q_PROPERTY(bool active READ active NOTIFY changed)

public:
    explicit PautaHintModel(QObject* parent = nullptr);

    QString text() const { return m_text; }
    bool active() const { return m_active; }

    Q_INVOKABLE void done();
    Q_INVOKABLE QPointF cursorIn(QQuickItem* item) const;

signals:
    void changed();

private:
    void show(const std::string& key);

    QString m_text;
    bool m_active = false;
};
}
