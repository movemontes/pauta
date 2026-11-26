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

import Muse.Ui
import Muse.UiComponents

StyledTabButton {
    id: root

    property alias contextMenuModel: contextMenuButton.menuModel

    // Pauta: a short tip saying what each panel is for
    readonly property var pautaTips: ({
        [qsTrc("appshell", "Palettes")]: qsTrc("appshell", "Symbols to add to the score: clefs, key and time signatures, dynamics, articulations and more. Select notes, then click a symbol."),
        [qsTrc("appshell", "Layout")]: qsTrc("appshell", "The instruments and staves of the score: add, remove, hide or reorder them."),
        [qsTrc("appshell", "Properties")]: qsTrc("appshell", "Settings for what is selected: color, position, size, style and more."),
        [qsTrc("appshell", "Selection filter")]: qsTrc("appshell", "Choose which kinds of things a selection picks up."),
        [qsTrc("notation", "History")]: qsTrc("appshell", "Every change you made, in order. Click one to go back to it."),
        [qsTrc("appshell", "Mixer")]: qsTrc("appshell", "Volume, pan, mute and solo for each instrument."),
        [qsTrc("appshell", "Piano keyboard")]: qsTrc("appshell", "A piano to enter notes with the mouse; the keys light up during playback."),
        [qsTrc("appshell", "Timeline")]: qsTrc("appshell", "An overview of the whole piece, measure by measure.")
    })

    onHoveredChanged: {
        if (hovered) {
            ui.tooltip.show(root, root.text, root.pautaTips[root.text] || "")
        } else {
            ui.tooltip.hide(root)
        }
    }

    property bool isCutOff: false

    signal handleContextMenuItemRequested(string itemId)

    readonly property real actualHeight: 34
    height: actualHeight + 1 // For separator
    width: implicitWidth

    readonly property real textPadding: 10
    readonly property real buttonPadding: 6

    leftPadding: textPadding
    rightPadding: (contextMenuButton.visible ? buttonPadding : textPadding) + 1 // For separator
    topPadding: 0
    bottomPadding: 1 // For separator

    clip: true

    contentItem: Row {
        spacing: root.buttonPadding

        StyledTextLabel {
            anchors.verticalCenter: parent.verticalCenter

            horizontalAlignment: Qt.AlignLeft
            font: root.isCurrent ? ui.theme.bodyBoldFont : ui.theme.bodyFont

            text: root.text
        }

        MenuButton {
            id: contextMenuButton

            height: 20
            width: height

            anchors.verticalCenter: parent.verticalCenter
            visible: root.isCurrent

            navigation.panel: root.navigation.panel
            navigation.order: root.navigation.order + 1

            onHandleMenuItem: function(itemId) {
                root.handleContextMenuItemRequested(itemId)
            }
        }
    }

    background: Rectangle {
        id: backgroundRect
        anchors.fill: root

        color: ui.theme.backgroundSecondaryColor
        opacity: 1

        states: [
            State {
                name: "HOVERED"
                when: root.hovered && !root.isCurrent

                PropertyChanges {
                    target: backgroundRect
                    color: ui.theme.backgroundPrimaryColor
                    opacity: ui.theme.buttonOpacityHover
                }
            },

            State {
                name: "SELECTED"
                when: root.isCurrent

                PropertyChanges {
                    target: backgroundRect
                    color: ui.theme.backgroundPrimaryColor
                }
            }
        ]
    }

    Rectangle {
        visible: root.isCutOff

        anchors.top: root.top
        anchors.right: root.right
        anchors.rightMargin: 1
        anchors.bottom: root.bottom
        anchors.bottomMargin: root.isCurrent ? 0 : 1

        width: 20

        gradient: Gradient {
            orientation: Qt.Horizontal

            GradientStop {
                position: 0.0
                color: "transparent"
            }
            GradientStop {
                position: 1.0
                color: Utils.colorWithAlpha(backgroundRect.color, 0.7)
            }
        }
    }

    SeparatorLine {
        id: rightSeparator
        anchors.right: root.right
        orientation: Qt.Vertical
    }

    SeparatorLine {
        id: bottomSeparator
        anchors.bottom: root.bottom
        visible: !root.isCurrent
    }

    NavigationFocusBorder {
        navigationCtrl: root.navigation
        drawOutsideParent: false
        anchors.rightMargin: 1
        anchors.bottomMargin: root.isCurrent ? 0 : 1
    }

    states: []
}
