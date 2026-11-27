/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: the top toolbar of the score page: boxes by subject, icons only (the name shows on hover).
 * Treco look: cream boxes, black outline, hard shadow. No titles: the boxes themselves show the groups.
 */
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.AppShell

Item {
    id: root

    implicitWidth: row.implicitWidth + 12
    implicitHeight: 48

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "PautaDeck"
        enabled: root.enabled && root.visible
        accessible.name: qsTrc("appshell", "Tools")
    }

    PautaDeckModel { id: deck }

    Component.onCompleted: deck.load()

    Row {
        id: row
        x: 6
        y: 3
        spacing: 10

        Repeater {
            model: deck.categories

            delegate: Item {
                id: box

                required property var modelData
                required property int index

                width: card.width + 3
                height: card.height + 3

                Rectangle { x: 3; y: 3; width: card.width; height: card.height; color: ui.theme.strokeColor }

                Rectangle {
                    id: card
                    width: buttons.width + 10
                    height: 40
                    color: ui.theme.textFieldColor
                    border.width: 2
                    border.color: ui.theme.strokeColor

                    Row {
                        id: buttons
                        x: 5
                        y: 4
                        spacing: 1

                        Repeater {
                            model: box.modelData.count

                            delegate: FlatButton {
                                id: btn

                                required property int index
                                readonly property var item: deck.items[box.modelData.start + index]

                                width: 32
                                height: 32

                                icon: btn.item ? btn.item.icon : IconCode.NONE
                                enabled: btn.item ? btn.item.enabled : false
                                accentButton: btn.item ? btn.item.checked : false
                                transparent: !accentButton

                                toolTipTitle: btn.item ? btn.item.title : ""
                                toolTipDescription: btn.item ? btn.item.description : ""
                                toolTipShortcut: btn.item ? btn.item.shortcuts : ""

                                navigation.panel: root.navigationPanel
                                navigation.order: box.modelData.start + index
                                navigation.name: btn.item ? btn.item.code : ""

                                onClicked: {
                                    if (btn.item) {
                                        deck.run(btn.item.code)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
