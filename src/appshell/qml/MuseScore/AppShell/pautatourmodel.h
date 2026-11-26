/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: first-run tour with the mascot. Opens once on the first start and again with Help > "Show the tour again".
 */
#pragma once

#include <QObject>
#include <QQmlParserStatus>
#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"

namespace mu::appshell {
class PautaTourModel : public QObject, public QQmlParserStatus, public muse::actions::Actionable, public muse::Contextable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };

public:
    explicit PautaTourModel(QObject* parent = nullptr);

    Q_INVOKABLE bool seen() const;
    Q_INVOKABLE void markSeen();
    Q_INVOKABLE void run(const QString& action);

signals:
    void startRequested();

private:
    void classBegin() override;
    void componentComplete() override {}
};
}
