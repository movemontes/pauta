/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: pick the program you want to use the score in; Pauta picks the right file format.
 */

import QtQuick

import Muse.Ui
import Muse.UiComponents

StyledDialogView {
    id: root

    title: qsTrc("appshell", "Export to another program")

    contentWidth: 600
    contentHeight: column.implicitHeight + 40

    readonly property var targets: [
        { "name": "Sibelius, Dorico, Finale, Guitar Pro, Flat, Noteflight",
          "info": qsTrc("appshell", "Notation programs. Keeps notes, lyrics, chords and layout (MusicXML)."),
          "type": "musicxml" },
        { "name": "GarageBand, Logic, Ableton, FL Studio, Reaper",
          "info": qsTrc("appshell", "Music production (DAW). Notes and instruments only, no sheet music (MIDI)."),
          "type": "mid" },
        { "name": qsTrc("appshell", "Print, WhatsApp, e-mail"),
          "info": qsTrc("appshell", "Ready-to-read sheet music, not editable (PDF)."),
          "type": "pdf" },
        { "name": qsTrc("appshell", "Video editor, social media, music player"),
          "info": qsTrc("appshell", "The sound of the score (MP3)."),
          "type": "mp3" },
        { "name": qsTrc("appshell", "Braille reader"),
          "info": qsTrc("appshell", "Braille music for blind musicians (BRF)."),
          "type": "brf" }
    ]

    Column {
        id: column
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        StyledTextLabel {
            width: parent.width
            text: qsTrc("appshell", "Where do you want to use this score?")
            font: ui.theme.largeBodyBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        Repeater {
            model: root.targets

            delegate: Rectangle {
                width: column.width
                height: 70
                // Pauta: theme colours so titles stay readable in every colour preset
                color: area.containsMouse ? Qt.tint(ui.theme.textFieldColor, "#335B7A93") : ui.theme.textFieldColor
                border.color: ui.theme.strokeColor
                border.width: 2

                Column {
                    anchors.left: parent.left
                    anchors.leftMargin: 14
                    anchors.right: parent.right
                    anchors.rightMargin: 14
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 3

                    StyledTextLabel {
                        width: parent.width
                        text: modelData.name
                        font: ui.theme.bodyBoldFont
                        color: area.containsMouse ? ui.theme.accentColor : ui.theme.fontPrimaryColor
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideRight
                    }
                    StyledTextLabel {
                        width: parent.width
                        text: modelData.info
                        color: ui.theme.fontPrimaryColor
                        opacity: 0.8
                        horizontalAlignment: Text.AlignLeft
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                    }
                }

                MouseArea {
                    id: area
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        var type = modelData.type
                        root.hide()
                        api.launcher.open("musescore://project/export?presetType=" + type)
                    }
                }
            }
        }
    }
}
