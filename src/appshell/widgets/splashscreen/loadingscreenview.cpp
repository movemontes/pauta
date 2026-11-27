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
#include <QFile>
#include <QSettings>

#include "translation.h"
#include "muse_framework_config.h"

using namespace mu::appshell;

// Pauta: Treco-style loading card (white, thick black border, hard blue shadow). No MuseScore logo or website.
static const QString imagePath(":/resources/LoadingScreen.svg"); // kept for the renderer member, not drawn
static constexpr QSize loadingScreenSize(640, 290);

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

        // Pauta test hook: PAUTA_DEBUG_SPLASH=<file.png> saves the loading card once
        static bool saved = false;
        if (!saved && qEnvironmentVariableIsSet("PAUTA_DEBUG_SPLASH")) {
            saved = true;
            QImage img(size() * 2, QImage::Format_ARGB32_Premultiplied);
            img.setDevicePixelRatio(2);
            img.fill(Qt::transparent);
            QPainter p2(&img);
            draw(&p2);
            p2.end();
            img.save(qEnvironmentVariable("PAUTA_DEBUG_SPLASH"));
        }
    }

    return QWidget::event(event);
}

void LoadingScreenView::draw(QPainter* painter)
{
    // Pauta: Treco look. Cream card, black outline, hard black shadow, a stripe of the accent colour,
    // Colcheia conducting and the name in the wide title font.
    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

    const QString textFamily = QString::fromStdString(uiConfiguration()->fontFamily()); // also loads Pauta's fonts
    const QColor ink(0x0A, 0x0A, 0x0A), cream(0xFD, 0xF9, 0xEE);
    QColor accent(QSettings().value("pauta/accent", "#FF4A00").toString());
    if (!accent.isValid()) {
        accent = QColor("#FF4A00");
    }
    const int shadow = 10, border = 4;
    const QRect card(0, 0, width() - shadow, height() - shadow);

    painter->fillRect(card.translated(shadow, shadow), ink);
    painter->fillRect(card, ink);
    painter->fillRect(card.adjusted(border, border, -border, -border), cream);
    painter->fillRect(QRect(border, card.height() - border - 18, card.width() - 2 * border, 18), accent);
    painter->fillRect(QRect(border, card.height() - border - 20, card.width() - 2 * border, 2), ink);

    // Colcheia conducting, in the accent colour
    QFile f(":/resources/mascote/mascote-regendo.svg");
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray svg = f.readAll();
        svg.replace("#0000FF", accent.name(QColor::HexRgb).toUpper().toUtf8());
        svg.replace("#0000ff", accent.name(QColor::HexRgb).toUtf8());
        QSvgRenderer mascot(svg);
        mascot.render(painter, QRectF(26, 28, 230, 205));
    }

    QFont title("Pauta Titulo");
    title.setPixelSize(70);
    painter->setFont(title);
    painter->setPen(ink);
    painter->drawText(QRectF(276, 52, card.width() - 300, 84), Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("PAUTA"));

    QFont body(textFamily);
    body.setPixelSize(17);
    painter->setFont(body);
    painter->drawText(QRectF(280, 138, card.width() - 300, 26), Qt::AlignLeft | Qt::AlignVCenter,
                      muse::qtrc("appshell", "Free sheet music app"));

    body.setBold(true);
    body.setPixelSize(15);
    painter->setFont(body);
    painter->drawText(QRectF(280, 168, card.width() - 300, 24), Qt::AlignLeft | Qt::AlignVCenter,
                      muse::qtrc("appshell", "Version %1").arg(QString(PAUTA_VERSION)));
}
