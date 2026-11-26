/*
 * Pauta: one translator that wraps MuseScore's .qm translators.
 * - Shows the app name "Pauta" instead of "MuseScore Studio" in every UI string.
 * - Adds Portuguese (Brazil) text for strings that exist only in Pauta.
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <QTranslator>
#include <QHash>
#include <vector>

namespace muse::languages {
class PautaTranslator : public QTranslator
{
public:
    explicit PautaTranslator(const QString& languageCode, QObject* parent = nullptr);
    ~PautaTranslator() override;

    void addTranslator(QTranslator* translator);

    QString translate(const char* context, const char* sourceText, const char* disambiguation = nullptr, int n = -1) const override;
    bool isEmpty() const override { return false; }

    static QString brand(QString text);

private:
    std::vector<QTranslator*> m_translators;
    bool m_portuguese = false;
};
}
