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

import Muse.Ui
import Muse.UiComponents
import MuseScore.AppShell

// Pauta: the page tabs (Home, Score, Publish) are three big flat buttons that share the whole top row.
// Bold names; the open page is filled with the accent colour and has the hard black shadow of the Treco look.
Item {
    id: root

    implicitWidth: 3 * 180
    implicitHeight: 52

    property alias navigation: navPanel

    property alias currentUri: toolBarModel.currentUri

    signal selected(string uri)

    function select(uri) {
        root.selected(uri)
    }

    function focusOnFirst() {
        var tab = tabs.itemAt(0)
        if (tab) {
            tab.navigation.requestActive()
        }
    }

    MainToolBarModel {
        id: toolBarModel
    }

    Component.onCompleted: {
        toolBarModel.load()
    }

    NavigationPanel {
        id: navPanel
        name: "MainToolBar"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "Main toolbar") + " " + navPanel.directionInfo
    }

    Row {
        id: row

        anchors.fill: parent
        anchors.leftMargin: 8
        anchors.rightMargin: 11 // room for the shadow
        anchors.topMargin: 5
        anchors.bottomMargin: 8
        spacing: 10

        Repeater {
            id: tabs

            model: toolBarModel

            delegate: Item {
                id: tab

                required property string title
                required property bool isChecked
                required property string uri
                required property int index

                property alias navigation: nav

                // the open page carries the accent colour, with white letters
                readonly property color fill: tab.isChecked ? ui.theme.accentColor
                                                            : (area.containsMouse ? Qt.tint(ui.theme.textFieldColor, "#1A000000") : ui.theme.textFieldColor)
                readonly property bool darkFill: (0.299 * fill.r + 0.587 * fill.g + 0.114 * fill.b) < 0.4

                width: (row.width - row.spacing * (tabs.count - 1)) / Math.max(1, tabs.count)
                height: row.height

                NavigationControl {
                    id: nav
                    name: tab.title
                    panel: navPanel
                    order: tab.index
                    enabled: tab.visible

                    accessible.role: MUAccessible.RadioButton
                    accessible.name: tab.title
                    accessible.checked: tab.isChecked

                    onTriggered: root.selected(tab.uri)
                }

                Rectangle { // hard shadow under the open page
                    x: 3
                    y: 3
                    width: parent.width
                    height: parent.height
                    color: ui.theme.strokeColor
                    visible: tab.isChecked
                }

                Rectangle {
                    anchors.fill: parent
                    color: tab.fill
                    border.width: 2
                    border.color: ui.theme.strokeColor

                    Behavior on color { ColorAnimation { duration: 150 } }

                    NavigationFocusBorder { navigationCtrl: nav }

                    StyledTextLabel {
                        anchors.centerIn: parent
                        width: parent.width - 16
                        text: tab.title.toUpperCase()
                        font.family: "Pauta Titulo" // Archivo, wide: big, flat letters
                        font.weight: Font.Black
                        font.pixelSize: 25
                        font.letterSpacing: 1
                        color: tab.isChecked ? "#FFFFFF" : "#0A0A0A"
                        elide: Text.ElideRight
                    }
                }

                MouseArea {
                    id: area
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        nav.requestActiveByInteraction()
                        root.selected(tab.uri)
                    }
                }
            }
        }
    }
}
