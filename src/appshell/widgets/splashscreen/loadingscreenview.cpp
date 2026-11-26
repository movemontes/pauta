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

#include <QImage>

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

    const QColor slate(0x7D, 0x92, 0x9E), black(0x1B, 0x1A, 0x1F), beige(0xD3, 0xCB, 0xBE), gray(0x4F, 0x5F, 0x6E);
    const QColor blue = gray, white = beige; // used below for text colours
    const int shadow = 8, border = 3;
    const QRect card(0, 0, width() - shadow, height() - shadow);

    // Pauta (pixel look): beige card, black outline, hard slate shadow
    painter->fillRect(card.translated(shadow, shadow), slate);
    painter->fillRect(card, black);
    painter->fillRect(card.adjusted(border, border, -border, -border), beige);

    // pixel-art app icon, kept sharp
    static const QImage icon(":/resources/pauta_icon.png");
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter->drawImage(QRect(36, 52, 112, 112), icon);

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
