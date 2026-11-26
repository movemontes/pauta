/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: vertical column of tools, always visible (like Aseprite's tool bar).
 */
import QtQuick

import Muse.Ui
import Muse.UiComponents

import MuseScore.AppShell

Item {
    id: root

    property NavigationSection navigationSection: null
    property int navigationOrder: 1

    implicitWidth: 108
    implicitHeight: column.implicitHeight

    PautaToolsModel {
        id: toolsModel
        Component.onCompleted: toolsModel.load()
    }

    NavigationPanel {
        id: navPanel
        name: "PautaTools"
        section: root.navigationSection
        order: root.navigationOrder
        direction: NavigationPanel.Vertical
    }

    StyledFlickable {
        anchors.fill: parent
        contentHeight: column.implicitHeight

        Column {
            id: column
            width: root.width
            spacing: 4
            topPadding: 4
            bottomPadding: 4

            Repeater {
                model: toolsModel

                delegate: StyledToolBarItem {
                    required property var item
                    required property int index

                    itemData: item
                    width: column.width - 8
                    x: 4
                    height: 66
                    orientation: Qt.Vertical
                    textFont: ui.theme.bodyFont
                    maximumLineCount: 2
                    navigation.panel: navPanel
                    navigation.row: index
                }
            }
        }
    }
}
