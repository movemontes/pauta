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
#include "globalcontext.h"

#include <algorithm>

#include "notation/imasternotation.h"

using namespace mu::context;
using namespace mu::project;
using namespace mu::notation;
using namespace muse::async;

GlobalContext::GlobalContext()
{
    m_playbackState = std::make_shared<PlaybackState>();
}

void GlobalContext::setCurrentProject(const INotationProjectPtr& project)
{
    if (m_currentProject == project) {
        return;
    }

    m_currentProject = project;

    // Pauta: remember every score opened in this window (tabs); announced after the switch is complete
    bool isNewTab = false;
    if (project && std::find(m_openedProjects.begin(), m_openedProjects.end(), project) == m_openedProjects.end()) {
        m_openedProjects.push_back(project);
        isNewTab = true;
    }

    INotationPtr notation = project ? project->masterNotation()->notation() : nullptr;
    doSetCurrentNotation(notation);

    m_currentProjectChanged.notify();
    m_currentNotationChanged.notify();

    if (isNewTab) {
        m_openedProjectsChanged.notify();
    }
}

INotationProjectPtr GlobalContext::currentProject() const
{
    return m_currentProject;
}

Notification GlobalContext::currentProjectChanged() const
{
    return m_currentProjectChanged;
}

IMasterNotationPtr GlobalContext::currentMasterNotation() const
{
    return m_currentProject ? m_currentProject->masterNotation() : nullptr;
}

Notification GlobalContext::currentMasterNotationChanged() const
{
    //! NOTE Same as project
    return m_currentProjectChanged;
}

void GlobalContext::setCurrentNotation(const INotationPtr& notation)
{
    if (m_currentNotation == notation) {
        return;
    }

    doSetCurrentNotation(notation);
    m_currentNotationChanged.notify();
}

INotationPtr GlobalContext::currentNotation() const
{
    return m_currentNotation;
}

Notification GlobalContext::currentNotationChanged() const
{
    return m_currentNotationChanged;
}

void GlobalContext::doSetCurrentNotation(const INotationPtr& notation)
{
    if (m_currentNotation == notation) {
        return;
    }

    m_currentNotation = notation;

    if (m_currentNotation) {
        m_currentNotation->setIsOpen(true);
    }
}

void GlobalContext::setCurrentPlayer(const muse::audio::IPlayerPtr& player)
{
    m_playbackState->setPlayer(player);
}

IPlaybackStatePtr GlobalContext::playbackState() const
{
    return m_playbackState;
}

std::vector<INotationProjectPtr> GlobalContext::openedProjects() const
{
    return m_openedProjects;
}

void GlobalContext::removeOpenedProject(const INotationProjectPtr& project)
{
    auto it = std::find(m_openedProjects.begin(), m_openedProjects.end(), project);
    if (it != m_openedProjects.end()) {
        m_openedProjects.erase(it);
        m_openedProjectsChanged.notify();
    }
}

muse::async::Notification GlobalContext::openedProjectsChanged() const
{
    return m_openedProjectsChanged;
}
