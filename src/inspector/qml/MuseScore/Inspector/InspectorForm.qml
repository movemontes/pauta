/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

Rectangle {
    id: root

    property alias model: sectionList.model
    property var notationView: null

    property NavigationSection navigationSection: null
    property int navigationOrderStart: 1

    color: ui.theme.backgroundPrimaryColor

    onVisibleChanged: {
        inspectorListModel.setInspectorVisible(root.visible)
    }

    function focusFirstItem() {
        var item = sectionList.itemAtIndex(0)
        if (item) {
            item.navigationPanel.requestActive()
        }
    }

    InspectorPopupControllerModel {
        id: popupController
    }

    onNotationViewChanged: {
        popupController.setNotationView(root.notationView)
    }

    StyledListView {
        id: sectionList
        anchors.fill: parent

        topMargin: 12
        bottomMargin: 12

        spacing: 12

        interactive: !popupController.isAnyPopupOpen

        function ensureContentVisible(invisibleContentHeight) {
            if (sectionList.contentY + invisibleContentHeight > 0) {
                sectionList.contentY += invisibleContentHeight
            } else {
                sectionList.contentY = 0
            }
        }

        Behavior on contentY {
            NumberAnimation { duration: 250 }
        }

        model: InspectorListModel {
            id: inspectorListModel
        }

        onContentHeightChanged: {
            returnToBounds()

            if (contentHeight > cacheBuffer) {
                cacheBuffer = contentHeight
            }
        }

        onContentYChanged: {
            popupController.repositionPopupIfNeed()
        }

        // Pauta: each section is a cream card with a black outline and a hard shadow, title in caps
        delegate: Item {
            id: delegateItem

            required property AbstractInspectorModel inspectorSectionModel
            required property int index

            width: ListView.view.width
            height: card.height + 4

            property var navigationPanel: _item.navigationPanel

            Rectangle { // hard shadow
                x: card.x + 4
                y: card.y + 4
                width: card.width
                height: card.height
                color: ui.theme.strokeColor
            }

            Rectangle {
                id: card

                x: 10
                width: parent.width - 24
                height: _item.height + 24

                color: ui.theme.textFieldColor
                border.width: 2
                border.color: ui.theme.strokeColor

                Rectangle { // accent strip on the left of the title
                    x: 2
                    y: 12
                    width: 4
                    height: 22
                    color: ui.theme.accentColor
                }

                InspectorSectionDelegate {
                    id: _item

                    y: 12
                    anchors.left: parent.left
                    anchors.leftMargin: 16
                    anchors.right: parent.right
                    anchors.rightMargin: 12

                    titleFont.family: "Pauta Titulo"
                    titleFont.pixelSize: 15
                    titleFont.weight: Font.Black
                    titleFont.capitalization: Font.AllUppercase
                    titleFont.letterSpacing: 0.5

                    sectionModel: delegateItem.inspectorSectionModel
                    anchorItem: root
                    navigationPanel.section: root.navigationSection
                    navigationPanel.order: root.navigationOrderStart + delegateItem.index

                    onEnsureContentVisibleRequested: function(invisibleContentHeight) {
                        sectionList.ensureContentVisible(invisibleContentHeight)
                    }
                }
            }
        }
    }
}
