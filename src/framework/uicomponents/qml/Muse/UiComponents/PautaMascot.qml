/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: the mascot (Colcheia), drawn in the current accent color.
 * Poses: normal, oi, aponta-esquerda, aponta-direita, pensando, surpreso, comemorando, triste, bravo, regendo, dormindo, rosto
 */
import QtQuick

import Muse.Ui

Image {
    id: root

    property string pose: "normal"

    source: "image://pautamascote/" + pose + "/" + String(ui.theme.accentColor).substring(1, 7)
    sourceSize.width: width * 2
    sourceSize.height: height * 2
    fillMode: Image.PreserveAspectFit
    smooth: true
    mipmap: true
    asynchronous: false
}
