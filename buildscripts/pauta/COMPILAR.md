# Compilar o Pauta no Mac

Tudo fica em `/Volumes/Dados/PautaCodigo` (o caminho não pode ter espaços nem parênteses, por causa do sistema de compilação).

1. Ferramentas (uma vez):
   ```bash
   python3 -m venv /Volumes/Dados/PautaCodigo/venv
   /Volumes/Dados/PautaCodigo/venv/bin/pip install cmake ninja aqtinstall fonttools
   cd /Volumes/Dados/PautaCodigo && venv/bin/aqt install-qt mac desktop 6.10.2 clang_64 -O Qt -m qt5compat qtshadertools qtnetworkauth qtwebsockets qtimageformats
   ```
   Precisa também das Command Line Tools da Apple (`xcode-select --install`). O Xcode completo não é necessário.
2. Configurar (uma vez):
   ```bash
   cd /Volumes/Dados/PautaCodigo && mkdir -p build && cd build
   ../venv/bin/cmake ../src -GNinja -DCMAKE_BUILD_TYPE=Release \
     -DCMAKE_PREFIX_PATH=/Volumes/Dados/PautaCodigo/Qt/6.10.2/macos \
     -DCMAKE_INSTALL_PREFIX=/Volumes/Dados/PautaCodigo/install \
     -DMUSESCORE_BUILD_CONFIGURATION=app -DMUSE_APP_BUILD_MODE=release \
     -DMUSE_MODULE_VST=ON -DMUSE_MODULE_CLOUD=OFF -DMUSE_MODULE_MUSESAMPLER=OFF \
     -DMUSE_MODULE_LEARN=OFF -DMUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT=OFF \
     -DMUE_BUILD_MACOS_INTEGRATION=OFF
   ```
3. Gerar o app e o .dmg: `bash buildscripts/pauta/make_dmg.sh` (sai em `Pauta/dist/Pauta-mac.dmg`).

Ícone: `python3 buildscripts/pauta/make_icon.py`. Fontes: `venv/bin/python buildscripts/pauta/make_fonts.py <Archivo-Variable.ttf>`.
Versão do Pauta: `PAUTA_VERSION` em `version.cmake` (a `MUSE_APP_VERSION` fica na versão do MuseScore de origem, porque vai dentro dos arquivos de partitura).
