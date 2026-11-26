/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: wheel / two-finger scrolling moves the list directly, with no glide after the fingers stop.
 */
import QtQuick

WheelHandler {
    id: root

    property Flickable flickable: parent
    property real stepPerNotch: 48

    target: null
    acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad

    onWheel: function(event) {
        var f = root.flickable
        if (!f || !f.interactive) {
            event.accepted = false
            return
        }

        var dy = event.pixelDelta.y !== 0 ? event.pixelDelta.y : event.angleDelta.y / 120 * root.stepPerNotch
        var dx = event.pixelDelta.x !== 0 ? event.pixelDelta.x : event.angleDelta.x / 120 * root.stepPerNotch
        if ((event.modifiers & Qt.ShiftModifier) && dx === 0) {
            dx = dy
            dy = 0
        }

        var moved = false
        if (dy !== 0 && f.contentHeight > f.height) {
            var minY = f.originY - f.topMargin
            var maxY = f.originY + f.contentHeight + f.bottomMargin - f.height
            var y = Math.max(minY, Math.min(maxY, f.contentY - dy))
            if (y !== f.contentY) {
                f.cancelFlick()
                f.contentY = y
                moved = true
            }
        }
        if (dx !== 0 && f.contentWidth > f.width) {
            var minX = f.originX - f.leftMargin
            var maxX = f.originX + f.contentWidth + f.rightMargin - f.width
            var x = Math.max(minX, Math.min(maxX, f.contentX - dx))
            if (x !== f.contentX) {
                f.cancelFlick()
                f.contentX = x
                moved = true
            }
        }
        event.accepted = moved
    }
}
