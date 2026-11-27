/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: command palette (type the name of any command and press Enter).
 */
#pragma once

#include <QAbstractListModel>
#include <qqmlintegration.h>

#include "modularity/ioc.h"
#include "ui/iuiactionsregister.h"
#include "actions/iactionsdispatcher.h"
#include "shortcuts/ishortcutsregister.h"

namespace mu::appshell {
class CommandPaletteModel : public QAbstractListModel, public muse::Contextable
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString filter READ filter WRITE setFilter NOTIFY filterChanged)

    muse::ContextInject<muse::ui::IUiActionsRegister> actionsRegister = { this };
    muse::ContextInject<muse::actions::IActionsDispatcher> dispatcher = { this };
    muse::ContextInject<muse::shortcuts::IShortcutsRegister> shortcutsRegister = { this };

public:
    explicit CommandPaletteModel(QObject* parent = nullptr);

    enum Roles { TitleRole = Qt::UserRole + 1, ShortcutRole, CodeRole };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString filter() const { return m_filter; }
    void setFilter(const QString& filter);

    Q_INVOKABLE void load();
    Q_INVOKABLE bool trigger(int row);
    Q_INVOKABLE QString pautaDebugTypeText() const; // test hook: PAUTA_DEBUG_PALETTE_TYPE

signals:
    void filterChanged();

private:
    struct Item {
        std::string code;
        QString title;
        QString searchText;
        QString shortcut;
    };

    void applyFilter();
    static QString fold(const QString& s);

    std::vector<Item> m_all;
    std::vector<const Item*> m_visible;
    QString m_filter;
};
}
