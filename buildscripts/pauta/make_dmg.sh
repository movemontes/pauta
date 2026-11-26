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
DMG="$OUT/Pauta-$VERSION-mac.dmg"

echo "== build"
cd "$BUILD" && ninja -j8 && ninja install >/dev/null

echo "== bundle Qt (macdeployqt)"
macdeployqt "$APP" -qmldir="$SRC/src" -verbose=1

find "$APP/Contents" -type d -name "*.dSYM" -exec rm -rf {} +
# Database drivers Qt adds on its own: Pauta does not use them, and they point to libraries outside the app
rm -f "$APP/Contents/PlugIns/sqldrivers/"libqsql{mimer,odbc,psql}.dylib
if [ -d "$APP/Contents/Resources/qml" ]; then
    rm -rf "$APP/Contents/Resources/qml_mu"
    mv "$APP/Contents/Resources/qml" "$APP/Contents/Resources/qml_mu"
    sed -i '' 's:Resources/qml:Resources/qml_mu:g' "$APP/Contents/Resources/qt.conf"
fi

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
