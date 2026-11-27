/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: first page of "New score". Colcheia asks what you are going to write, and big cards (with a picture of
 * the score) start from a ready-made model. "Choose the instruments myself" opens the full list.
 */
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

Item {
    id: root

    // folder of the models that ship with the app (…/Resources/templates)
    property string templatesDir: ""

    signal modelChosen(string templatePath, string name)
    signal chooseInstruments()

    // the models: folder inside templates/, name and a short line
    readonly property var models: [
        { "dir": "04-Solo/04-Piano", "name": qsTrc("project", "Piano"), "info": qsTrc("project", "Two staves, left and right hand") },
        { "dir": "04-Solo/02-Guitar_+_Tablature", "name": qsTrc("project", "Guitar"), "info": qsTrc("project", "Staff and tablature together") },
        { "dir": "05-Jazz/01-Jazz_Lead_Sheet", "name": qsTrc("project", "Melody with chords"), "info": qsTrc("project", "One line of melody, chords on top") },
        { "dir": "02-Choral/07-Voice_+_Piano", "name": qsTrc("project", "Voice and piano"), "info": qsTrc("project", "A song with accompaniment") },
        { "dir": "02-Choral/01-SATB", "name": qsTrc("project", "Choir"), "info": qsTrc("project", "Soprano, alto, tenor and bass") },
        { "dir": "06-Popular/01-Rock_Band", "name": qsTrc("project", "Band"), "info": qsTrc("project", "Guitar, bass, keys and drums") },
        { "dir": "03-Chamber_Music/01-String_Quartet", "name": qsTrc("project", "String quartet"), "info": qsTrc("project", "Two violins, viola and cello") },
        { "dir": "08-Orchestral/01-Classical_Orchestra", "name": qsTrc("project", "Orchestra"), "info": qsTrc("project", "Winds, brass, timpani and strings") },
        { "dir": "01-General/01-Treble_Clef", "name": qsTrc("project", "Blank staff"), "info": qsTrc("project", "Just one staff to start") }
    ]

    function fileOf(dir) {
        const parts = dir.split("/")
        return root.templatesDir + "/" + dir + "/" + parts[parts.length - 1] + ".mscx"
    }

    Row {
        id: header
        spacing: 12

        PautaMascot {
            width: 84
            height: 92
            pose: "oi"
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 4

            StyledTextLabel {
                text: qsTrc("project", "What are you going to write?")
                font.family: "Pauta Titulo"
                font.pixelSize: 26
                horizontalAlignment: Text.AlignLeft
            }
            StyledTextLabel {
                text: qsTrc("project", "Pick a ready-made start. You can add or remove instruments later.")
                opacity: 0.75
                horizontalAlignment: Text.AlignLeft
            }
        }
    }

    GridView {
        id: grid

        anchors.top: header.bottom
        anchors.topMargin: 14
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        cellWidth: Math.floor(width / 5) // 10 cards in two rows
        cellHeight: 214
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        model: root.models.length + 1

        delegate: Item {
            id: cell

            required property int index
            readonly property bool isOwn: index === root.models.length
            readonly property var m: isOwn ? null : root.models[index]

            width: grid.cellWidth
            height: grid.cellHeight

            Rectangle { // hard shadow
                x: 9; y: 9
                width: card.width; height: card.height
                color: ui.theme.strokeColor
            }

            Rectangle {
                id: card
                x: 6; y: 6
                width: parent.width - 18
                height: parent.height - 18
                color: area.containsMouse ? "#FFFFFF" : ui.theme.textFieldColor
                border.width: 2
                border.color: ui.theme.strokeColor

                Rectangle { // picture of the score
                    id: thumbBox
                    x: 10; y: 10
                    width: parent.width - 20
                    height: 110
                    color: "#FFFFFF"
                    border.width: 1
                    border.color: "#33000000"
                    clip: true

                    Image {
                        anchors.fill: parent
                        anchors.margins: 1
                        visible: !cell.isOwn
                        source: cell.isOwn ? "" : "file://" + root.templatesDir + "/" + cell.m.dir + "/Thumbnails/thumbnail.png"
                        fillMode: Image.PreserveAspectCrop
                        verticalAlignment: Image.AlignTop
                        smooth: true
                        asynchronous: true
                    }

                    StyledIconLabel {
                        anchors.centerIn: parent
                        visible: cell.isOwn
                        iconCode: IconCode.PLUS
                        font.pixelSize: 40
                    }
                }

                StyledTextLabel {
                    x: 12
                    anchors.top: thumbBox.bottom
                    anchors.topMargin: 8
                    width: parent.width - 24
                    text: cell.isOwn ? qsTrc("project", "Choose the instruments myself") : cell.m.name
                    font: ui.theme.largeBodyBoldFont
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideRight
                }

                StyledTextLabel {
                    x: 12
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 10
                    width: parent.width - 24
                    text: cell.isOwn ? qsTrc("project", "Any combination, from the full list") : cell.m.info
                    opacity: 0.7
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap
                    maximumLineCount: 2
                }

                MouseArea {
                    id: area
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (cell.isOwn) {
                            root.chooseInstruments()
                        } else {
                            root.modelChosen(root.fileOf(cell.m.dir), cell.m.name)
                        }
                    }
                }
            }
        }
    }
}
