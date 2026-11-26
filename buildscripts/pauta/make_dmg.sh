#!/usr/bin/env bash
# Pauta: builds, bundles Qt into Pauta.app and makes Pauta-<version>-mac.dmg.
# Everything stays on the Dados disk. Usage: bash buildscripts/pauta/make_dmg.sh
set -euo pipefail

ROOT=/Volumes/Dados/PautaCodigo
SRC=$ROOT/src
BUILD=$ROOT/build
INSTALL=$ROOT/install
QT=$ROOT/Qt/6.10.2/macos
OUT="/Volumes/Dados/Softwares e Apps (Dev)/Pauta/dist"
export PATH="$ROOT/venv/bin:$QT/bin:$PATH"

VERSION=$(grep 'set(PAUTA_VERSION' "$SRC/version.cmake" | sed -E 's/.*"([^"]+)".*/\1/')
APP=$INSTALL/Pauta.app
DMG="$OUT/Pauta-mac.dmg" # owner standard: fixed name for the updater and the download link

echo "== build"
if pgrep -f "$APP/Contents/MacOS/Pauta" >/dev/null; then
    echo "O Pauta desta pasta está aberto. Feche-o antes de gerar de novo (trocar os arquivos com ele aberto faz ele fechar sozinho)."
    exit 2
fi
rm -rf "$APP"   # always start from a clean bundle (old runs left a broken qt.conf)
cd "$BUILD" && ninja -j3 && ninja install >/dev/null   # -j3: more parallel jobs once froze the Mac

echo "== bundle Qt (macdeployqt)"
macdeployqt "$APP" -qmldir="$SRC/src" -verbose=1

find "$APP/Contents" -type d -name "*.dSYM" -exec rm -rf {} +
# Database drivers Qt adds on its own: Pauta does not use them, and they point to libraries outside the app
rm -f "$APP/Contents/PlugIns/sqldrivers/"libqsql{mimer,odbc,psql}.dylib
# keep Qt's QML folder as macdeployqt made it (Resources/qml, listed in qt.conf)
grep -q "Resources/qml$" "$APP/Contents/Resources/qt.conf" || { echo "qt.conf inesperado"; cat "$APP/Contents/Resources/qt.conf"; exit 1; }

# Denigma (MIT): converts Finale files so Pauta can open them. Built from source, see COMPILAR.md
DENIGMA=$ROOT/downloads/denigma-src/build/build/Release/denigma
if [ -x "$DENIGMA" ]; then
    cp "$DENIGMA" "$APP/Contents/MacOS/denigma"
    mkdir -p "$APP/Contents/Resources/licenses"
    cp "$ROOT/downloads/denigma-src/LICENSE" "$APP/Contents/Resources/licenses/denigma-LICENSE.txt"
else
    echo "AVISO: denigma não encontrado; o Pauta não vai abrir arquivos do Finale"
fi

# Clash Grotesk (ITF Free Font License): may ship inside the app, but not in a public repository.
# The files live outside Git, in $ROOT/fontes-privadas. Without them Pauta uses Archivo (same look, a bit plainer).
CLASH=$ROOT/fontes-privadas
if [ -f "$CLASH/ClashGrotesk-Medium.otf" ] && [ -f "$CLASH/ClashGrotesk-Semibold.otf" ]; then
    mkdir -p "$APP/Contents/Resources/fonts" "$APP/Contents/Resources/licenses"
    cp "$CLASH/ClashGrotesk-Medium.otf" "$CLASH/ClashGrotesk-Semibold.otf" "$APP/Contents/Resources/fonts/"
    [ -f "$CLASH/LICENSE.txt" ] && cp "$CLASH/LICENSE.txt" "$APP/Contents/Resources/licenses/ClashGrotesk-LICENSE.txt"
else
    echo "AVISO: Clash Grotesk não encontrada em $CLASH; o Pauta vai usar a Archivo no texto"
fi

# Qt 6.10 needs macOS 13; say so in the bundle so older Macs get a clear message
plutil -replace LSMinimumSystemVersion -string 13.0 "$APP/Contents/Info.plist"

echo "== ad-hoc signature (no paid Apple account)"
codesign --force --deep --sign - "$APP"
codesign --verify --deep --strict "$APP"

echo "== dmg"
STAGE=$(mktemp -d "$ROOT/dmg-stage.XXXX")
cp -R "$APP" "$STAGE/Pauta.app"
ln -s /Applications "$STAGE/Applications"
rm -f "$DMG"
hdiutil create -volname "Pauta" -srcfolder "$STAGE" -fs HFS+ -format UDZO -ov "$DMG" >/dev/null
rm -rf "$STAGE"
shasum -a 256 "$DMG"
ls -lh "$DMG"
