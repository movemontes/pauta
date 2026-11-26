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
#include "global/iinteractive.h"
#include "context/iglobalcontext.h"

namespace mu::appshell {
class PautaTourModel : public QObject, public QQmlParserStatus, public muse::actions::Actionable, public muse::Contextable
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::ContextInject<muse::IInteractive> interactive = { this };
    muse::ContextInject<mu::context::IGlobalContext> globalContext = { this };

public:
    explicit PautaTourModel(QObject* parent = nullptr);

    Q_INVOKABLE bool seen() const;
    Q_INVOKABLE void markSeen();
    // testing: PAUTA_TOUR_STEP=n opens the tour at step n (1-based) on start; 0 = off
    Q_INVOKABLE int debugStep() const;
    Q_INVOKABLE void run(const QString& action);

    // score page for the tour: the current score, or a blank example score when none is open
    Q_INVOKABLE void showScorePage();
    // back to the home page; closes the example score if the tour opened it
    Q_INVOKABLE void showHomePage();

signals:
    void startRequested();

private:
    void classBegin() override;
    void componentComplete() override {}

    bool m_openedExample = false;
};
}
