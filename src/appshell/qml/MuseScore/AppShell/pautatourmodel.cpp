/*
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "pautatourmodel.h"

#include <QSettings>

using namespace mu::appshell;

PautaTourModel::PautaTourModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void PautaTourModel::classBegin()
{
    dispatcher()->reg(this, "pauta-tour", [this]() {
        emit startRequested();
    });
}

bool PautaTourModel::seen() const
{
    return QSettings().value("pauta/tourSeen", false).toBool();
}

void PautaTourModel::markSeen()
{
    QSettings().setValue("pauta/tourSeen", true);
}

void PautaTourModel::run(const QString& action)
{
    dispatcher()->dispatch(action.toStdString());
}
