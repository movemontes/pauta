/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: first-run thank-you note with the Pix key (same message as Treco's).
 */

import QtQuick

import Muse.Ui
import Muse.UiComponents

StyledDialogView {
    id: root

    title: "Pauta"

    contentWidth: 520
    contentHeight: content.implicitHeight + 48

    readonly property string pixKey: "e2a1f4a9-ebfe-4c3c-a531-92a0e7433f14"
    property bool copied: false

    Column {
        id: content

        anchors.fill: parent
        anchors.margins: 24
        spacing: 12

        StyledTextLabel {
            width: parent.width
            text: qsTrc("appshell/welcome", "HELP ME: I'VE GOT 5 MOUTHS TO FEED")
            font.family: ui.theme.headerBoldFont.family
            font.pixelSize: 24
            font.bold: true
            color: "#E10000"
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignLeft
        }

        StyledTextLabel {
            width: parent.width
            text: qsTrc("appshell/welcome", "and I made this app for free, for you.")
            font: ui.theme.largeBodyBoldFont
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignLeft
        }

        StyledTextLabel {
            width: parent.width
            topPadding: 6
            text: qsTrc("appshell/welcome", "Hi, I'm Mateus. Pauta has no ads, needs no account and never uploads your scores.")
                  + "\n\n"
                  + qsTrc("appshell/welcome", "If it makes your music easier, any amount helps my family a lot. Thank you so much!")
            font: ui.theme.bodyFont
            color: "#5C5C5C"
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignLeft
        }

        Column {
            width: parent.width
            spacing: 4
            topPadding: 6

            StyledTextLabel {
                text: qsTrc("appshell/welcome", "Pix (random key, PicPay):")
                font: ui.theme.bodyBoldFont
            }

            StyledTextLabel {
                text: root.pixKey
                font: ui.theme.bodyFont
                color: ui.theme.accentColor
            }
        }

        Item {
            width: parent.width
            height: 40

            NavigationPanel {
                id: buttonsPanel
                name: "PautaWelcomeButtons"
                section: root.navigationSection
                direction: NavigationPanel.Horizontal
                order: 1
            }

            FlatButton {
                id: laterButton
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                width: 150
                height: 36
                text: qsTrc("appshell/welcome", "Let's go")
                navigation.panel: buttonsPanel
                navigation.column: 0
                onClicked: root.accept()
            }

            FlatButton {
                id: pixButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                width: 210
                height: 36
                accentButton: true
                text: root.copied ? qsTrc("appshell/welcome", "Copied!") : qsTrc("appshell/welcome", "Copy Pix key")
                navigation.panel: buttonsPanel
                navigation.column: 1
                onClicked: {
                    clipHelper.copyText(root.pixKey)
                    root.copied = true
                }
            }

            TextEdit {
                id: clipHelper
                visible: false
                function copyText(t) { text = t; selectAll(); copy(); }
            }
        }
    }
}
