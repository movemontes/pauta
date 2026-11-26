/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: always-visible tool column (Aseprite-style), with music icons and short names.
 */
#pragma once

#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "uicomponents/qml/Muse/UiComponents/abstracttoolbarmodel.h"

namespace mu::appshell {
class PautaToolsModel : public muse::uicomponents::AbstractToolBarModel
{
    Q_OBJECT
    QML_ELEMENT

    muse::ContextInject<context::IGlobalContext> globalContext = { this };

public:
    explicit PautaToolsModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;

private:
    bool m_loaded = false;
};
}
