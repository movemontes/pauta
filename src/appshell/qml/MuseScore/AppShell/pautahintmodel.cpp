/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: Colcheia tells about an on/off option (see pautahintmodel.h).
 */
#include "pautahintmodel.h"

#include <QCursor>
#include <QSettings>
#include <QTimer>

#include "notation/internal/pautapasteextras.h"
#include "translation.h"

using namespace mu::appshell;

static constexpr int TIMES_PER_HINT = 3;

static QString hintText(const std::string& key)
{
    if (key == "rippleOff") {
        return muse::qtrc("appshell", "Changing a duration writes over the next notes. To push them forward instead, go to <b>Edit &gt; A new duration pushes the next notes</b>.");
    }
    if (key == "rippleOn") {
        return muse::qtrc("appshell", "The next notes moved forward with this one. To turn this off, go to <b>Edit &gt; A new duration pushes the next notes</b>.");
    }
    if (key == "paste") {
        return muse::qtrc("appshell", "The time signature, key, repeats and tempo came along with the notes. To choose what comes along, go to <b>Edit &gt; Paste brings along</b>.");
    }
    if (key == "backspace") {
        return muse::qtrc("appshell", "While you write, Backspace goes back and deletes, like in a text. To turn it off, go to <b>Edit &gt; Backspace deletes backwards while writing</b>.");
    }
    if (key == "repeatSize") {
        return muse::qtrc("appshell", "Repeat signs too small to read? To make them bigger, or give them wings, go to <b>Format &gt; Repeat signs</b>.");
    }
    if (key == "playOnRepeat") {
        return muse::qtrc("appshell", "Should some notes play only the 1st or only the 2nd time? Select them and go to <b>Tools &gt; Play on repeat</b>.");
    }
    if (key == "capture") {
        return muse::qtrc("appshell", "To send this passage as a picture (e-mail, WhatsApp), go to <b>Edit &gt; Copy selection as picture</b>.");
    }
    if (key == "copyLayout") {
        return muse::qtrc("appshell", "Happy with how this part looks? To give the other parts the same layout, go to <b>Format &gt; Copy this part's layout to the other parts</b>.");
    }
    if (key == "voices") {
        return muse::qtrc("appshell", "Want to hear only some voices (just the soprano, for example)? Go to <b>View &gt; Voices that play</b>.");
    }
    if (key == "sound") {
        return muse::qtrc("appshell", "Want a fuller sound for the instruments? Go to <b>View &gt; Instrument sound &gt; GeneralUser GS</b>.");
    }
    return QString();
}

PautaHintModel::PautaHintModel(QObject* parent)
    : QObject(parent)
{
    mu::notation::pautaHintRequested().onReceive(this, [this](const std::string& key) {
        show(key);
    });

    // test hook: PAUTA_DEBUG_HINT=<key> shows that hint once the window is up
    const QString debugKey = qEnvironmentVariable("PAUTA_DEBUG_HINT");
    if (!debugKey.isEmpty()) {
        QTimer::singleShot(2500, this, [this, debugKey]() { show(debugKey.toStdString()); });
    }
}

void PautaHintModel::show(const std::string& key)
{
    if (m_active) {
        return; // one at a time
    }
    QSettings s;
    if (!s.value("pauta/mascotTips", true).toBool()) {
        return; // Help > Mascot in tips is off
    }
    const QString text = hintText(key);
    const QString counter = QStringLiteral("pauta/hints/") + QString::fromStdString(key);
    const int times = s.value(counter, 0).toInt();
    if (text.isEmpty() || times >= TIMES_PER_HINT) {
        return;
    }
    s.setValue(counter, times + 1);
    m_text = text;
    m_active = true;
    emit changed();
}

void PautaHintModel::done()
{
    m_active = false;
    emit changed();
}

QPointF PautaHintModel::cursorIn(QQuickItem* item) const
{
    return item ? item->mapFromGlobal(QCursor::pos()) : QPointF();
}
