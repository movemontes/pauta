/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: study mode. One card with what you need to practice a passage: repeat the selected measures,
 * play them slower and hear a count before starting.
 */
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents

import MuseScore.Playback

StyledPopupView {
    id: root

    property PlaybackToolBarModel playbackModel: null

    property bool hasSelection: false
    property bool showNeedSelection: false

    contentWidth: 300
    contentHeight: contentColumn.implicitHeight

    onOpened: {
        root.hasSelection = root.playbackModel.hasRangeSelection()
        root.showNeedSelection = false
    }

    NavigationPanel {
        id: navPanel
        name: "PautaStudyPopup"
        section: root.navigationSection
        accessible.name: qsTrc("playback", "Study mode")
    }

    ColumnLayout {
        id: contentColumn
        width: root.contentWidth
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Rectangle { // accent strip, like the other Pauta cards
                Layout.preferredWidth: 4
                Layout.preferredHeight: 22
                color: ui.theme.accentColor
            }

            StyledTextLabel {
                Layout.fillWidth: true
                text: qsTrc("playback", "Study mode")
                font.family: "Pauta Titulo"
                font.pixelSize: 18
                font.weight: Font.Black
                font.capitalization: Font.AllUppercase
                horizontalAlignment: Text.AlignLeft
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            text: qsTrc("playback", "Select the measures you want to practice and press the button. They play again and again until you stop.")
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignLeft
            opacity: 0.8
        }

        FlatButton {
            Layout.fillWidth: true
            accentButton: true
            icon: IconCode.LOOP
            orientation: Qt.Horizontal
            text: qsTrc("playback", "Practice the selected measures")

            navigation.panel: navPanel
            navigation.order: 1

            onClicked: {
                if (root.playbackModel.studySelection()) {
                    root.close()
                } else {
                    root.showNeedSelection = true
                }
            }
        }

        StyledTextLabel {
            Layout.fillWidth: true
            visible: root.showNeedSelection || !root.hasSelection
            text: qsTrc("playback", "First select some measures in the score: click the first one and Shift-click the last one.")
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignLeft
            color: root.showNeedSelection ? ui.theme.accentColor : ui.theme.fontPrimaryColor
            opacity: root.showNeedSelection ? 1 : 0.7
        }

        SeparatorLine {
            Layout.fillWidth: true
        }

        StyledTextLabel {
            Layout.fillWidth: true
            text: qsTrc("playback", "Speed")
            font: ui.theme.bodyBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6

            Repeater {
                model: [0.5, 0.75, 1.0]

                FlatButton {
                    required property var modelData
                    required property int index

                    Layout.fillWidth: true
                    orientation: Qt.Horizontal
                    minWidth: 0
                    text: Math.round(modelData * 100) + "%"
                    accentButton: Math.abs(root.playbackModel.tempoMultiplier - modelData) < 0.01

                    navigation.panel: navPanel
                    navigation.order: 2 + index

                    onClicked: {
                        root.playbackModel.tempoMultiplier = modelData
                    }
                }
            }
        }

        SeparatorLine {
            Layout.fillWidth: true
        }

        CheckBox {
            Layout.fillWidth: true
            text: qsTrc("playback", "Repeat the passage")
            checked: root.playbackModel.studyLoopOn

            navigation.panel: navPanel
            navigation.order: 6

            onClicked: {
                root.playbackModel.toggleStudyLoop()
            }
        }

        CheckBox {
            Layout.fillWidth: true
            text: qsTrc("playback", "Count before starting")
            checked: root.playbackModel.studyCountInOn

            navigation.panel: navPanel
            navigation.order: 7

            onClicked: {
                root.playbackModel.toggleStudyCountIn()
            }
        }
    }
}
