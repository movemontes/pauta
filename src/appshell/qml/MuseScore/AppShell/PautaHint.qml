/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: Colcheia and a short note follow the mouse for a few seconds to tell about an on/off option.
 * Clicks go through (nothing here takes the mouse). Shown the first 3 times per subject (pautahintmodel.cpp).
 */
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.AppShell

Item {
    id: root

    anchors.fill: parent
    z: 99999

    PautaHintModel {
        id: hint
    }

    readonly property bool showing: hint.active

    visible: group.opacity > 0

    function follow() {
        const p = hint.cursorIn(root)
        group.x = Math.max(8, Math.min(p.x + 22, root.width - group.width - 8))
        group.y = Math.max(8, Math.min(p.y + 22, root.height - group.height - 8))
    }

    onShowingChanged: {
        if (showing) {
            follow()
            hideTimer.restart()
        }
    }

    Timer {
        interval: 16
        repeat: true
        running: root.visible
        onTriggered: root.follow()
    }

    Timer {
        id: hideTimer
        interval: 9000
        onTriggered: hint.done()
    }

    Row {
        id: group

        spacing: 4
        opacity: root.showing ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 250 } }

        PautaMascot {
            width: 76
            height: 84
            pose: "aponta-direita"
            anchors.verticalCenter: parent.verticalCenter
        }

        Item {
            width: 320
            height: bubble.height
            anchors.verticalCenter: parent.verticalCenter

            Rectangle { x: 4; y: 4; width: parent.width; height: parent.height; color: ui.theme.strokeColor }

            Rectangle {
                id: bubble
                width: parent.width
                height: col.implicitHeight + 22
                color: ui.theme.textFieldColor
                border.width: 2
                border.color: ui.theme.strokeColor

                Column {
                    id: col
                    x: 12
                    y: 11
                    width: parent.width - 24
                    spacing: 6

                    StyledTextLabel {
                        width: parent.width
                        text: qsTrc("appshell", "Important tip!")
                        font: ui.theme.largeBodyBoldFont
                        horizontalAlignment: Text.AlignLeft
                    }

                    StyledTextLabel {
                        width: parent.width
                        text: hint.text
                        textFormat: Text.StyledText // the menu path comes in <b>bold</b>
                        font: ui.theme.bodyFont
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignLeft
                    }

                    StyledTextLabel {
                        width: parent.width
                        text: qsTrc("appshell", "Tips: Help → Mascot in tips")
                        opacity: 0.55
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignLeft
                    }
                }
            }
        }
    }
}
