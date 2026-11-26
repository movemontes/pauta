/*
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "commandpalettemodel.h"

#include <set>

#include "shortcuts/shortcutstypes.h"

using namespace mu::appshell;
using namespace muse;

CommandPaletteModel::CommandPaletteModel(QObject* parent)
    : QAbstractListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

QString CommandPaletteModel::fold(const QString& s)
{
    // lower case, no accents: "transpor" finds "Transpor…", "ligadura" finds "Ligadura"
    QString d = s.normalized(QString::NormalizationForm_D);
    QString out;
    out.reserve(d.size());
    for (const QChar c : d) {
        if (c.category() != QChar::Mark_NonSpacing) {
            out.append(c.toLower());
        }
    }
    return out;
}

void CommandPaletteModel::load()
{
    beginResetModel();
    m_all.clear();
    std::set<QString> seenTitles;

    for (const ui::UiAction& action : actionsRegister()->actionList()) {
        if (action.code.empty() || action.title.isEmpty()) {
            continue;
        }
        if (!actionsRegister()->actionState(action.code).enabled) {
            continue;
        }
        if (action.code.rfind("nav-", 0) == 0 || action.code.rfind("diagnostic", 0) == 0 || action.code.rfind("dev-", 0) == 0) {
            continue;
        }

        QString title = action.title.qTranslatedWithoutMnemonic();
        title.remove(QStringLiteral("…"));
        title = title.trimmed();
        if (title.isEmpty() || seenTitles.count(title)) {
            continue;
        }
        seenTitles.insert(title);

        Item item;
        item.code = action.code;
        item.title = title;
        item.searchText = fold(title + " " + action.description.qTranslated());
        if (shortcutsRegister()) {
            item.shortcut = shortcuts::sequencesToNativeText(shortcutsRegister()->shortcut(action.code).sequences);
        }
        m_all.push_back(std::move(item));
    }

    std::sort(m_all.begin(), m_all.end(), [](const Item& a, const Item& b) {
        return QString::localeAwareCompare(a.title, b.title) < 0;
    });

    applyFilter();
    endResetModel();
}

void CommandPaletteModel::setFilter(const QString& filter)
{
    if (filter == m_filter) {
        return;
    }
    m_filter = filter;
    beginResetModel();
    applyFilter();
    endResetModel();
    emit filterChanged();
}

void CommandPaletteModel::applyFilter()
{
    m_visible.clear();
    const QStringList words = fold(m_filter).split(u' ', Qt::SkipEmptyParts);
    std::vector<const Item*> starts, contains;
    for (const Item& item : m_all) {
        bool all = true;
        for (const QString& w : words) {
            if (!item.searchText.contains(w)) {
                all = false;
                break;
            }
        }
        if (!all) {
            continue;
        }
        // titles that start with the first word come first
        if (!words.isEmpty() && fold(item.title).startsWith(words.first())) {
            starts.push_back(&item);
        } else {
            contains.push_back(&item);
        }
    }
    m_visible = starts;
    m_visible.insert(m_visible.end(), contains.begin(), contains.end());
}

int CommandPaletteModel::rowCount(const QModelIndex&) const
{
    return static_cast<int>(m_visible.size());
}

QVariant CommandPaletteModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }
    const Item* item = m_visible.at(index.row());
    switch (role) {
    case TitleRole: return item->title;
    case ShortcutRole: return item->shortcut;
    case CodeRole: return QString::fromStdString(item->code);
    }
    return QVariant();
}

QHash<int, QByteArray> CommandPaletteModel::roleNames() const
{
    return { { TitleRole, "title" }, { ShortcutRole, "shortcut" }, { CodeRole, "code" } };
}

bool CommandPaletteModel::trigger(int row)
{
    if (row < 0 || row >= rowCount()) {
        return false;
    }
    dispatcher()->dispatch(m_visible.at(row)->code);
    return true;
}
