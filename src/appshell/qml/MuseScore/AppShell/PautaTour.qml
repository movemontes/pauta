/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: first-run tour. The mascot walks to each part of the window and explains it in one sentence.
 */
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.AppShell

Item {
    id: root

    anchors.fill: parent
    visible: false
    z: 100000

    property int step: 0

    // where: c = center, br = bottom right (near "New"), l = left rail, r = right tools column
    readonly property var steps: [
        { pose: "oi", where: "c", title: qsTrc("appshell", "Hi! I'm Colcheia."),
          text: qsTrc("appshell", "I'll show you around Pauta in a few steps. You can skip at any time.") },
        { pose: "aponta-direita", where: "br", title: qsTrc("appshell", "Start here"),
          text: qsTrc("appshell", "“New” makes a blank score: pick the instruments and start writing. “Open other…” opens a file you already have.") },
        { pose: "aponta-esquerda", where: "l", title: qsTrc("appshell", "The side rail"),
          text: qsTrc("appshell", "Your recent scores, extra plugins and the quick guide live here.") },
        { pose: "pensando", where: "c", title: qsTrc("appshell", "Writing notes"),
          text: qsTrc("appshell", "With a score open, press N, pick a note length and type the letters C D E F G A B (or click on the staff).") },
        { pose: "aponta-direita", where: "r", title: qsTrc("appshell", "Tools on the right"),
          text: qsTrc("appshell", "With a score open, the column on the right has lyrics, chords, slurs, text, tempo, change key and more.") },
        { pose: "regendo", where: "c", title: qsTrc("appshell", "Listen"),
          text: qsTrc("appshell", "Press the space bar to play and pause. The piano keys light up while it plays.") },
        { pose: "comemorando", where: "c", title: qsTrc("appshell", "That's it!"),
          text: qsTrc("appshell", "Rest the mouse on any button for a moment and I'll tell you what it does.") }
    ]
    readonly property var cur: steps[step]

    function start() {
        step = 0
        visible = true
        enterAnim.restart()
        root.forceActiveFocus()
    }

    function back() {
        if (step > 0) {
            step--
            enterAnim.restart()
        }
    }

    function finish() {
        visible = false
        tour.markSeen()
    }

    function next() {
        if (step >= steps.length - 1) {
            finish()
            return
        }
        step++
        enterAnim.restart()
    }

    PautaTourModel {
        id: tour
        onStartRequested: root.start()
    }

    Component.onCompleted: {
        if (!tour.seen()) {
            startTimer.start()
        }
    }

    Timer { id: startTimer; interval: 1200; onTriggered: root.start() }

    // dim the window; clicks do not go through
    Rectangle {
        anchors.fill: parent
        color: "#0A0A0A"
        opacity: 0.38
        MouseArea { anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.AllButtons; onWheel: function(w) { w.accepted = true } }
    }

    Item {
        id: group

        width: row.implicitWidth
        height: row.implicitHeight

        x: {
            switch (root.cur.where) {
            case "br": return root.width - width - 360
            case "l": return 96
            case "r": return root.width - width - 96
            default: return (root.width - width) / 2
            }
        }
        y: {
            switch (root.cur.where) {
            case "br": return root.height - height - 96
            case "l": return 120
            case "r": return 160
            default: return (root.height - height) / 2
            }
        }

        transform: Translate { id: shift }

        ParallelAnimation {
            id: enterAnim
            NumberAnimation { target: shift; property: "y"; from: 12; to: 0; duration: 200; easing.type: Easing.OutBack }
            NumberAnimation { target: group; property: "opacity"; from: 0; to: 1; duration: 180 }
        }

        RowLayout {
            id: row
            spacing: 14
            layoutDirection: root.cur.where === "l" ? Qt.RightToLeft : Qt.LeftToRight

            // speech bubble
            Item {
                Layout.preferredWidth: 320
                Layout.preferredHeight: bubble.implicitHeight
                Layout.alignment: Qt.AlignVCenter

                Rectangle { x: 4; y: 4; width: parent.width; height: parent.height; color: ui.theme.strokeColor }

                Rectangle {
                    id: bubble
                    width: parent.width
                    implicitHeight: col.implicitHeight + 28
                    height: implicitHeight
                    color: ui.theme.textFieldColor
                    border.width: 2
                    border.color: ui.theme.strokeColor

                    ColumnLayout {
                        id: col
                        x: 14; y: 14
                        width: parent.width - 28
                        spacing: 6

                        StyledTextLabel {
                            Layout.fillWidth: true
                            text: root.cur.title
                            font: ui.theme.largeBodyBoldFont
                            horizontalAlignment: Text.AlignLeft
                            wrapMode: Text.WordWrap
                        }

                        StyledTextLabel {
                            Layout.fillWidth: true
                            text: root.cur.text
                            font: ui.theme.bodyFont
                            horizontalAlignment: Text.AlignLeft
                            wrapMode: Text.WordWrap
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.topMargin: 6
                            spacing: 8

                            FlatButton {
                                text: qsTrc("appshell", "Skip tour")
                                transparent: true
                                visible: root.step < root.steps.length - 1
                                onClicked: root.finish()
                            }

                            Item { Layout.fillWidth: true }

                            StyledTextLabel {
                                text: (root.step + 1) + " / " + root.steps.length
                                opacity: 0.6
                            }

                            FlatButton {
                                accentButton: true
                                text: root.step < root.steps.length - 1 ? qsTrc("appshell", "Next") : qsTrc("appshell", "Let's go")
                                onClicked: root.next()
                            }
                        }
                    }
                }
            }

            PautaMascot {
                Layout.preferredWidth: 150
                Layout.preferredHeight: 165
                Layout.alignment: Qt.AlignVCenter
                pose: root.cur.pose
            }
        }
    }

    // keyboard: Enter / right arrow = next, left arrow = back, Esc = skip
    focus: visible
    Keys.onEscapePressed: root.finish()
    Keys.onReturnPressed: root.next()
    Keys.onEnterPressed: root.next()
    Keys.onRightPressed: root.next()
    Keys.onLeftPressed: root.back()
}
