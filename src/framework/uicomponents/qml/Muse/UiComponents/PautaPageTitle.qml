/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta (Treco look): big wide title with a light accent "highlighter" behind its lower half,
 * and an optional short sentence under it.
 */
import QtQuick
import QtQuick.Layouts

import Muse.Ui

ColumnLayout {
    id: root

    property string text: ""
    property string lead: ""

    spacing: 4

    Item {
        Layout.preferredWidth: titleLabel.implicitWidth + 4
        Layout.preferredHeight: titleLabel.implicitHeight

        Rectangle { // highlighter
            x: 0
            width: parent.width
            height: Math.round(titleLabel.implicitHeight * 0.36)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: Math.round(titleLabel.implicitHeight * 0.06)
            color: Qt.tint(ui.theme.textFieldColor, Qt.rgba(ui.theme.accentColor.r, ui.theme.accentColor.g, ui.theme.accentColor.b, 0.28))
        }

        StyledTextLabel {
            id: titleLabel
            x: 2
            text: root.text
            font.family: ui.theme.titleBoldFont.family
            font.capitalization: Font.AllUppercase
            font.pixelSize: 42
            horizontalAlignment: Text.AlignLeft
        }
    }

    StyledTextLabel {
        Layout.fillWidth: true
        visible: root.lead !== ""
        text: root.lead
        font: ui.theme.largeBodyFont
        opacity: 0.8
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
        maximumLineCount: 2
    }
}
