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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.AppShell

// Pauta (Treco look): black rail on the left. Logo on top (accent square with the letter), sections standing up
// (icon + small capital label); the active one is a white block with an accent shadow; settings at the foot.
Rectangle {
    id: root

    property string currentPageName: ""
    property bool iconsOnly: false

    signal selected(string name)

    color: "#0A0A0A"

    PautaToolsModel { id: actions }

    NavigationSection {
        id: navSec
        name: "HomeMenuSection"
        enabled: root.enabled && root.visible
        order: 2
    }

    NavigationPanel {
        id: navPanel
        name: "HomeMenuPanel"
        enabled: root.enabled && root.visible
        section: navSec
        order: 1
        direction: NavigationPanel.Vertical

        accessible.name: qsTrc("appshell", "Home menu") + " " + navPanel.directionInfo
    }

    component RailItem: Item {
        id: item

        property string name: ""
        property string action: ""
        property string title: ""
        property int iconCode: IconCode.NONE
        property int row: 0
        readonly property bool active: name !== "" && name === root.currentPageName
        readonly property bool hovered: area.containsMouse

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: 72
        Layout.preferredHeight: 60
        width: 72
        height: 60

        function trigger() {
            if (item.action !== "") {
                actions.run(item.action)
            } else {
                root.selected(item.name)
            }
        }

        NavigationControl {
            id: nav
            name: item.title
            panel: navPanel
            row: item.row
            enabled: item.visible
            accessible.role: MUAccessible.Button
            accessible.name: item.title
            onTriggered: item.trigger()
        }

        Rectangle { // accent shadow under the active block
            x: 3; y: 3; width: parent.width; height: parent.height
            color: ui.theme.accentColor
            opacity: item.active ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }

        Rectangle {
            anchors.fill: parent
            color: item.active ? ui.theme.textFieldColor : (item.hovered ? "#262626" : "transparent")
            Behavior on color { ColorAnimation { duration: 150; easing.type: Easing.OutCubic } }

            NavigationFocusBorder { navigationCtrl: nav }
        }

        Column {
            anchors.centerIn: parent
            spacing: 5

            StyledIconLabel {
                anchors.horizontalCenter: parent.horizontalCenter
                iconCode: item.iconCode
                font.family: ui.theme.iconsFont.family
                font.pixelSize: 20
                color: item.active ? ui.theme.fontPrimaryColor : "#EDE8DC"
            }

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter
                width: item.width - 4
                text: item.title.toUpperCase()
                font.family: ui.theme.bodyBoldFont.family
                font.weight: Font.DemiBold
                font.pixelSize: 9
                font.letterSpacing: 0.2
                color: item.active ? ui.theme.fontPrimaryColor : (item.hovered ? "#EDE8DC" : "#B5B1A8")
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                maximumLineCount: 2
                lineHeight: 0.95
            }
        }

        MouseArea {
            id: area
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                nav.requestActiveByInteraction()
                item.trigger()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 14
        anchors.bottomMargin: 14
        spacing: 4

        // logo: Colcheia (the mascot) on a cream tile, like the app icon; opens "About Pauta"
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 14
            width: 48
            height: 48
            radius: 11
            color: logoArea.containsMouse ? "#FFFFFF" : "#FDF9EE"
            border.width: 2
            border.color: "#0A0A0A"
            Behavior on color { ColorAnimation { duration: 150 } }

            PautaMascot {
                anchors.fill: parent
                anchors.margins: 5
                pose: "regendo"
            }

            MouseArea {
                id: logoArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: actions.run("about-musescore")
            }
        }

        RailItem { name: "scores"; title: qsTrc("appshell", "Scores"); iconCode: IconCode.MUSIC_NOTES; row: 1 }
        RailItem { objectName: "pautaTourRail"; name: "extensions"; title: qsTrc("appshell", "Plugins"); iconCode: IconCode.PLUGIN; row: 2 }
        RailItem { action: "pauta-quick-guide"; title: qsTrc("action", "Quick guide"); iconCode: IconCode.QUESTION_MARK; row: 3 }

        Item { Layout.fillHeight: true }

        RailItem { action: "preference-dialog"; title: qsTrc("appshell", "Preferences"); iconCode: IconCode.SETTINGS_COG; row: 4 }
    }
}
