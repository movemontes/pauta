/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: "Start from a model" strip on the home page. One click makes a score from a ready-made model.
 */
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

Item {
    id: root

    implicitHeight: 196

    NewScoreModel { id: newScore }

    readonly property string dir: newScore.pautaTemplatesDir()

    readonly property var models: [
        { "dir": "04-Solo/04-Piano", "name": qsTrc("project", "Piano") },
        { "dir": "04-Solo/02-Guitar_+_Tablature", "name": qsTrc("project", "Guitar") },
        { "dir": "05-Jazz/01-Jazz_Lead_Sheet", "name": qsTrc("project", "Melody with chords") },
        { "dir": "02-Choral/07-Voice_+_Piano", "name": qsTrc("project", "Voice and piano") },
        { "dir": "02-Choral/01-SATB", "name": qsTrc("project", "Choir") },
        { "dir": "06-Popular/01-Rock_Band", "name": qsTrc("project", "Band") },
        { "dir": "03-Chamber_Music/01-String_Quartet", "name": qsTrc("project", "String quartet") },
        { "dir": "08-Orchestral/01-Classical_Orchestra", "name": qsTrc("project", "Orchestra") }
    ]

    StyledTextLabel {
        id: heading
        text: qsTrc("project", "Start from a model")
        font: ui.theme.largeBodyBoldFont
        horizontalAlignment: Text.AlignLeft
    }

    ListView {
        id: strip

        anchors.top: heading.bottom
        anchors.topMargin: 10
        anchors.left: parent.left
        anchors.right: parent.right
        height: 160

        orientation: ListView.Horizontal
        spacing: 16
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        model: root.models

        delegate: Item {
            id: cell

            required property var modelData

            width: 150
            height: 156

            Rectangle { x: 4; y: 4; width: card.width; height: card.height; color: ui.theme.strokeColor }

            Rectangle {
                id: card
                width: 146
                height: 150
                color: area.containsMouse ? Qt.tint(ui.theme.textFieldColor, Qt.rgba(ui.theme.accentColor.r, ui.theme.accentColor.g, ui.theme.accentColor.b, 0.15)) : ui.theme.textFieldColor // Pauta: a hint of the accent on hover, readable in every theme
                border.width: 2
                border.color: ui.theme.strokeColor

                Rectangle {
                    id: thumb
                    x: 8; y: 8
                    width: parent.width - 16
                    height: 100
                    color: "#FFFFFF"
                    border.width: 1
                    border.color: "#33000000"
                    clip: true

                    Image {
                        anchors.fill: parent
                        anchors.margins: 1
                        source: "file://" + root.dir + "/" + cell.modelData.dir + "/Thumbnails/thumbnail.png"
                        fillMode: Image.PreserveAspectCrop
                        verticalAlignment: Image.AlignTop
                        smooth: true
                        asynchronous: true
                    }
                }

                StyledTextLabel {
                    anchors.top: thumb.bottom
                    anchors.topMargin: 8
                    x: 9
                    width: parent.width - 18
                    text: cell.modelData.name
                    font: ui.theme.bodyBoldFont
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideRight
                }

                MouseArea {
                    id: area
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        const parts = cell.modelData.dir.split("/")
                        newScore.pautaCreateFromTemplate(root.dir + "/" + cell.modelData.dir + "/" + parts[parts.length - 1] + ".mscx",
                                                         qsTrc("project", "Untitled score"))
                    }
                }
            }
        }
    }
}
