/*
 * SPDX-License-Identifier: GPL-3.0-only
 * Pauta: quick guide with the basics, for people who never used a notation program.
 */

import QtQuick

import Muse.Ui
import Muse.UiComponents

StyledDialogView {
    id: root

    title: pt ? "Guia rápido" : "Quick guide"

    contentWidth: 640
    contentHeight: 560

    readonly property bool pt: Qt.locale().name.startsWith("pt")

    readonly property var sections: pt ? [
        { "t": "1. Começar", "b": "Na tela inicial, clique em Nova partitura, escolha os instrumentos (piano, violão, bateria, voz…) e confirme." },
        { "t": "2. Escrever notas", "b": "Clique num compasso e aperte N. Digite as notas: C D E F G A B (Dó Ré Mi Fá Sol Lá Si). Esc para parar." },
        { "t": "3. Duração", "b": "Antes da nota, aperte a duração: 4 colcheia, 5 semínima, 6 mínima, 7 semibreve (3 semicolcheia). Ponto: tecla ponto. Pausa: 0." },
        { "t": "4. Corrigir", "b": "Seta para cima/baixo sobe ou desce meio tom. Cmd+seta muda a oitava. Cmd+Z desfaz." },
        { "t": "5. Ouvir", "b": "Espaço toca e para. O teclado de piano (menu Exibir) acende as notas enquanto toca." },
        { "t": "6. Letra e cifra", "b": "Selecione a nota e aperte Cmd+L para letra, Cmd+K para cifra. Ou selecione as notas e use Cifras a partir das notas selecionadas." },
        { "t": "7. Mudar o tom", "b": "Botão Mudar tom, na barra de cima. Escolha o tom novo e confirme." },
        { "t": "8. Bateria", "b": "Paleta Bateria: nota fantasma, chimbal aberto/fechado, flam, rufo, baquetamento. Painel de percussão: menu Exibir." },
        { "t": "9. Mandar para alguém", "b": "Exportar para outro programa: PDF para imprimir ou WhatsApp, MusicXML para Sibelius/Finale/Dorico, MIDI para GarageBand." },
        { "t": "10. Não achou?", "b": "Cmd+Shift+P e digite o que procura (ex.: transpor, ligadura, compasso)." }
    ] : [
        { "t": "1. Start", "b": "On the home screen, click New score, choose the instruments (piano, guitar, drums, voice…) and confirm." },
        { "t": "2. Write notes", "b": "Click a bar and press N. Type the notes: C D E F G A B. Esc to stop." },
        { "t": "3. Duration", "b": "Before the note, press the duration: 4 eighth, 5 quarter, 6 half, 7 whole (3 sixteenth). Dot: period key. Rest: 0." },
        { "t": "4. Fix", "b": "Up/down arrow moves a half step. Cmd+arrow changes the octave. Cmd+Z undoes." },
        { "t": "5. Listen", "b": "Space plays and stops. The piano keyboard (View menu) lights up the notes while playing." },
        { "t": "6. Lyrics and chords", "b": "Select the note and press Cmd+L for lyrics, Cmd+K for a chord symbol. Or select notes and use Chord symbols from selected notes." },
        { "t": "7. Change key", "b": "Change key button on the top bar. Choose the new key and confirm." },
        { "t": "8. Drums", "b": "Drums palette: ghost note, open/closed hi-hat, flam, roll, sticking. Percussion panel: View menu." },
        { "t": "9. Send to someone", "b": "Export to another program: PDF to print or share, MusicXML for Sibelius/Finale/Dorico, MIDI for GarageBand." },
        { "t": "10. Can't find it?", "b": "Cmd+Shift+P and type what you are looking for (e.g. transpose, slur, bar)." }
    ]

    StyledFlickable {
        anchors.fill: parent
        anchors.margins: 20
        contentHeight: column.implicitHeight

        Column {
            id: column
            width: parent.width
            spacing: 14

            Repeater {
                model: root.sections
                delegate: Column {
                    width: column.width
                    spacing: 3
                    StyledTextLabel {
                        width: parent.width
                        text: modelData.t
                        font: ui.theme.largeBodyBoldFont
                        color: ui.theme.accentColor
                        horizontalAlignment: Text.AlignLeft
                    }
                    StyledTextLabel {
                        width: parent.width
                        text: modelData.b
                        font: ui.theme.bodyFont
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignLeft
                    }
                }
            }
        }
    }
}
