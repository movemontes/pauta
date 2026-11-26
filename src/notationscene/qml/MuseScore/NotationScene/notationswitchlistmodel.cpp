/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "notationswitchlistmodel.h"

#include <algorithm>
#include "project/inotationproject.h"

#include "log.h"

using namespace mu::notation;
using namespace mu::project;

NotationSwitchListModel::NotationSwitchListModel(QObject* parent)
    : QAbstractListModel(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
    m_notationChangedReceiver = std::make_unique<muse::async::Asyncable>();
}

void NotationSwitchListModel::load()
{
    TRACEFUNC;

    onCurrentProjectChanged();
    context()->currentProjectChanged().onNotify(m_notationChangedReceiver.get(), [this]() {
        onCurrentProjectChanged();
    });

    onCurrentNotationChanged();
    context()->currentNotationChanged().onNotify(m_notationChangedReceiver.get(), [this]() {
        onCurrentNotationChanged();
    });

    // Pauta: other scores open in this window are shown as tabs too
    context()->openedProjectsChanged().onNotify(m_notationChangedReceiver.get(), [this]() {
        loadNotations();
        onCurrentNotationChanged();
    });
}

void NotationSwitchListModel::onCurrentProjectChanged()
{
    async_disconnectAll();

    loadNotations();

    INotationProjectPtr project = context()->currentProject();
    if (!project) {
        return;
    }

    project->masterNotation()->excerptsChanged().onNotify(this, [this]() {
        loadNotations();
    });

    listenProjectSavingStatusChanged();
}

void NotationSwitchListModel::onCurrentNotationChanged()
{
    INotationPtr notation = context()->currentNotation();
    if (!notation) {
        return;
    }

    int currentNotationIndex = m_notations.indexOf(notation);
    emit currentNotationIndexChanged(currentNotationIndex);
}

void NotationSwitchListModel::loadNotations()
{
    TRACEFUNC;

    beginResetModel();
    m_notations.clear();
    m_owners.clear();

    project::INotationProjectPtr current = context()->currentProject();
    std::vector<project::INotationProjectPtr> projects = context()->openedProjects();
    if (current && std::find(projects.begin(), projects.end(), current) == projects.end()) {
        projects.push_back(current);
    }

    for (const project::INotationProjectPtr& project : projects) {
        if (!project || !project->masterNotation()) {
            continue;
        }
        IMasterNotationPtr masterNotation = project->masterNotation();

        // Pauta: one tab per score; the score being edited also shows its open parts
        m_notations << masterNotation->notation();
        m_owners << project;

        if (project != current) {
            continue;
        }

        listenNotationOpeningStatus(masterNotation->notation());

        for (const IExcerptNotationPtr& excerpt: masterNotation->excerpts()) {
            if (excerpt->notation()->isOpen()) {
                m_notations << excerpt->notation();
                m_owners << project;
            }

            listenNotationOpeningStatus(excerpt->notation());
            listenExcerptNotationTitleChanged(excerpt);
        }
    }

    endResetModel();

    if (current && context()->currentNotation() && !m_notations.contains(context()->currentNotation())) {
        int masterIndex = m_notations.indexOf(current->masterNotation()->notation());
        if (masterIndex >= 0) {
            context()->setCurrentNotation(m_notations[masterIndex]);
        }
    }
}

void NotationSwitchListModel::listenNotationOpeningStatus(INotationPtr notation)
{
    notation->openChanged().onNotify(this, [this]() {
        loadNotations(); // Pauta: rebuild so parts stay next to their score
        onCurrentNotationChanged();
    }, Asyncable::Mode::SetReplace);
}

void NotationSwitchListModel::listenExcerptNotationTitleChanged(IExcerptNotationPtr excerptNotation)
{
    INotationWeakPtr weakNotationPtr = excerptNotation->notation();

    excerptNotation->nameChanged().onNotify(this, [this, weakNotationPtr]() {
        INotationPtr notation = weakNotationPtr.lock();
        if (!notation) {
            return;
        }

        int index = m_notations.indexOf(notation);
        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex, { RoleTitle });
    }, Asyncable::Mode::SetReplace);
}

void NotationSwitchListModel::listenProjectSavingStatusChanged()
{
    INotationProjectPtr currentProject = context()->currentProject();
    if (!currentProject) {
        return;
    }

    currentProject->needSave().notification.onNotify(this, [this]() {
        INotationProjectPtr project = context()->currentProject();
        if (!project) {
            return;
        }

        int index = m_notations.indexOf(project->masterNotation()->notation());
        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex, { RoleNeedSave, RoleIsCloud });
    });

    currentProject->displayNameChanged().onNotify(this, [this]() {
        INotationProjectPtr project = context()->currentProject();
        if (!project) {
            return;
        }

        int index = m_notations.indexOf(project->masterNotation()->notation());
        QModelIndex modelIndex = this->index(index);
        emit dataChanged(modelIndex, modelIndex, { RoleTitle, RoleIsCloud });
    });
}

INotationPtr NotationSwitchListModel::currentNotation() const
{
    return context()->currentNotation();
}

IMasterNotationPtr NotationSwitchListModel::currentMasterNotation() const
{
    return context()->currentMasterNotation();
}

QVariant NotationSwitchListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    INotationPtr notation = m_notations[index.row()];

    switch (role) {
    case RoleTitle: return QVariant::fromValue(notation->name());
    case RoleNeedSave: {
        project::INotationProjectPtr owner = ownerOf(index.row());
        bool needSave = owner && owner->needSave().val && isMasterNotation(notation);
        return QVariant::fromValue(needSave);
    }
    case RoleIsCloud: {
        project::INotationProjectPtr owner = ownerOf(index.row());
        bool isCloud = owner && owner->isCloudProject() && isMasterNotation(notation);
        return QVariant::fromValue(isCloud);
    }
    }

    return QVariant();
}

int NotationSwitchListModel::rowCount(const QModelIndex&) const
{
    return m_notations.size();
}

QHash<int, QByteArray> NotationSwitchListModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { RoleTitle, "title" },
        { RoleNeedSave, "needSave" },
        { RoleIsCloud, "isCloud" }
    };

    return roles;
}

void NotationSwitchListModel::setCurrentNotation(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    project::INotationProjectPtr owner = ownerOf(index);
    if (owner && owner != context()->currentProject()) {
        context()->setCurrentProject(owner); // Pauta: switch to another score's tab
        return;
    }

    context()->setCurrentNotation(m_notations[index]);
}

void NotationSwitchListModel::closeNotation(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    INotationPtr notation = m_notations[index];

    if (isMasterNotation(notation)) {
        project::INotationProjectPtr owner = ownerOf(index);
        if (owner && owner != context()->currentProject()) {
            context()->setCurrentProject(owner); // Pauta: close that score's tab (asks "save?" if needed)
        }
        dispatcher()->dispatch("file-close");
    } else {
        if (notation == currentNotation()) {
            // Set new current notation
            context()->setCurrentNotation(currentMasterNotation()->notation()); // Pauta: back to this score, not another tab
        }
        currentMasterNotation()->setExcerptIsOpen(notation, false);
    }
}

void NotationSwitchListModel::closeOtherNotations(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    INotationPtr notationToKeepOpen = m_notations[index];
    context()->setCurrentNotation(notationToKeepOpen);

    // Copy the list to avoid modifying it while iterating
    QList<INotationPtr> notations = m_notations;

    for (const INotationPtr& notation : notations) {
        if (!isMasterNotation(notation) && notation != notationToKeepOpen) {
            currentMasterNotation()->setExcerptIsOpen(notation, false);
        }
    }
}

void NotationSwitchListModel::closeAllNotations()
{
    dispatcher()->dispatch("file-close");
}

QVariantList NotationSwitchListModel::contextMenuItems(int index) const
{
    if (!isIndexValid(index)) {
        return {};
    }

    QVariantList result {
        QVariantMap { { "id", "close-tab" }, { "title", muse::qtrc("notation", "Close tab") } },
    };

    bool canCloseOtherTabs = rowCount() > 2 || (rowCount() == 2 && isMasterNotation(m_notations[index]));
    if (canCloseOtherTabs) {
        result << QVariantMap { { "id", "close-other-tabs" }, { "title", muse::qtrc("notation", "Close other tabs") } };
    }

    bool canCloseAllTabs = rowCount() > 1;
    if (canCloseAllTabs) {
        result << QVariantMap { { "id", "close-all-tabs" }, { "title", muse::qtrc("notation", "Close all tabs") } };
    }

    return result;
}

void NotationSwitchListModel::handleContextMenuItem(int index, const QString& itemId)
{
    if (itemId == "close-tab") {
        closeNotation(index);
    } else if (itemId == "close-other-tabs") {
        closeOtherNotations(index);
    } else if (itemId == "close-all-tabs") {
        closeAllNotations();
    }
}

bool NotationSwitchListModel::isIndexValid(int index) const
{
    return index >= 0 && index < m_notations.size();
}

bool NotationSwitchListModel::isMasterNotation(const INotationPtr notation) const
{
    for (const project::INotationProjectPtr& project : m_owners) {
        if (project && project->masterNotation() && project->masterNotation()->notation() == notation) {
            return true;
        }
    }
    return currentMasterNotation() && currentMasterNotation()->notation() == notation;
}

project::INotationProjectPtr NotationSwitchListModel::ownerOf(int index) const
{
    return (index >= 0 && index < m_owners.size()) ? m_owners[index] : nullptr;
}
