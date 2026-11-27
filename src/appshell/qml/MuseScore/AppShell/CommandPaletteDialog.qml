/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: command palette. Type part of a command name, use the arrows, press Enter.
 */

import QtQuick

import Muse.Ui
import Muse.UiComponents

import MuseScore.AppShell

StyledDialogView {
    id: root

    title: qsTrc("appshell", "Command palette…").replace("…", "")

    contentWidth: 560
    contentHeight: 420

    CommandPaletteModel {
        id: paletteModel
    }

    onOpened: {
        paletteModel.load()
        list.currentIndex = 0
        searchField.ensureActiveFocus()
    }

    // test hook (see CommandPaletteModel::pautaDebugTypeText): types a text letter by letter
    property string pautaTypeText: paletteModel.pautaDebugTypeText()
    property int pautaTyped: 0
    Timer {
        interval: 260
        repeat: true
        running: root.pautaTypeText.length > 0 && root.pautaTyped < root.pautaTypeText.length
        onTriggered: {
            root.pautaTyped++
            searchField.inputField.text = root.pautaTypeText.substring(0, root.pautaTyped)
        }
    }

    function runCurrent() {
        if (list.count === 0) {
            return
        }
        var row = Math.max(0, list.currentIndex)
        root.hide()
        paletteModel.trigger(row)
    }

    Column {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        TextInputField {
            id: searchField
            width: parent.width
            height: 40
            hint: qsTrc("appshell", "Type a command")
            navigation.panel: NavigationPanel {
                name: "PautaCommandSearch"
                section: root.navigationSection
                order: 0
            }

            onTextChanged: function(newTextValue) {
                paletteModel.filter = newTextValue
                list.currentIndex = 0
            }
            onAccepted: root.runCurrent()
            onEscaped: root.hide()

            Keys.onDownPressed: list.incrementCurrentIndex()
            Keys.onUpPressed: list.decrementCurrentIndex()
        }

        StyledListView {
            id: list
            width: parent.width
            height: parent.height - searchField.height - parent.spacing
            model: paletteModel
            clip: true
            keyNavigationWraps: true
            highlightMoveDuration: 0

            delegate: Rectangle {
                width: ListView.view.width
                height: 36
                color: ListView.isCurrentItem ? ui.theme.accentColor : (mouse.containsMouse ? Qt.tint(ui.theme.backgroundPrimaryColor, "#335B7A93") : "transparent")

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    anchors.right: shortcutLabel.left
                    anchors.verticalCenter: parent.verticalCenter
                    horizontalAlignment: Text.AlignLeft
                    text: model.title
                    font: ui.theme.bodyBoldFont
                    color: parent.ListView.isCurrentItem ? "#FFFFFF" : ui.theme.fontPrimaryColor
                }

                StyledTextLabel {
                    id: shortcutLabel
                    anchors.right: parent.right
                    anchors.rightMargin: 12
                    anchors.verticalCenter: parent.verticalCenter
                    text: model.shortcut
                    color: parent.ListView.isCurrentItem ? "#FFFFFF" : ui.theme.fontPrimaryColor
                    opacity: parent.ListView.isCurrentItem ? 1.0 : 0.8
                }

                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        list.currentIndex = index
                        root.runCurrent()
                    }
                }
            }
        }
    }
}
