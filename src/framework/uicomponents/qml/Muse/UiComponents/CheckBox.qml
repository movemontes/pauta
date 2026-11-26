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
import QtQuick.Layouts

import Muse.Ui

FocusScope {
    id: root

    property bool checked: false
    property alias pressed: clickableArea.containsPress
    property alias hovered: clickableArea.containsMouse
    property bool isIndeterminate: false

    property alias text: label.text
    property alias font: label.font
    property alias backgroundColor: box.color
    property alias backgroundOpacity: box.opacity

    property alias navigation: navCtrl

    signal clicked

    implicitHeight: contentRow.implicitHeight
    implicitWidth: contentRow.implicitWidth

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    NavigationControl {
        id: navCtrl

        name: root.objectName != "" ? root.objectName : "CheckBox"
        enabled: root.enabled && root.visible
        accessible.role: MUAccessible.CheckBox
        accessible.name: root.text
        accessible.checked: root.checked

        onActiveChanged: {
            if (!root.activeFocus) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.clicked()
    }

    RowLayout {
        id: contentRow
        spacing: 6

        Rectangle {
            id: box

            height: 20
            width: 20

            opacity: ui.theme.buttonOpacityNormal

            border.width: 2
            border.color: ui.theme.strokeColor
            color: ui.theme.textFieldColor // Pauta: white box like Aseprite

            radius: 2

            // Pauta: sunken look: shadow on the top and left inner edges
            Rectangle { x: 2; y: 2; width: parent.width - 4; height: 2; color: Qt.darker(ui.theme.textFieldColor, 1.3) }
            Rectangle { x: 2; y: 2; width: 2; height: parent.height - 4; color: Qt.darker(ui.theme.textFieldColor, 1.3) }

            NavigationFocusBorder { navigationCtrl: navCtrl }

            StyledIconLabel {
                anchors.fill: parent
                iconCode: root.isIndeterminate ? IconCode.MINUS : IconCode.TICK_RIGHT_ANGLE
                visible: root.checked || root.isIndeterminate
                color: ui.theme.fontPrimaryColor
                font.bold: true
            }
        }

        StyledTextLabel {
            id: label
            visible: !isEmpty

            readonly property real availableWidth: root.width - contentRow.spacing - box.width

            Layout.preferredWidth: availableWidth > 0 ? Math.min(availableWidth, label.implicitWidth) : label.implicitWidth
            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            displayTruncatedTextOnHover: true
        }
    }

    MouseArea {
        id: clickableArea

        anchors.fill: contentRow
        anchors.margins: -4

        hoverEnabled: !label.hoveredLink
        z: label.z - 1 // enable clicking on links in label text

        onClicked: {
            navCtrl.requestActiveByInteraction()

            root.clicked()
        }
    }

    states: [
        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !clickableArea.pressed

            PropertyChanges {
                target: box
                opacity: ui.theme.buttonOpacityHover
            }
        },

        State {
            name: "PRESSED"
            when: clickableArea.containsMouse && clickableArea.pressed

            PropertyChanges {
                target: box
                opacity: ui.theme.buttonOpacityHit
            }
        }
    ]
}
