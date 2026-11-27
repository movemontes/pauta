/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
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

#include "page.h"

#ifndef ENGRAVING_NO_ACCESSIBILITY
#include "accessibility/accessibleitem.h"
#endif

#include "factory.h"
#include "measurebase.h"
#include "mscore.h"
#include "score.h"
#include "system.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

Page::Page(RootItem* parent)
    : EngravingItem(ElementType::PAGE, parent, ElementFlag::NOT_SELECTABLE)
{
    m_bspTreeValid = false;
}

//---------------------------------------------------------
//   items
//---------------------------------------------------------

std::vector<EngravingItem*> Page::items(const RectF& rect)
{
    // Pauta: in continuous view the whole score is one very long page, and every edit made the next paint rebuild
    // the index of all its items (two full scans, ~80 ms in a big score) just to find the few on screen. There we
    // now look only at the measures near the visible area (with a wide margin for long texts) and keep the same
    // test the index uses (item rect intersects the area).
    if (score()->linearMode()) {
        std::vector<EngravingItem*> found;
        auto take = [&](EngravingItem* item) {
            if (item->collectForDrawing() && item->pageBoundingRect().intersects(rect)) {
                found.push_back(item);
            }
        };
        const double margin = 80.0 * spatium();
        for (System* s : m_systems) {
            for (MeasureBase* m : s->measures()) {
                const double x = m->pagePos().x();
                if (x + m->width() + margin < rect.left() || x - margin > rect.right()) {
                    continue;
                }
                m->scanElements(take);
            }
            s->scanElements(take);
        }
        take(this);
        for (int i = 0; i < MAX_HEADERS; i++) {
            if (Text* t = headerText(i)) {
                take(t);
            }
        }
        for (int i = 0; i < MAX_FOOTERS; i++) {
            if (Text* t = footerText(i)) {
                take(t);
            }
        }
        return found;
    }

    if (!m_bspTreeValid) {
        doRebuildBspTree();
    }
    return bspTree.items(rect);
}

std::vector<EngravingItem*> Page::items(const PointF& point)
{
    if (!m_bspTreeValid) {
        doRebuildBspTree();
    }
    return bspTree.items(point);
}

//---------------------------------------------------------
//   appendSystem
//---------------------------------------------------------

void Page::appendSystem(System* s)
{
    s->moveToPage(this);
    m_systems.push_back(s);
}

#ifndef ENGRAVING_NO_ACCESSIBILITY
AccessibleItemPtr Page::createAccessible()
{
    return std::make_shared<AccessibleItem>(this, AccessibleItem::Group);
}

#endif

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Page::scanElements(std::function<void(EngravingItem*)> func)
{
    for (System* s :m_systems) {
        for (MeasureBase* m : s->measures()) {
            m->scanElements(func);
        }
        s->scanElements(func);
    }

    func(this);

    for (int i = 0; i < MAX_HEADERS; i++) {
        if (Text* t = headerText(i)) {
            func(t);
        }
    }
    for (int i = 0; i < MAX_FOOTERS; i++) {
        if (Text* t = footerText(i)) {
            func(t);
        }
    }
}

//---------------------------------------------------------
//   doRebuildBspTree
//---------------------------------------------------------

void Page::doRebuildBspTree()
{
    int n = 0;
    auto countElements = [&](EngravingItem* item) {
        if (item->collectForDrawing()) {
            ++n;
        }
    };

    scanElements(countElements);

    RectF r;
    if (score()->linearMode()) {
        double w = 0.0;
        double h = 0.0;
        if (!m_systems.empty()) {
            h = m_systems.front()->height();
            if (!m_systems.front()->measures().empty()) {
                MeasureBase* mb = m_systems.front()->measures().back();
                w = mb->x() + mb->width();
            }
        }
        r = RectF(0.0, 0.0, w, h);
    } else {
        r = pageBoundingRect();
    }

    bspTree.initialize(r, n);

    auto bspInsert = [&] (EngravingItem* item) {
        if (item->collectForDrawing()) {
            bspTree.insert(item);
        }
    };
    scanElements(bspInsert);

    m_bspTreeValid = true;
}

//---------------------------------------------------------
//   isOdd
//---------------------------------------------------------

bool Page::isOdd() const
{
    return (m_pageNumber + 1 + score()->pageNumberOffset()) & 1;
}

//---------------------------------------------------------
//   elements
//---------------------------------------------------------

std::vector<EngravingItem*> Page::elements() const
{
    return getChildren(false);
}

//---------------------------------------------------------
//   tm
//---------------------------------------------------------

double Page::tm() const
{
    return ((!style().styleB(Sid::pageTwosided) || isOdd())
            ? style().styleD(Sid::pageOddTopMargin) : style().styleD(Sid::pageEvenTopMargin)) * DPI;
}

//---------------------------------------------------------
//   bm
//---------------------------------------------------------

double Page::bm() const
{
    return ((!style().styleB(Sid::pageTwosided) || isOdd())
            ? style().styleD(Sid::pageOddBottomMargin) : style().styleD(Sid::pageEvenBottomMargin)) * DPI;
}

//---------------------------------------------------------
//   lm
//---------------------------------------------------------

double Page::lm() const
{
    return ((!style().styleB(Sid::pageTwosided) || isOdd())
            ? style().styleD(Sid::pageOddLeftMargin) : style().styleD(Sid::pageEvenLeftMargin)) * DPI;
}

//---------------------------------------------------------
//   rm
//---------------------------------------------------------

double Page::rm() const
{
    return (style().styleD(Sid::pageWidth) - style().styleD(Sid::pagePrintableWidth)) * DPI - lm();
}

//---------------------------------------------------------
//   tbbox
//    calculates and returns smallest rectangle containing all (visible) page elements
//---------------------------------------------------------

RectF Page::tbbox() const
{
    double x1 = width();
    double x2 = 0.0;
    double y1 = height();
    double y2 = 0.0;
    const std::vector<EngravingItem*> el = elements();
    for (EngravingItem* e : el) {
        if (e == this || !e->isPrintable()) {
            continue;
        }
        RectF ebbox = e->pageBoundingRect();
        if (ebbox.left() < x1) {
            x1 = ebbox.left();
        }
        if (ebbox.right() > x2) {
            x2 = ebbox.right();
        }
        if (ebbox.top() < y1) {
            y1 = ebbox.top();
        }
        if (ebbox.bottom() > y2) {
            y2 = ebbox.bottom();
        }
    }
    if (x1 < x2 && y1 < y2) {
        return RectF(x1, y1, x2 - x1, y2 - y1);
    } else {
        return pageBoundingRect();
    }
}

//---------------------------------------------------------
//   endTick
//---------------------------------------------------------

Fraction Page::endTick() const
{
    return m_systems.empty() ? Fraction(-1, 1) : m_systems.back()->measures().back()->endTick();
}

Measure* Page::firstMeasure() const
{
    for (System* s : m_systems) {
        if (Measure* m = s->firstMeasure()) {
            return m;
        }
    }

    return nullptr;
}
