/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: first-run tour. The mascot walks to each part of the window and explains it in one sentence.
 * Each step names a target (objectName of a real item); the mascot stands beside it and points at it.
 * Steps on the score page open it (the current score, or a blank example that is closed at the end).
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
    property bool scorePageShown: false
    property real groupX: 0
    property real groupY: 0
    property bool pointsRight: true
    property bool placed: false
    property int tries: 0

    // side: "left" = mascot left of the target, pointing right; "right" = mascot right of it, pointing left
    readonly property var steps: [
        { pose: "oi", page: "home", target: "", title: qsTrc("appshell", "Hi! I'm Colcheia."),
          text: qsTrc("appshell", "I'll show you around Pauta in a few steps. You can skip at any time.") },
        { pose: "aponta-direita", page: "home", target: "pautaTourNew", side: "left", title: qsTrc("appshell", "Start here"),
          text: qsTrc("appshell", "“New” makes a blank score: pick the instruments and start writing. “Open other…” opens a file you already have.") },
        { pose: "aponta-esquerda", page: "home", target: "pautaTourRail", side: "right", title: qsTrc("appshell", "The side rail"),
          text: qsTrc("appshell", "Your recent scores, extra plugins and the quick guide live here.") },
        { pose: "aponta-direita", page: "score", target: "pautaTool_note-input", side: "left", title: qsTrc("appshell", "Writing notes"),
          text: qsTrc("appshell", "Click “Write” (or press N), pick a note length and type the letters C D E F G A B. C is do, D is re, and so on.") },
        { pose: "aponta-direita", page: "score", target: "pautaTool_chord-text", side: "left", title: qsTrc("appshell", "Tools on the right"),
          text: qsTrc("appshell", "This column has lyrics, chords, slurs, text, tempo, change key and more. Select a note first, then click the tool.") },
        { pose: "aponta-direita", page: "score", target: "pautaTool_play", side: "left", title: qsTrc("appshell", "Listen"),
          text: qsTrc("appshell", "“Play” (or the space bar) plays and pauses. The piano keys light up while it plays.") },
        { pose: "comemorando", page: "score", target: "", title: qsTrc("appshell", "That's it!"),
          text: qsTrc("appshell", "Rest the mouse on any button for a moment and I'll tell you what it does.") }
    ]
    readonly property var cur: steps[step]

    function start() {
        step = 0
        scorePageShown = false
        visible = true
        root.forceActiveFocus()
        enterStep()
    }

    function finish() {
        visible = false
        tour.markSeen()
        if (scorePageShown) {
            tour.showHomePage()
        }
    }

    function next() {
        if (step >= steps.length - 1) {
            finish()
            return
        }
        step++
        enterStep()
    }

    function back() {
        if (step > 0) {
            step--
            enterStep()
        }
    }

    function enterStep() {
        placed = false
        tries = 0
        if (cur.page === "score" && !scorePageShown) {
            scorePageShown = true
            tour.showScorePage()
        }
        placeTimer.restart()
    }

    function findItem(item, name) {
        if (!item) {
            return null
        }
        if (item.objectName === name && item.visible && item.width > 0) {
            return item
        }
        const kids = item.children
        for (let i = 0; i < kids.length; ++i) {
            const found = findItem(kids[i], name)
            if (found) {
                return found
            }
        }
        return null
    }

    function place() {
        const margin = 16
        let target = null
        if (cur.target !== "") {
            target = findItem(ui.rootItem, cur.target)
            if (!target && tries < 25) { // the page may still be opening
                tries++
                placeTimer.restart()
                return
            }
        }

        pointsRight = cur.side !== "right"
        const gw = row.implicitWidth
        const gh = row.implicitHeight
        let x = (root.width - gw) / 2
        let y = (root.height - gh) / 2

        if (target) {
            const r = target.mapToItem(root, 0, 0, target.width, target.height)
            const cy = r.y + r.height / 2
            x = cur.side === "right" ? r.x + r.width + 8 : r.x - gw - 8
            // the pointing hand is about 45% down the mascot
            y = cy - gh * 0.45
        }

        groupX = Math.max(margin, Math.min(x, root.width - gw - margin))
        groupY = Math.max(margin, Math.min(y, root.height - gh - margin))
        placed = true
        enterAnim.restart()
    }

    Timer { id: placeTimer; interval: 200; onTriggered: root.place() }

    PautaTourModel {
        id: tour
        onStartRequested: root.start()
    }

    Component.onCompleted: {
        if (!tour.seen() || tour.debugStep() > 0) {
            startTimer.start()
        }
    }

    Timer {
        id: startTimer
        interval: 1200
        onTriggered: {
            root.start()
            const n = tour.debugStep()
            if (n > 1 && n <= root.steps.length) {
                root.step = n - 1
                root.enterStep()
            }
        }
    }

    onWidthChanged: if (visible && placed) Qt.callLater(place)
    onHeightChanged: if (visible && placed) Qt.callLater(place)

    // dim the window; clicks do not go through
    Rectangle {
        anchors.fill: parent
        color: "#0A0A0A"
        opacity: 0.38
        MouseArea { anchors.fill: parent; hoverEnabled: true; acceptedButtons: Qt.AllButtons; onWheel: function(w) { w.accepted = true } }
    }

    Item {
        id: group

        x: root.groupX
        y: root.groupY
        width: row.implicitWidth
        height: row.implicitHeight
        opacity: root.placed ? 1 : 0

        transform: Translate { id: shift }

        ParallelAnimation {
            id: enterAnim
            NumberAnimation { target: shift; property: "y"; from: 12; to: 0; duration: 200; easing.type: Easing.OutBack }
            NumberAnimation { target: group; property: "opacity"; from: 0; to: 1; duration: 180 }
        }

        RowLayout {
            id: row
            spacing: 6
            // pointing right: bubble, then mascot (its hand points at the target on the right)
            layoutDirection: root.pointsRight ? Qt.LeftToRight : Qt.RightToLeft

            Item {
                Layout.preferredWidth: 330
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
