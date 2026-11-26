/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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
import QtQuick
import QtQuick.Controls

import Muse.Ui
import Muse.UiComponents

TabButton {
    id: root

    property bool fillWidth: false

    readonly property TabBar tabBar: TabBar.tabBar
    property bool isCurrent: tabBar && (tabBar.currentItem === this)

    property alias navigation: navCtrl

    signal navigationTriggered()

    onNavigationTriggered: {
        if (tabBar) {
            tabBar.currentIndex = TabBar.index
        }
    }

    width: fillWidth && tabBar
           ? (tabBar.width - (tabBar.count - 1) * tabBar.spacing) / tabBar.count
           : implicitWidth

    leftPadding: 0
    rightPadding: 0

    font: isCurrent ? ui.theme.largeBodyBoldFont : ui.theme.largeBodyFont

    onPressed: {
        navigation.requestActiveByInteraction()

        root.ensureActiveFocus()
    }

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "TabButton"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.RadioButton
        accessible.name: root.text
        accessible.checked: root.isCurrent

        onActiveChanged: function(active) {
            if (active) {
                root.ensureActiveFocus()
            }
        }
        onTriggered: root.navigationTriggered()
    }

    contentItem: StyledTextLabel {
        id: textLabel
        text: root.text
        font: root.font
        opacity: 1.0 // Pauta: readable labels on tabs
    }

    background: Item {
        implicitHeight: 32

        NavigationFocusBorder { navigationCtrl: navCtrl }

        // Pauta: Aseprite-like tab (raised beige when selected)
        Rectangle {
            anchors.fill: parent
            anchors.topMargin: 2
            radius: 2
            color: isCurrent ? ui.theme.backgroundPrimaryColor : Qt.lighter(ui.theme.backgroundSecondaryColor, 1.15)
            border.width: 2
            border.color: ui.theme.strokeColor
            Rectangle { x: 2; y: 2; width: parent.width - 4; height: 2; color: Qt.lighter(ui.theme.backgroundPrimaryColor, 1.4); opacity: 0.7; visible: isCurrent }
        }

        Rectangle {
            id: selectedUnderline

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            height: 2

            visible: isCurrent
            color: ui.theme.accentColor
        }
    }

    states: [
        State {
            name: "HOVERED"
            when: root.hovered && !root.isCurrent

            PropertyChanges {
                target: contentItem
                opacity: 1
            }
        },

        State {
            name: "SELECTED"
            when: root.isCurrent

            PropertyChanges {
                target: textLabel
                opacity: 1
            }
        }
    ]
}
