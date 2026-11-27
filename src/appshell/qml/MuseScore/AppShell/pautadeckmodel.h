/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: the top toolbar of the score page, in boxes by subject (icons only; the name shows on hover).
 */
#pragma once

#include <QVariantList>
#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"
#include "uicomponents/qml/Muse/UiComponents/abstracttoolbarmodel.h"

namespace mu::appshell {
class PautaDeckModel : public muse::uicomponents::AbstractToolBarModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QVariantList categories READ categories NOTIFY categoriesChanged)

    muse::ContextInject<context::IGlobalContext> globalContext = { this };

public:
    explicit PautaDeckModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;
    Q_INVOKABLE void run(const QString& action);

    QVariantList categories() const { return m_categories; }

signals:
    void categoriesChanged();

private:
    QVariantList m_categories;
    bool m_loaded = false;
};
}
