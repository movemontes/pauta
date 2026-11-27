/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: MIDI port and channel of each instrument, used by the MIDI export (and by DAWs that read it).
 */
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.AppShell

StyledDialogView {
    id: root

    title: qsTrc("appshell", "MIDI channels")

    contentWidth: 520
    contentHeight: Math.min(560, column.implicitHeight + 40)

    PautaMidiChannelsModel { id: midiModel }

    Component.onCompleted: midiModel.load()

    Column {
        id: column
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        StyledTextLabel {
            width: parent.width
            text: qsTrc("appshell", "Port and channel of each instrument in the exported MIDI file. Two instruments on the same port and channel share the sound in the other program.")
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignLeft
        }

        RowLayout {
            width: parent.width
            StyledTextLabel { Layout.fillWidth: true; text: qsTrc("appshell", "Instrument"); font: ui.theme.bodyBoldFont; horizontalAlignment: Text.AlignLeft }
            StyledTextLabel { Layout.preferredWidth: 90; text: qsTrc("appshell", "Port"); font: ui.theme.bodyBoldFont }
            StyledTextLabel { Layout.preferredWidth: 90; text: qsTrc("appshell", "Channel"); font: ui.theme.bodyBoldFont }
        }

        StyledListView {
            width: parent.width
            height: Math.min(contentHeight, 400)
            spacing: 6
            model: midiModel.rows

            delegate: RowLayout {
                required property var modelData
                width: ListView.view.width

                StyledTextLabel {
                    Layout.fillWidth: true
                    text: modelData.name
                    horizontalAlignment: Text.AlignLeft
                    elide: Text.ElideRight
                }
                IncrementalPropertyControl {
                    Layout.preferredWidth: 90
                    currentValue: modelData.port
                    minValue: 1
                    maxValue: 16
                    step: 1
                    decimals: 0
                    onValueEdited: function(newValue) { midiModel.setPortAndChannel(modelData.index, newValue, modelData.channel) }
                }
                IncrementalPropertyControl {
                    Layout.preferredWidth: 90
                    currentValue: modelData.channel
                    minValue: 1
                    maxValue: 16
                    step: 1
                    decimals: 0
                    onValueEdited: function(newValue) { midiModel.setPortAndChannel(modelData.index, modelData.port, newValue) }
                }
            }
        }

        FlatButton {
            anchors.right: parent.right
            accentButton: true
            text: qsTrc("global", "OK")
            onClicked: root.hide()
        }
    }
}
