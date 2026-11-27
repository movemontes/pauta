# Pauta para Windows: guia para a sessão que vai fazer

Escrito em 27/11/2026, quando a versão do Mac era a 1.0.9 (commit 0ea5fb52).

## O que é o Pauta
- Programa de partitura grátis, derivado do MuseScore Studio 4.7.7 (GPL-3.0).
- Código: `/Volumes/Dados/PautaCodigo/src`, branch `pauta`.
- Repositório público: github.com/movemontes/pauta. Para enviar: `git push github pauta:main`.
- Site: pasta `docs/`, publicado em https://movemontes.github.io/pauta/.
- Versão do Pauta: `PAUTA_VERSION` em `version.cmake`. Não mexa na `MUSE_APP_VERSION`, que vai dentro dos arquivos de partitura.
- Como compilar no Mac: `buildscripts/pauta/COMPILAR.md` e `buildscripts/pauta/make_dmg.sh`.

## Regras do dono (valem também aqui)
- O dono é o Mateus (movemontes). Responda em português simples e use pronomes neutros. O dono não é programador e decide vendo: mande prints e mostre as opções cedo.
- Diga com clareza o que foi testado e o que não foi.
- Commit, push, publicar versão e mudar o site: só com o OK do dono.
- Autor dos commits: `-c user.name="movemontes" -c user.email="361244903+movemontes@users.noreply.github.com"`. Termine a mensagem com `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Nunca use o nome, o logo ou a marca MuseScore dentro do app. Mencionar a origem no README e no site pode.
- Nunca apague arquivos do dono: mande para a Lixeira (`~/.Trash`).
- Tudo o que for gerado fica no disco Dados. No Mac, compile só com `/Volumes/Dados/PautaCodigo/build_seguro.sh build 3`: muitos processos já travaram o Mac.
- Não mexa na pasta do Treco; ler pode.
- Nada de ícone na barra de menus.
- Fontes Clash Grotesk: a licença deixa ir dentro do app, mas não num repositório público. Ficam fora do Git, em `/Volumes/Dados/PautaCodigo/fontes-privadas`.

## O ponto principal: onde compilar
Este Mac não compila para Windows: o MuseScore no Windows precisa do Visual Studio (MSVC).

**Caminho recomendado: GitHub Actions**, com uma máquina Windows na nuvem do GitHub.
- O repositório é público, então os minutos são grátis.
- O Actions já está ligado nele.
- Já existe `.github/workflows/build_windows.yml`, vindo do MuseScore, mas ele depende de segredos deles (Sentry, Transifex, assinatura na AWS, servidor OSUOSL). Por isso não serve como está.
- Crie um workflow próprio e enxuto, por exemplo `.github/workflows/pauta_windows.yml`:
  - roda só quando alguém manda rodar (`workflow_dispatch`);
  - usa o `runs-on: windows-2025` e reaproveita `buildscripts/ci/windows/setup.bat`, `build.bat` e `package.bat` como base;
  - entrega o instalador como artefato da execução.
- As mesmas opções de CMake do Mac: `MUSE_MODULE_CLOUD=OFF`, `MUSE_MODULE_MUSESAMPLER=OFF`, `MUSE_MODULE_LEARN=OFF`, `MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT=OFF`, `MUSE_MODULE_VST=ON`, `MUSE_APP_BUILD_MODE=release`, Qt 6.10.2. Confira em `COMPILAR.md`.
- Atenção: o push que cria o workflow precisa do OK do dono, e cada execução leva tempo (mais de 1 hora no MuseScore original). Use o Monitor/`gh run watch` em segundo plano, sem ficar consultando a cada pouco.

**Testar:** o dono provavelmente não tem um PC com Windows. Pergunte. Uma alternativa é uma máquina virtual com Windows 11 ARM no Mac (UTM ou Parallels), que roda programas x64. Peça permissão antes de baixar qualquer coisa.

## O que hoje só funciona no Mac (precisa de versão Windows)
Caminhos relativos ao executável. No Mac é `Pauta.app/Contents/MacOS/Pauta`; no Windows, `bin/Pauta.exe`, com os dados em `share/` ao lado. O original já usa `../Resources` no Mac e outro lugar no Windows (veja `globalconfiguration.cpp:75`). Siga esse mesmo padrão.

1. **Atualização automática.** `src/appshell/internal/applicationactioncontroller.cpp`, função `quit`, perto da linha 318. No Mac, um script `sh` monta o .dmg e troca o app. No Windows o código cai em `openUrl(installer)`, que só abre o instalador. É aceitável, mas o melhor é rodar o .msi em modo silencioso depois de fechar.
   - O atualizador escolhe o arquivo da versão pela extensão (`appupdateservice.cpp`, função `platformFileSuffix`: Windows = `msi`). Então basta pôr um `Pauta-windows.msi` na mesma versão do GitHub, ao lado do `Pauta-mac.dmg`. O nome fixo deixa o link "latest/download" do site funcionando.
2. **Fontes Clash Grotesk.** `src/framework/ui/internal/uiconfiguration.cpp:658` procura em `../Resources/fonts/`.
   - Elas não podem ir para o repositório. Sem elas, o app usa a Archivo, que funciona.
   - Pergunte ao dono se quer pôr as fontes como segredo do GitHub (por exemplo em base64) ou se aceita a Archivo no Windows.
3. **Som GeneralUser GS.** `src/framework/audio/engine/internal/audioengineconfiguration.cpp:116` procura `../Resources/sound/`. O arquivo .sf2 (32 MB) fica fora do Git, em `/Volumes/Dados/PautaCodigo/downloads/GeneralUser-GS`. A licença permite distribuir: o workflow pode baixar da fonte oficial.
4. **Modelos novos e passeio inicial.** `newscoremodel.cpp:54` e `pautatourmodel.cpp:53` usam `../Resources/templates`.
5. **FFmpeg dentro do app (exportar vídeo).** `src/framework/media/internal/ffmpegutils.cpp:62` procura `../Frameworks/ffmpeg`. No Windows, ponha as DLLs ao lado do .exe ou numa subpasta. O script `buildscripts/pauta/build_ffmpeg.sh` é só para Mac; no Windows dá para usar um FFmpeg LGPL pronto, com a licença junto.
6. **Denigma (abrir arquivos do Finale).** `src/project/internal/projectactionscontroller.cpp:176` chama `applicationDirPath() + "/denigma"`. No Windows precisa ser `denigma.exe`, compilado no workflow a partir do código (MIT, github.com/rpatters1/denigma, v3.1).
7. **MIDI.** O aquecimento rápido do MIDI na abertura é só do Mac (`midimodule.cpp`, `Q_OS_MACOS`). O Windows usa as portas originais e não precisa de mudança.
8. **Ícone.** Hoje só existe o `.icns` do Mac (`share/icons/AppIcon/`). O Windows precisa de um `.ico`: gere a partir de `PautaAppIcon_1024.png` (veja `buildscripts/pauta/make_icon.py`).
9. **Atalhos.** `shortcuts.xml` (Windows) já recebeu as mudanças do Pauta junto com o do Mac. Confira mesmo assim.
10. **Assinatura.** No Mac a assinatura é ad-hoc. No Windows, sem certificado, o SmartScreen mostra "O Windows protegeu o computador": avise no site como passar por esse aviso.

## Depois que o Windows funcionar
- No site (`docs/index.html` e `docs/en/index.html`): o botão de download, os requisitos e a tabela de comparação, que hoje fala em "Atualização no Mac".
- No README (`README.md` e `README.en.md`).
- O texto de cada versão segue o padrão com os blocos `<!-- pt -->` e `<!-- /pt -->`; veja as versões anteriores no GitHub.
- Faça a primeira versão Windows como "pré-lançamento" até o dono testar.

## Ajudas de teste que já existem (variáveis de ambiente, para todos os sistemas)
- `PAUTA_DEBUG_FAKE_VERSION=1.0.7`: o app acha que é a 1.0.7 e oferece a atualização.
- `PAUTA_DEBUG_UPDATE_ACCEPT=1`: aceita a atualização sem clique, baixa e fecha.
- `PAUTA_DEBUG_BENCH=<arquivo.tsv>`: mede o tempo das edições. Veja `pautabenchmark.cpp`.
- `PAUTA_DEBUG_PALETTE_TYPE`, `PAUTA_DEBUG_ACTION`, `PAUTA_DEBUG_OPEN_STUDY` e outros: procure `PAUTA_DEBUG` no código.
