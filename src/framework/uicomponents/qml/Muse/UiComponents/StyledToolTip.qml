/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
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

StyledPopupView {
    id: root

    property alias title: titleLabel.text
    property alias description: descriptionLabel.text
    property string shortcut: ""

    // Pauta: the mascot sits next to the tip (Help > Mascot in tips turns it off)
    readonly property bool pautaMascot: ui.tooltip.pautaMascot()

    contentWidth: Math.min(content.implicitWidth, (pautaMascot ? 360 : 300) - margins * 2)
    contentHeight: Math.max(content.implicitHeight, pautaMascot ? 46 : 0)

    x: root.parent.width / 2 - (contentWidth + padding * 2 + margins * 2) / 2
    y: root.parent.height

    padding: 8
    margins: 8

    openPolicies: PopupView.NoActivateFocus

    //! NOTE: No navigation needed for tooltip
    navigationSection: null

    RowLayout {
        id: content

        anchors.fill: parent
        spacing: 10

        PautaMascot {
            Layout.preferredWidth: 40
            Layout.preferredHeight: 46
            Layout.alignment: Qt.AlignTop
            visible: root.pautaMascot
            pose: Boolean(root.description) ? "pensando" : "normal"
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            spacing: 4

            RowLayout {
                id: row
                Layout.fillWidth: true

                spacing: 6

                StyledTextLabel {
                    id: titleLabel
                    Layout.fillWidth: true

                    font: ui.theme.bodyBoldFont
                    horizontalAlignment: Text.AlignLeft
                    wrapMode: Text.WordWrap
                }

                StyledTextLabel {
                    id: shortcutLabel

                    text: "(" + root.shortcut + ")"
                    horizontalAlignment: Text.AlignRight
                    opacity: 0.8

                    visible: Boolean(root.shortcut)
                }
            }

            StyledTextLabel {
                id: descriptionLabel
                Layout.fillWidth: true

                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.Wrap

                visible: Boolean(root.description)
            }
        }
    }
}
