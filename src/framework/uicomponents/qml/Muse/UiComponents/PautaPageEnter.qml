/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta (Treco look): when a section opens, its content slides in about 10 px and fades in (180 ms).
 * Usage: PautaPageEnter { target: root }
 */
import QtQuick

QtObject {
    id: root

    property Item target: null

    readonly property Translate shift: Translate { x: 10 }

    readonly property ParallelAnimation anim: ParallelAnimation {
        NumberAnimation { target: root.shift; property: "x"; from: 10; to: 0; duration: 180; easing.type: Easing.OutCubic }
        NumberAnimation { target: root.target; property: "opacity"; from: 0; to: 1; duration: 180; easing.type: Easing.OutCubic }
    }

    onTargetChanged: {
        if (target) {
            target.transform = [shift]
            anim.restart()
        }
    }
}
