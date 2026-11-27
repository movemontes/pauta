/*
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "pautatourmodel.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QUrl>

#ifdef Q_OS_WIN
#define PAUTA_RESOURCES_DIR "/../" // Windows: bin/Pauta.exe, data next to bin/
#else
#define PAUTA_RESOURCES_DIR "/../Resources/" // Mac: Pauta.app/Contents/Resources
#endif

#include "actions/actiontypes.h"
#include "project/inotationproject.h"

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

void PautaTourModel::showScorePage()
{
    if (globalContext()->currentProject()) {
        interactive()->open("musescore://notation");
        return;
    }

    // copy the blank treble clef template to a temporary folder and open it as "Exemplo"
    const QString src = QCoreApplication::applicationDirPath() + PAUTA_RESOURCES_DIR "templates/01-General/01-Treble_Clef";
    const QString dst = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/PautaTour";
    QDir(dst).removeRecursively();
    QDir().mkpath(dst);
    const QDir srcDir(src);
    for (const QString& name : srcDir.entryList(QDir::Files)) {
        const QString target = name.endsWith(".mscx") ? QString("Exemplo.mscx") : name;
        QFile::copy(srcDir.filePath(name), dst + "/" + target);
    }
    const QString file = dst + "/Exemplo.mscx";
    if (!QFile::exists(file)) {
        return;
    }
    m_openedExample = true;
    dispatcher()->dispatch("file-open", muse::actions::ActionData::make_arg1<QUrl>(QUrl::fromLocalFile(file)));
}

void PautaTourModel::showHomePage()
{
    // only when the tour opened its own example score: close it and go back home
    if (m_openedExample) {
        m_openedExample = false;
        // the example comes from an old-format template, so it counts as changed; save it quietly
        // in the temporary folder first, so closing it does not ask "save changes?"
        if (auto project = globalContext()->currentProject()) {
            const QString dst = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/PautaTour/Exemplo.mscz";
            project->save(muse::io::path_t(dst), mu::project::SaveMode::SaveAs, false);
        }
        dispatcher()->dispatch("file-close");
        interactive()->open("musescore://home");
    }
}

int PautaTourModel::debugStep() const
{
    return qEnvironmentVariableIntValue("PAUTA_TOUR_STEP");
}
