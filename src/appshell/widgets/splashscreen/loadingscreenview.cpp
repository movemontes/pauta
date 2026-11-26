/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited and others
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

#include "loadingscreenview.h"

#include <QApplication>
#include <QPainter>
#include <QScreen>
#include <QSvgRenderer>

#include "translation.h"
#include "muse_framework_config.h"

using namespace mu::appshell;

// Pauta: Treco-style loading card (white, thick black border, hard blue shadow). No MuseScore logo or website.
static const QString imagePath(":/resources/LoadingScreen.svg"); // kept for the renderer member, not drawn
static constexpr QSize loadingScreenSize(560, 260);

LoadingScreenView::LoadingScreenView(QWidget* parent)
    : QWidget(parent), muse::Contextable(muse::iocCtxForQWidget(this)),
    m_backgroundRenderer(new QSvgRenderer(imagePath, this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    resize(loadingScreenSize);
}

bool LoadingScreenView::event(QEvent* event)
{
    if (event->type() == QEvent::Paint) {
        QPainter painter(this);
        painter.setLayoutDirection(layoutDirection());
        draw(&painter);
    }

    return QWidget::event(event);
}

void LoadingScreenView::draw(QPainter* painter)
{
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    const QColor blue(0, 0, 255), black(0, 0, 0), white(255, 255, 255), gray(92, 92, 92);
    const int shadow = 10, border = 4;
    const QRect card(0, 0, width() - shadow, height() - shadow);

    // hard blue shadow, white card with thick black border
    painter->fillRect(card.translated(shadow, shadow), blue);
    painter->fillRect(card, black);
    painter->fillRect(card.adjusted(border, border, -border, -border), white);

    // app icon: blue square with a white block "P" and black shadow (same drawing as the .icns)
    const QRectF icon(36, 52, 112, 112);
    painter->setPen(Qt::NoPen);
    painter->setBrush(blue);
    painter->drawRoundedRect(icon, 20, 20);
    auto letter = [&](qreal off, const QColor& c) {
        const qreal u = icon.width() / 824.0; // icon art is 824 units wide (100..924 in the 1024 canvas)
        auto r = [&](qreal x0, qreal y0, qreal x1, qreal y1) {
            painter->fillRect(QRectF(icon.left() + (x0 - 100 + off) * u, icon.top() + (y0 - 100 + off) * u,
                                     (x1 - x0) * u, (y1 - y0) * u), c);
        };
        r(300, 272, 458, 752);
        r(300, 272, 700, 392);
        r(580, 272, 724, 592);
        r(300, 472, 700, 592);
    };
    letter(38, black);
    letter(0, white);

    // name, tagline and version
    QFont title(QString::fromStdString(uiConfiguration()->fontFamily()));
    title.setPixelSize(52);
    title.setBold(true);
    painter->setFont(title);
    painter->setPen(black);
    painter->drawText(QRectF(176, 52, card.width() - 200, 64), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("PAUTA"));

    QFont body(QString::fromStdString(uiConfiguration()->fontFamily()));
    body.setPixelSize(15);
    painter->setFont(body);
    painter->setPen(gray);
    painter->drawText(QRectF(178, 118, card.width() - 200, 24), Qt::AlignLeft | Qt::AlignVCenter,
                      muse::qtrc("appshell", "Free sheet music app"));

    body.setBold(true);
    painter->setFont(body);
    painter->setPen(blue);
    painter->drawText(QRectF(178, 146, card.width() - 200, 24), Qt::AlignLeft | Qt::AlignVCenter,
                      muse::qtrc("appshell", "Version %1").arg(QString(PAUTA_VERSION)));
}
