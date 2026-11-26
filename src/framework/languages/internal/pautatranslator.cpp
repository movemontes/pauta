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
