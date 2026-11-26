/*
 * SPDX-License-Identifier: GPL-3.0-only
 */
#include "pautatranslator.h"

using namespace muse::languages;

// English text of strings that exist only in Pauta -> Portuguese (Brazil).
static const QHash<QString, QString>& pautaPortuguese()
{
    static const QHash<QString, QString> table {
        { "&About Pauta…", "&Sobre o Pauta…" },
        { "About Pauta", "Sobre o Pauta" },
        { "&Language", "&Idioma" },
        { "Language", "Idioma" },
        { "Automatic (system)", "Automático (do sistema)" },
        { "Português (Brasil)", "Português (Brasil)" },
        { "English", "English" },
        { "Pauta needs to restart to change the language.", "O Pauta precisa reiniciar para trocar o idioma." },
        { "Restart now", "Reiniciar agora" },
        { "Later", "Depois" },
        { "Support Pauta with Pix…", "Apoiar o Pauta com Pix…" },
        { "Thank you note", "Recado de agradecimento" },
        { "HELP ME: I'VE GOT 5 MOUTHS TO FEED", "ME AJUDA: TENHO 5 BOCAS PRA ALIMENTAR" },
        { "and I made this app for free, for you.", "e fiz esse app de graça pra você." },
        { "Hi, I'm Mateus. Pauta has no ads, needs no account and never uploads your scores.",
          "Oi, eu sou o Mateus. O Pauta não tem anúncio, não pede conta e não manda nada seu pra internet." },
        { "If it makes your music easier, any amount helps my family a lot. Thank you so much!",
          "Se ele facilitar a sua música, qualquer valor ajuda muito aqui em casa. Obrigado de coração!" },
        { "Pix (random key, PicPay):", "Pix (chave aleatória, PicPay):" },
        { "Copy Pix key", "Copiar chave Pix" },
        { "Copied!", "Copiado!" },
        { "Let's go", "Bora usar" },
        { "Pauta is free software based on MuseScore Studio.", "O Pauta é um programa livre baseado no MuseScore Studio." },
        { "Pauta is not affiliated with or endorsed by MuseScore Limited or Muse Group.",
          "O Pauta não tem ligação com a MuseScore Limited nem com o Muse Group." },
        { "Made by Mateus (movemontes). Free, no ads, no account.", "Feito por Mateus (movemontes). Grátis, sem anúncio e sem conta." },
        { "Source code", "Código-fonte" },
        { "Hide the window on close and keep playing", "Esconder a janela ao fechar e continuar tocando" },
        { "Light mode (no reverb, lighter sound)", "Modo leve (sem reverb, som mais leve)" },
        { "Play repeat only the 2nd time", "Tocar só na 2ª vez da repetição" },
        { "Command palette…", "Paleta de comandos…" },
        { "Type a command", "Digite um comando" },
        { "Tap tempo", "Bater o tempo" },
        { "Reverb (on/off)", "Reverb (ligar/desligar)" },
        { "Change key…", "Mudar tom…" },
        { "Tools", "Ferramentas" },
        { "Write", "Escrever" },
        { "Lyrics", "Letra" },
        { "Chord", "Cifra" },
        { "Slur", "Ligadura" },
        { "Text", "Texto" },
        { "Tempo", "Andamento" },
        { "Change key", "Mudar tom" },
        { "Chords from notes", "Cifras das notas" },
        { "Play", "Tocar" },
        { "Metronome", "Metrônomo" },
        { "Zoom in", "Aproximar" },
        { "Zoom out", "Afastar" },
        { "Find", "Buscar" },
        { "Quick guide", "Guia rápido" },
        { "Chord symbols from selected notes", "Cifras a partir das notas selecionadas" },
        { "Select some notes first", "Selecione algumas notas antes" },
        { "Select the notes of the chord (or a whole passage) and try again.", "Selecione as notas do acorde (ou um trecho inteiro) e tente de novo." },
        { "No chord recognised", "Nenhum acorde reconhecido" },
        { "Select at least two different notes that sound together.", "Selecione pelo menos duas notas diferentes que soam juntas." },
        { "Write like Finale (Speedy Entry) on/off", "Escrever como no Finale (Speedy Entry): ligar/desligar" },
        { "Writing like Finale is on", "Escrever como no Finale: ligado" },
        { "Writing like Finale is off", "Escrever como no Finale: desligado" },
        { "Press N to start writing. Move the cursor with the arrows or click a line of the staff, then press the duration: 4 eighth, 5 quarter, 6 half, 7 whole. Use up and down arrows to change the pitch.",
          "Aperte N para começar. Leve o cursor com as setas ou clicando numa linha da pauta e aperte a duração: 4 colcheia, 5 semínima, 6 mínima, 7 semibreve. Setas para cima e para baixo mudam a altura." },
        { "Back to the usual way: press N, then type the note name (A to G).", "De volta ao jeito comum: aperte N e digite o nome da nota (A a G)." },
        { "Drums", "Bateria" },
        { "Closed hi-hat", "Chimbal fechado" },
        { "Half-open hi-hat", "Chimbal meio aberto" },
        { "Open hi-hat", "Chimbal aberto" },
        { "Choke (stop the cymbal)", "Abafar prato" },
        { "Rim shot", "Aro (rim shot)" },
        { "Let ring", "Deixar soar" },
        { "Sticking: right hand", "Baquetamento: mão direita (R)" },
        { "Sticking: left hand", "Baquetamento: mão esquerda (L)" },
        { "Couldn't open this Finale file", "Não deu para abrir este arquivo do Finale" },
        { "Pauta could not convert it. You can also export it from Finale as MusicXML and open that.", "O Pauta não conseguiu converter. Se tiver o Finale, exporte como MusicXML e abra esse arquivo." },
        { "Finale files", "Arquivos do Finale" },
        { "Free sheet music app", "Programa de partitura grátis" },
        { "Open PDF as editable score (beta)…", "Abrir PDF como partitura editável (beta)…" },
        { "To read sheet music from a PDF, Pauta uses Audiveris", "Para ler partitura de PDF, o Pauta usa o Audiveris" },
        { "Audiveris is a free program that turns printed sheet music (PDF or photo) into an editable score, right on your Mac. Download the version for Apple chips (macosx-arm64.dmg), drag it to Applications, open it once, and then come back here.",
          "O Audiveris é um programa grátis que transforma partitura impressa (PDF ou foto) em partitura editável, no próprio Mac. Baixe a versão para chip Apple (macosx-arm64.dmg), arraste para Aplicativos, abra uma vez e volte aqui." },
        { "Download Audiveris", "Baixar o Audiveris" },
        { "Choose the PDF or picture of the sheet music", "Escolha o PDF ou a foto da partitura" },
        { "PDF and pictures", "PDF e imagens" },
        { "Reading the sheet music… (this can take a minute)", "Lendo a partitura… (pode levar um minuto)" },
        { "Couldn't read this sheet music", "Não deu para ler esta partitura" },
        { "It works best with clean printed scores (not handwritten or blurry photos).", "Funciona melhor com partitura impressa e nítida (não manuscrita nem foto borrada)." },
        { "Change the key of the whole score", "Mudar o tom da partitura inteira" },
        { "Export to another program…", "Exportar para outro programa…" },
        { "Export to another program", "Exportar para outro programa" },
        { "Where do you want to use this score?", "Onde você quer usar esta partitura?" },
        { "MuseScore Studio opens Pauta files (.mscz) directly, no export needed.", "O MuseScore Studio abre os arquivos do Pauta (.mscz) direto, sem exportar." },
        { "Notation programs. Keeps notes, lyrics, chords and layout (MusicXML).", "Programas de partitura. Leva notas, letra, cifras e diagramação (MusicXML)." },
        { "Music production (DAW). Notes and instruments only, no sheet music (MIDI).", "Produção musical (DAW). Só notas e instrumentos, sem a partitura (MIDI)." },
        { "Print, WhatsApp, e-mail", "Imprimir, WhatsApp, e-mail" },
        { "Ready-to-read sheet music, not editable (PDF).", "Partitura pronta para ler, não editável (PDF)." },
        { "Video editor, social media, music player", "Editor de vídeo, redes sociais, tocador de música" },
        { "The sound of the score (MP3).", "O som da partitura (MP3)." },
        { "Braille reader", "Leitor de braille" },
        { "Braille music for blind musicians (BRF).", "Partitura em braille para músicos cegos (BRF)." },
        { "Language: Automatic (system)", "Idioma: Automático (do sistema)" },
        { "Language: Português (Brasil)", "Idioma: Português (Brasil)" },
        { "Language: English", "Idioma: English" },
    };
    return table;
}

PautaTranslator::PautaTranslator(const QString& languageCode, QObject* parent)
    : QTranslator(parent), m_portuguese(languageCode.startsWith(u"pt"))
{
}

PautaTranslator::~PautaTranslator()
{
    for (QTranslator* t : m_translators) {
        delete t;
    }
}

void PautaTranslator::addTranslator(QTranslator* translator)
{
    m_translators.push_back(translator);
}

QString PautaTranslator::brand(QString text)
{
    text.replace(QStringLiteral("MuseScore Studio"), QStringLiteral("Pauta"));
    return text;
}

QString PautaTranslator::translate(const char* context, const char* sourceText, const char* disambiguation, int n) const
{
    if (!sourceText) {
        return QString();
    }

    // Pauta-only strings: Portuguese from the table, English as written (never re-branded)
    auto it = pautaPortuguese().constFind(QString::fromUtf8(sourceText));
    if (it != pautaPortuguese().cend()) {
        return m_portuguese ? it.value() : QString::fromUtf8(sourceText);
    }

    for (auto it = m_translators.rbegin(); it != m_translators.rend(); ++it) {
        QString result = (*it)->translate(context, sourceText, disambiguation, n);
        if (!result.isNull() && !result.isEmpty()) {
            return brand(result);
        }
    }

    QString source = QString::fromUtf8(sourceText);
    QString branded = brand(source);
    // Returning a null string lets Qt use the source text itself (keeps its own %n handling).
    return branded == source ? QString() : branded;
}
