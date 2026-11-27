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
import QtQuick
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.Project

import "internal/NewScore"

StyledDialogView {
    id: root

    title: qsTrc("project", "New score")

    contentHeight: 600
    contentWidth: 1024
    resizable: true

    objectName: "NewScoreDialog"

    // Pauta: a ready-made start chosen on the first page (empty = instruments chosen one by one)
    property string quickTemplate: ""

    function onDone() {
        if (root.quickTemplate !== "") {
            var quick = scoreInfoPage.result()
            quick["templatePath"] = root.quickTemplate
            if (newScoreModel.createScore(quick)) {
                root.activateParentOnClose = false
                root.accept()
            }
            return
        }

        var result = {}

        var instrumentsAndTemplatePageResult = chooseInstrumentsAndTemplatePage.result()
        for (var key in instrumentsAndTemplatePageResult) {
            result[key] = instrumentsAndTemplatePageResult[key]
        }

        var scoreInfoPageResult = scoreInfoPage.result()
        for (key in scoreInfoPageResult) {
            result[key] = scoreInfoPageResult[key]
        }

        if (newScoreModel.createScore(result)) {
            root.activateParentOnClose = false
            root.accept()
        }
    }

    onNavigationActivateRequested: {
        chooseInstrumentsAndTemplatePage.focusOnSelected()
    }

    NewScoreModel {
        id: newScoreModel
    }

    Item {
        id: popupsAnchorItem
        anchors.fill: parent
        anchors.margins: 8
    }

    StackLayout {
        id: pagesStack

        anchors.top: parent.top
        anchors.topMargin: 12
        anchors.left: parent.left
        anchors.leftMargin: 12
        anchors.right: parent.right
        anchors.rightMargin: 12
        anchors.bottom: footer.top
        anchors.bottomMargin: 20

        // Pauta: page 0, ready-made starts with Colcheia
        PautaQuickStartPage {
            templatesDir: newScoreModel.pautaTemplatesDir()

            onModelChosen: function(templatePath, name) {
                root.quickTemplate = templatePath
                pagesStack.currentIndex = 2
            }

            onChooseInstruments: {
                root.quickTemplate = ""
                pagesStack.currentIndex = 1
            }
        }

        ChooseInstrumentsAndTemplatesPage {
            id: chooseInstrumentsAndTemplatePage

            navigationSection: root.navigationSection

            Component.onCompleted: {
                preferredScoreCreationMode = newScoreModel.preferredScoreCreationMode()
            }

            onDone: {
                root.onDone()
            }
        }

        ScoreInfoPage {
            id: scoreInfoPage

            navigationSection: root.navigationSection
            popupsAnchorItem: popupsAnchorItem
        }
    }

    RowLayout {
        id: footer

        anchors.left: parent.left
        anchors.leftMargin: 24
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16

        spacing: 12

        StyledTextLabel {
            id: descriptionLabel
            text: pagesStack.currentIndex === 1
                  ? chooseInstrumentsAndTemplatePage.description
                  : ""
            displayTruncatedTextOnHover: true

            Layout.fillWidth: true
            Layout.maximumHeight: buttonBox.height

            font: ui.theme.bodyFont
            opacity: 0.7
            horizontalAlignment: Text.AlignLeft
            wrapMode: Text.Wrap
        }

        ButtonBox {
            id: buttonBox

            Layout.preferredWidth: implicitWidth

            navigationPanel.name: "BottomPanel"
            navigationPanel.section: root.navigationSection
            navigationPanel.order: 100

            FlatButton {
                text: qsTrc("global", "Cancel")
                buttonRole: ButtonBoxModel.CustomRole
                buttonId: ButtonBoxModel.CustomButton + 1

                onClicked: {
                    root.reject()
                }
            }

            FlatButton {
                text: qsTrc("global", "Back")
                buttonRole: ButtonBoxModel.CustomRole
                buttonId: ButtonBoxModel.CustomButton + 2
                visible: pagesStack.currentIndex > 0

                onClicked: {
                    // Pauta: from the score info, go back to where the start was chosen
                    if (pagesStack.currentIndex === 2 && root.quickTemplate !== "") {
                        pagesStack.currentIndex = 0
                    } else {
                        pagesStack.currentIndex--
                    }
                }
            }

            FlatButton {
                text: qsTrc("global", "Next")
                buttonRole: ButtonBoxModel.CustomRole
                buttonId: ButtonBoxModel.CustomButton + 3
                visible: pagesStack.currentIndex === 1
                enabled: chooseInstrumentsAndTemplatePage.hasSelection

                onClicked: {
                    pagesStack.currentIndex++
                }
            }

            FlatButton {
                text: qsTrc("global", "Done")
                buttonRole: ButtonBoxModel.AcceptRole
                buttonId: ButtonBoxModel.Done
                accentButton: true
                visible: pagesStack.currentIndex > 0
                enabled: pagesStack.currentIndex === 2 || chooseInstrumentsAndTemplatePage.hasSelection

                onClicked: {
                    root.onDone()
                }
            }
        }
    }
}
