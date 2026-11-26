/*
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "pautacursor.h"

#include <QCoreApplication>
#include <QCursor>
#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QQuickItem>
#include <QQuickWindow>

using namespace muse::ui;

static QCursor makeArrow()
{
    // Arrow in a 24x24 point box, tip at (3, 2): black body, white edge, hard blue shadow.
    static const QPointF pts[] = { { 3, 2 }, { 3, 19 }, { 7.5, 15 }, { 10.5, 21.5 }, { 13.5, 20.2 }, { 10.6, 14 }, { 16.5, 14 } };
    const qreal scale = 2.0; // sharp on Retina
    QPixmap pix(int(24 * scale), int(24 * scale));
    pix.setDevicePixelRatio(scale);
    pix.fill(Qt::transparent);

    auto path = [&](qreal off) {
        QPainterPath p;
        p.moveTo(pts[0] + QPointF(off, off));
        for (size_t i = 1; i < std::size(pts); ++i) {
            p.lineTo(pts[i] + QPointF(off, off));
        }
        p.closeSubpath();
        return p;
    };

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillPath(path(2), QColor(0, 0, 255));
    QPainterPath main = path(0);
    painter.fillPath(main, Qt::black);
    QPen pen(Qt::white, 1.5);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.strokePath(main, pen);
    painter.end();

    return QCursor(pix, 3, 2);
}

void PautaCursor::install()
{
    static PautaCursor* filter = nullptr;
    if (!filter) {
        filter = new PautaCursor();
        QCoreApplication::instance()->installEventFilter(filter);
    }
}

bool PautaCursor::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::Show || event->type() == QEvent::Expose) {
        if (auto window = qobject_cast<QQuickWindow*>(watched)) {
            QQuickItem* root = window->contentItem();
            if (root && !root->property("pautaCursorSet").toBool()) {
                static const QCursor arrow = makeArrow();
                root->setCursor(arrow); // items with their own pointer (text, resize) keep it
                root->setProperty("pautaCursorSet", true);
            }
        }
    }
    return QObject::eventFilter(watched, event);
}
