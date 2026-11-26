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
#include "applicationactioncontroller.h"

#include <QSettings>

#include <QApplication>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QFileOpenEvent>
#include <QWindow>
#include <QDir>
#include <map>
#include "settings.h"
#include <QProcess>
#include <QFileInfo>
#include <QDirIterator>
#include <QDateTime>
#include <QCoreApplication>
#include "muse_framework_config.h"
#include <QMimeData>
#include <QTimer>

#include "async/async.h"
#include "audio/common/soundfonttypes.h"

#include "defer.h"
#include "translation.h"
#include "log.h"

using namespace mu::appshell;
using namespace muse;
using namespace muse::actions;

void ApplicationActionController::preInit()
{
    qApp->installEventFilter(this);
}

void ApplicationActionController::init()
{
    dispatcher()->reg(this, "quit", [this](const ActionData& args) {
        bool isAllInstances = args.count() > 0 ? args.arg<bool>(0) : true;
        muse::io::path_t installatorPath = args.count() > 1 ? args.arg<muse::io::path_t>(1) : "";

        QTimer::singleShot(0, [this, isAllInstances, installatorPath]() {
            quit(isAllInstances, installatorPath);
        });
    });

    dispatcher()->reg(this, "restart", [this]() {
        restart();
    });

    dispatcher()->reg(this, "fullscreen", this, &ApplicationActionController::toggleFullScreen);

    dispatcher()->reg(this, "about-musescore", this, &ApplicationActionController::openAboutDialog);
    dispatcher()->reg(this, "about-qt", this, &ApplicationActionController::openAboutQtDialog);
    dispatcher()->reg(this, "about-musicxml", this, &ApplicationActionController::openAboutMusicXMLDialog);
    dispatcher()->reg(this, "online-handbook", this, &ApplicationActionController::openOnlineHandbookPage);
    // Pauta
    for (const char* preset : { "blue", "orange", "green", "pink", "purple", "light", "graphite", "night" }) {
        const std::string name(preset);
        dispatcher()->reg(this, "pauta-theme-" + name, [this, name]() { pautaApplyColors(name); });
    }
    dispatcher()->reg(this, "pauta-toggle-mascot", [this]() {
        QSettings s;
        s.setValue("pauta/mascotTips", !s.value("pauta/mascotTips", true).toBool());
        m_pautaMascotChanged.notify();
    });
    dispatcher()->reg(this, "pauta-theme-custom", this, &ApplicationActionController::pautaChooseAccent);
    // Pauta: first start with the Treco look resets colors saved by older versions (pixel look)
    {
        QSettings s;
        if (s.value("pauta/lookVersion", 0).toInt() < 2) {
            s.setValue("pauta/lookVersion", 2);
            pautaApplyColors("light");
        }
    }
    dispatcher()->reg(this, "pauta-quick-guide", [this]() { interactive()->open("musescore://pauta/guide"); });
    dispatcher()->reg(this, "pauta-import-pdf", this, &ApplicationActionController::pautaImportPdf);
    dispatcher()->reg(this, "pauta-export-to-program", [this]() { interactive()->open("musescore://pauta/exportto"); });
    dispatcher()->reg(this, "pauta-command-palette", [this]() { interactive()->open("musescore://pauta/commandpalette"); });
    dispatcher()->reg(this, "pauta-support", [this]() { interactive()->open("musescore://welcomedialog"); });
    dispatcher()->reg(this, "pauta-language-system", [this]() { pautaSetLanguage("system"); });
    dispatcher()->reg(this, "pauta-language-pt", [this]() { pautaSetLanguage("pt_BR"); });
    dispatcher()->reg(this, "pauta-language-en", [this]() { pautaSetLanguage("en"); });
    dispatcher()->reg(this, "ask-help", this, &ApplicationActionController::openAskForHelpPage);
    dispatcher()->reg(this, "accessibility-statement", this, &ApplicationActionController::openAccessibilityStatementPage);
    dispatcher()->reg(this, "preference-dialog", this, &ApplicationActionController::openPreferencesDialog);

    dispatcher()->reg(this, "revert-factory", this, &ApplicationActionController::revertToFactorySettings);

    dispatcher()->reg(this, "manage-plugins", [this]() {
        interactive()->open("musescore://home?section=plugins");
    });

    // Global actions
    dispatcher()->reg(this, "action://copy", this, &ApplicationActionController::doGlobalCopy);
    dispatcher()->reg(this, "action://cut", this, &ApplicationActionController::doGlobalCut);
    dispatcher()->reg(this, "action://paste", this, &ApplicationActionController::doGlobalPaste);
    dispatcher()->reg(this, "action://undo", this, &ApplicationActionController::doGlobalUndo);
    dispatcher()->reg(this, "action://redo", this, &ApplicationActionController::doGlobalRedo);
    dispatcher()->reg(this, "action://delete", this, &ApplicationActionController::doGlobalDelete);
    dispatcher()->reg(this, "action://cancel", this, &ApplicationActionController::doGlobalCancel);
}

bool ApplicationActionController::eventFilter(QObject* watched, QEvent* event)
{
    if ((event->type() == QEvent::Close && watched == qWindow())
        || event->type() == QEvent::Quit) {
        bool accepted = quit(false);
        event->setAccepted(accepted);

        return true;
    }

    if (watched == qApp) {
        if (event->type() == QEvent::FileOpen) {
            const QFileOpenEvent* openEvent = static_cast<const QFileOpenEvent*>(event);
            const QUrl url = openEvent->url();

            if (projectFilesController()->isUrlSupported(url)) {
                if (startupScenario()->startupCompleted()) {
                    dispatcher()->dispatch("file-open", ActionData::make_arg1<QUrl>(url));
                } else {
                    startupScenario()->setStartupScoreFile(project::ProjectFile { url });
                }

                return true;
            }
        }
    }

    if (watched == qWindow()) {
        if (event->type() == QEvent::DragEnter) {
            if (onDragEnterEvent(static_cast<QDragEnterEvent*>(event))) {
                return true;
            }
        } else if (event->type() == QEvent::DragMove) {
            if (onDragMoveEvent(static_cast<QDragMoveEvent*>(event))) {
                return true;
            }
        } else if (event->type() == QEvent::Drop) {
            if (onDropEvent(static_cast<QDropEvent*>(event))) {
                return true;
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

QWindow* ApplicationActionController::qWindow() const
{
    return mainWindow() ? mainWindow()->qWindow() : nullptr;
}

ApplicationActionController::DragTarget ApplicationActionController::dragTarget(const QUrl& url) const
{
    if (projectFilesController()->isUrlSupported(url)) {
        return DragTarget::ProjectFile;
    } else if (url.isLocalFile()) {
        muse::io::path_t filePath = url.toLocalFile();
        if (muse::audio::synth::isSoundFont(filePath)) {
            return DragTarget::SoundFont;
        } else if (extensionInstaller()->isFileSupported(filePath)) {
            return DragTarget::Extension;
        } else {
            bool scorePageWithProjectOpen = interactive()->currentUri().val == NOTATION_URI
                                            && globalContext()->currentProject() != nullptr;
            if (!scorePageWithProjectOpen && convertFileToScoreScenario()->isFileSupported(filePath)) {
                return DragTarget::ConvertibleFile;
            }
        }
    }
    return DragTarget::Unknown;
}

bool ApplicationActionController::onDragEnterEvent(QDragEnterEvent* event)
{
    return onDragMoveEvent(event);
}

bool ApplicationActionController::onDragMoveEvent(QDragMoveEvent* event)
{
    const QMimeData* mime = event->mimeData();
    QList<QUrl> urls = mime->urls();
    if (urls.count() > 0) {
        const QUrl& url = urls.front();
        DragTarget target = dragTarget(url);
        if (target != DragTarget::Unknown) {
            event->setDropAction(Qt::LinkAction);
            event->acceptProposedAction();
            return true;
        }
    }

    return false;
}

bool ApplicationActionController::onDropEvent(QDropEvent* event)
{
    const QMimeData* mime = event->mimeData();
    QList<QUrl> urls = mime->urls();
    if (urls.count() > 0) {
        const QUrl& url = urls.front();

        bool shouldBeHandled = true;
        DragTarget target = dragTarget(url);
        switch (target) {
        case DragTarget::ProjectFile: {
            async::Async::call(this, [this, url]() {
                    Ret ret = projectFilesController()->openProject(url);
                    if (!ret) {
                        LOGE() << ret.toString();
                    }
                });
        } break;
        case DragTarget::SoundFont: {
            muse::io::path_t filePath = url.toLocalFile();
            async::Async::call(this, [this, filePath]() {
                    soundFontController()->addSoundFont(Uri::fromLocalFile(filePath));
                });
        } break;
        case DragTarget::Extension: {
            muse::io::path_t filePath = url.toLocalFile();
            async::Async::call(this, [this, filePath]() {
                    extensionInstaller()->installExtension(filePath);
                });
        } break;
        case DragTarget::ConvertibleFile: {
            muse::io::paths_t paths;
            for (const QUrl& u : urls) {
                if (u.isLocalFile()) {
                    paths.push_back(muse::io::path_t(u.toLocalFile()));
                }
            }
            async::Async::call(this, [this, paths]() {
                    convertFileToScoreScenario()->convertFiles(paths);
                });
        } break;
        case DragTarget::Unknown:
            shouldBeHandled = false;
            break;
        }

        if (shouldBeHandled) {
            event->accept();
        } else {
            event->ignore();
        }

        return shouldBeHandled;
    }

    return false;
}

bool ApplicationActionController::quit(bool isAllInstances, const muse::io::path_t& installerPath)
{
    if (m_quiting) {
        return false;
    }

    m_quiting = true;
    DEFER {
        m_quiting = false;
    };

    // Pauta: close every tab, asking "save?" for each score with changes
    if (!pautaCloseAllTabs()) {
        return false;
    }

    if (isAllInstances) {
        multiwindowsProvider()->quitForAll();
    }

    if (multiwindowsProvider()->windowCount() == 1 && !installerPath.empty()) {
#if defined(Q_OS_MACOS)
        // Pauta (owner standard, same as Treco): when this app has closed, open the downloaded .dmg,
        // send the old Pauta to the Trash, put the new one in its place, clear quarantine and reopen.
        const QString bundle = QDir(QCoreApplication::applicationDirPath() + "/../..").canonicalPath();
        const QString script = QStringLiteral(
            "DMG=\"$1\"; OLD=\"$2\"; PID=\"$3\"; VER=\"$4\"\n"
            "while kill -0 \"$PID\" 2>/dev/null; do sleep 0.3; done\n"
            "M=$(mktemp -d /tmp/pauta_mount.XXXXXX)\n"
            "hdiutil attach -nobrowse -readonly -mountpoint \"$M\" \"$DMG\" >/dev/null || { open \"$DMG\"; exit 1; }\n"
            "NEW=\"$(dirname \"$OLD\")/.Pauta-novo.app\"\n"
            "[ -e \"$NEW\" ] && mv \"$NEW\" \"$HOME/.Trash/Pauta-novo-$(date +%s).app\"\n"
            "ditto \"$M/Pauta.app\" \"$NEW\"; R=$?\n"
            "hdiutil detach -force \"$M\" >/dev/null\n"
            "[ $R -eq 0 ] || { open \"$DMG\"; exit 1; }\n"
            "mv \"$OLD\" \"$HOME/.Trash/Pauta $VER ($(date +%H.%M.%S)).app\" && mv \"$NEW\" \"$OLD\" && xattr -dr com.apple.quarantine \"$OLD\"\n"
            "open \"$OLD\"\n");
        QProcess::startDetached("/bin/sh", { "-c", script, "pauta-update", installerPath.toQString(), bundle,
                                             QString::number(QCoreApplication::applicationPid()), QString(PAUTA_VERSION) });
#elif defined(Q_OS_LINUX)
        interactive()->revealInFileBrowser(installerPath);
#else
        interactive()->openUrl(QUrl::fromLocalFile(installerPath.toQString()));
#endif
    }

    if (multiwindowsProvider()->windowCount() > 1) {
        multiwindowsProvider()->notifyAboutWindowWasQuited();
    }

    QCoreApplication::exit();
    return true;
}

void ApplicationActionController::restart()
{
    if (pautaCloseAllTabs()) {
        if (multiwindowsProvider()->windowCount() == 1) {
            application()->restart();
        } else {
            multiwindowsProvider()->quitAllAndRestartLast();

            QCoreApplication::exit();
        }
    }
}

void ApplicationActionController::toggleFullScreen()
{
    mainWindow()->toggleFullScreen();
}

void ApplicationActionController::openAboutDialog()
{
    interactive()->open("musescore://about/musescore");
}

void ApplicationActionController::openAboutQtDialog()
{
    QApplication::aboutQt();
}

void ApplicationActionController::openAboutMusicXMLDialog()
{
    interactive()->open("musescore://about/musicxml");
}

void ApplicationActionController::openOnlineHandbookPage()
{
    std::string handbookUrl = configuration()->handbookUrl();
    interactive()->openUrl(handbookUrl);
}

void ApplicationActionController::openAskForHelpPage()
{
    std::string askForHelpUrl = configuration()->askForHelpUrl();
    interactive()->openUrl(askForHelpUrl);
}

void ApplicationActionController::openAccessibilityStatementPage()
{
    std::string accessibilityStatementUrl = configuration()->accessibilityStatementUrl();
    interactive()->openUrl(accessibilityStatementUrl);
}

void ApplicationActionController::openPreferencesDialog()
{
    const context::IPlaybackStatePtr state = globalContext()->playbackState();
    if (state->playbackStatus() == audio::PlaybackStatus::Running) {
        dispatcher()->dispatch("stop");

        async::Channel<audio::PlaybackStatus> statusChanged = state->playbackStatusChanged();
        statusChanged.onReceive(this, [statusChanged, this](audio::PlaybackStatus) {
            auto statusChangedMut = statusChanged;
            statusChangedMut.disconnect(this);
            doOpenPreferencesDialog();
        });

        return;
    }

    doOpenPreferencesDialog();
}

void ApplicationActionController::doOpenPreferencesDialog()
{
    if (multiwindowsProvider()->isPreferencesAlreadyOpened()) {
        multiwindowsProvider()->activateWindowWithOpenedPreferences();
        return;
    }

    interactive()->open("muse://preferences");
}

void ApplicationActionController::revertToFactorySettings()
{
    std::string title = muse::trc("appshell", "Are you sure you want to revert to factory settings?");
    std::string question = muse::trc("appshell", "This action will reset all your app preferences and delete all custom palettes and custom shortcuts. "
                                                 "The list of recent scores will also be cleared.\n\n"
                                                 "This action will not delete any of your scores.");

    IInteractive::ButtonData cancelBtn = interactive()->buttonData(IInteractive::Button::Cancel);
    cancelBtn.accent = true;

    int revertBtn = int(IInteractive::Button::Apply);
    auto promise = interactive()->warning(title, question,
                                          { cancelBtn,
                                            IInteractive::ButtonData(revertBtn, muse::trc("appshell", "Revert")) },
                                          cancelBtn.btn, { muse::IInteractive::Option::WithIcon },
                                          muse::trc("appshell", "Revert to factory settings"));

    promise.onResolve(this, [this](const IInteractive::Result& res) {
        if (res.isButton(IInteractive::Button::Cancel)) {
            return;
        }

        static constexpr bool KEEP_DEFAULT_SETTINGS = false;
        static constexpr bool NOTIFY_ABOUT_CHANGES = false;
        static constexpr bool NOTIFY_OTHER_INSTANCES = false;
        configuration()->revertToFactorySettings(KEEP_DEFAULT_SETTINGS, NOTIFY_ABOUT_CHANGES, NOTIFY_OTHER_INSTANCES);

        std::string title = muse::trc("appshell", "Would you like to restart MuseScore Studio now?");
        std::string question = muse::trc("appshell", "MuseScore Studio needs to be restarted for these changes to take effect.");

        int restartBtn = int(IInteractive::Button::Apply);
        auto promise = interactive()->question(title, question,
                                               { interactive()->buttonData(IInteractive::Button::Cancel),
                                                 IInteractive::ButtonData(restartBtn, muse::trc("appshell", "Restart"), true) },
                                               restartBtn);

        promise.onResolve(this, [this](const IInteractive::Result& res) {
            if (!res.isButton(IInteractive::Button::Cancel)) {
                restart();
            }
        });
    });
}

// Pauta: PDF or picture of printed sheet music -> editable score, on this Mac (no cloud).
// Uses Audiveris (free, AGPL, separate app the person installs once): https://github.com/Audiveris/audiveris
static QString findAudiveris()
{
    for (const QString& app : { QStringLiteral("/Applications/Audiveris.app"), QDir::homePath() + "/Applications/Audiveris.app" }) {
        const QString exe = app + "/Contents/MacOS/Audiveris";
        if (QFileInfo::exists(exe)) {
            return exe;
        }
    }
    return QString();
}

void ApplicationActionController::pautaImportPdf()
{
    const QString audiveris = findAudiveris();
    if (audiveris.isEmpty()) {
        int getBtn = int(IInteractive::Button::Apply);
        interactive()->info(muse::trc("appshell", "To read sheet music from a PDF, Pauta uses Audiveris"),
                            muse::trc("appshell", "Audiveris is a free program that turns printed sheet music (PDF or photo) into an editable score, "
                                                  "right on your Mac. Download the version for Apple chips (macosx-arm64.dmg), drag it to "
                                                  "Applications, open it once, and then come back here."),
                            { interactive()->buttonData(IInteractive::Button::Cancel),
                              IInteractive::ButtonData(getBtn, muse::trc("appshell", "Download Audiveris"), true) },
                            getBtn)
        .onResolve(this, [this, getBtn](const IInteractive::Result& res) {
            if (res.isButton(getBtn)) {
                interactive()->openUrl(std::string("https://github.com/Audiveris/audiveris/releases/latest"));
            }
        });
        return;
    }

    const muse::io::path_t file = interactive()->selectOpeningFileSync(
        muse::trc("appshell", "Choose the PDF or picture of the sheet music"), "",
        { muse::trc("appshell", "PDF and pictures") + " (*.pdf *.png *.jpg *.jpeg *.tif *.tiff)" });
    if (file.empty()) {
        return;
    }

    const QString outDir = QDir::tempPath() + "/pauta-pdf-" + QString::number(QDateTime::currentMSecsSinceEpoch());
    QDir().mkpath(outDir);

    auto* proc = new QProcess(this);
    proc->setProgram(audiveris);
    proc->setArguments({ "-batch", "-export", "-output", outDir, "--", file.toQString() });

    m_pautaPdfProgress = muse::Progress();
    m_pautaPdfProgress.start();
    interactive()->showProgress(muse::trc("appshell", "Reading the sheet music… (this can take a minute)"), m_pautaPdfProgress);

    QObject::connect(proc, &QProcess::finished, this, [this, proc, outDir](int, QProcess::ExitStatus) {
        proc->deleteLater();
        m_pautaPdfProgress.finish(muse::make_ok());

        QStringList found;
        QDirIterator it(outDir, { "*.mxl", "*.musicxml", "*.xml" }, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            found << it.next();
        }
        found.sort();
        if (found.isEmpty()) {
            interactive()->warning(muse::trc("appshell", "Couldn't read this sheet music"),
                                   muse::trc("appshell", "It works best with clean printed scores (not handwritten or blurry photos)."));
            return;
        }
        for (const QString& f : found) {
            projectFilesController()->openProject(project::ProjectFile(muse::io::path_t(f)));
        }
    });
    proc->start();
}

// Pauta: ready-made interface color palettes (pixel look). The score page stays white.
// Pauta: Treco look. The base (light cream, soft dark gray, night) and the accent color are chosen apart.
// On a dark base the accent is lightened so it keeps enough contrast without glare.
static bool pautaIsDarkBase(const QColor& primary)
{
    return primary.lightnessF() < 0.5;
}

static QColor pautaAccentFor(const QColor& accent, bool darkBase)
{
    if (!darkBase || accent.lightnessF() >= 0.6) {
        return accent;
    }
    return QColor::fromRgbF(accent.redF() + (1 - accent.redF()) * 0.45, accent.greenF() + (1 - accent.greenF()) * 0.45,
                            accent.blueF() + (1 - accent.blueF()) * 0.45);
}

void ApplicationActionController::pautaSetAccent(const QColor& chosen)
{
    using namespace muse::ui;
    QSettings().setValue("pauta/accent", chosen.name());
    const QColor primary = uiConfiguration()->currentTheme().values.value(BACKGROUND_PRIMARY_COLOR).value<QColor>();
    const QColor accent = pautaAccentFor(chosen, pautaIsDarkBase(primary));
    uiConfiguration()->setCurrentThemeStyleValue(ACCENT_COLOR, muse::Val(accent));
    uiConfiguration()->setCurrentThemeStyleValue(FOCUS_COLOR, muse::Val(accent));
    uiConfiguration()->setCurrentThemeStyleValue(LINK_COLOR, muse::Val(accent));
}

void ApplicationActionController::pautaApplyColors(const std::string& name)
{
    static const std::map<std::string, const char*> accents {
        { "blue", "#0000FF" }, { "orange", "#FF4A00" }, { "green", "#00914A" }, { "pink", "#F0187F" }, { "purple", "#7426F2" },
    };
    auto acc = accents.find(name);
    if (acc != accents.end()) {
        pautaSetAccent(QColor(acc->second));
        return;
    }

    struct Base {
        const char* primary; const char* secondary; const char* button; const char* field;
        const char* font; const char* fontOnAccent; const char* stroke; const char* canvas; const char* paper;
    };
    // paper: the score page on screen only (printing and PDF stay white); dimmer on dark bases to avoid glare
    static const std::map<std::string, Base> bases {
        // light: cream paper, near-black ink
        { "light",    { "#FDF9EE", "#F4F0E3", "#FFFFFF", "#FFFFFF", "#0A0A0A", "#FFFFFF", "#0A0A0A", "#E4DECD", "#F9F9F9" } },
        // soft dark: mid-dark grays, warm off-white text, dark outlines (no pure black, no pure white)
        { "graphite", { "#303136", "#28292D", "#3C3D43", "#37383E", "#DDD8CC", "#F2EEE6", "#16171A", "#45464D", "#E6E2D8" } },
        // night: near black, cream outlines
        { "night",    { "#161616", "#0E0E0E", "#222222", "#1C1C1C", "#F4F0E3", "#FFFFFF", "#F4F0E3", "#2A2A2A", "#DCD8CE" } },
    };
    auto it = bases.find(name);
    if (it == bases.end()) {
        return;
    }
    const Base& p = it->second;
    using namespace muse::ui;
    auto set = [this](ThemeStyleKey key, const char* hex) { uiConfiguration()->setCurrentThemeStyleValue(key, muse::Val(QColor(hex))); };
    set(BACKGROUND_PRIMARY_COLOR, p.primary);
    set(POPUP_BACKGROUND_COLOR, p.primary);
    set(BACKGROUND_SECONDARY_COLOR, p.secondary);
    set(BUTTON_COLOR, p.button);
    set(TEXT_FIELD_COLOR, p.field);
    set(FONT_PRIMARY_COLOR, p.font);
    set(FONT_SECONDARY_COLOR, p.fontOnAccent);
    set(STROKE_COLOR, p.stroke);
    QSettings().setValue("pauta/base", QString::fromStdString(name));

    // keep the chosen accent, adjusted for the new base
    pautaSetAccent(QColor(QSettings().value("pauta/accent", "#0000FF").toString()));

    // the area around the page
    muse::settings()->setSharedValue(muse::Settings::Key("notation", "ui/canvas/background/lightTheme_score_background_color"),
                                     muse::Val(QColor(p.canvas)));
    muse::settings()->setSharedValue(muse::Settings::Key("notation", "ui/canvas/foreground/color"), muse::Val(QColor(p.paper)));
}

void ApplicationActionController::pautaChooseAccent()
{
    // Pauta: any accent color, keeping the base colors
    const QColor current(QSettings().value("pauta/accent", "#0000FF").toString());
    interactive()->selectColor(muse::Color::fromQColor(current), muse::trc("appshell", "Choose the accent color"))
    .onResolve(this, [this](const muse::Color& c) {
        pautaSetAccent(c.toQColor());
    });
}

void ApplicationActionController::pautaSetLanguage(const QString& code)
{
    if (languagesConfiguration()->currentLanguageCode().val == code) {
        return;
    }
    languagesConfiguration()->setCurrentLanguageCode(code);

    std::string title = muse::trc("appshell", "Language");
    std::string question = muse::trc("appshell", "Pauta needs to restart to change the language.");
    int restartBtn = int(IInteractive::Button::Apply);
    auto promise = interactive()->question(title, question,
                                           { IInteractive::ButtonData(int(IInteractive::Button::Cancel), muse::trc("appshell", "Later")),
                                             IInteractive::ButtonData(restartBtn, muse::trc("appshell", "Restart now"), true) },
                                           restartBtn);
    promise.onResolve(this, [this](const IInteractive::Result& res) {
        if (!res.isButton(IInteractive::Button::Cancel)) {
            restart();
        }
    });
}

bool ApplicationActionController::pautaCloseAllTabs()
{
    // closeOpenedProject() closes the current tab and shows the next one, so repeat until none is left
    size_t guard = globalContext()->openedProjects().size() + 2;
    while (globalContext()->currentProject() && guard-- > 0) {
        if (!projectFilesController()->closeOpenedProject(false)) {
            return false; // the person pressed Cancel
        }
    }
    return true;
}

bool ApplicationActionController::hasProjectAndIsFocused() const
{
    bool hasProject = globalContext()->currentProject() != nullptr;
    bool isFocused = uiContextResolver()->currentUiContext() == context::UiCtxProjectFocused;
    return hasProject && isFocused;
}

void ApplicationActionController::doGlobalCopy()
{
    if (hasProjectAndIsFocused()) {
        dispatcher()->dispatch("action://notation/copy");
    } else {
        // resolve other actions
    }
}

void ApplicationActionController::doGlobalCut()
{
    if (hasProjectAndIsFocused()) {
        dispatcher()->dispatch("action://notation/cut");
    } else {
        // resolve other actions
    }
}

void ApplicationActionController::doGlobalPaste()
{
    if (hasProjectAndIsFocused()) {
        dispatcher()->dispatch("action://notation/paste");
    } else {
        // resolve other actions
    }
}

void ApplicationActionController::doGlobalUndo()
{
    if (hasProjectAndIsFocused()) {
        dispatcher()->dispatch("action://notation/undo");
    } else {
        // resolve other actions
    }
}

void ApplicationActionController::doGlobalRedo()
{
    if (hasProjectAndIsFocused()) {
        dispatcher()->dispatch("action://notation/redo");
    } else {
        // resolve other actions
    }
}

void ApplicationActionController::doGlobalDelete()
{
    if (hasProjectAndIsFocused()) {
        dispatcher()->dispatch("action://notation/delete");
    } else {
        // resolve other actions
    }
}

void ApplicationActionController::doGlobalCancel()
{
    if (hasProjectAndIsFocused()) {
        dispatcher()->dispatch("action://notation/cancel");
    } else {
        dispatcher()->dispatch("nav-escape");
    }
}
