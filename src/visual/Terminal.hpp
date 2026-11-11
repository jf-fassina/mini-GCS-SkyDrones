// Pequenos auxiliares ANSI para desenhar no terminal.
//
// Toda a "parte gráfica" do esqueleto é feita com sequências de escape ANSI:
// nada para instalar, funciona por SSH, e o aluno vê exatamente como o desenho
// é construído. Trocar isto por uma janela de verdade (Dear ImGui + ImPlot, Qt,
// SFML) é justamente o desafio proposto no trabalho — veja o README.
#pragma once

#include <string>

namespace gcs::terminal {

inline constexpr const char* NORMAL = "\033[0m";
inline constexpr const char* NEGRITO = "\033[1m";

/// Move o cursor para o canto superior esquerdo (sem apagar a tela: é isso que
/// evita o piscar entre quadros).
inline constexpr const char* INICIO = "\033[H";
/// Apaga do cursor até o fim da linha.
inline constexpr const char* LIMPAR_LINHA = "\033[K";
inline constexpr const char* LIMPAR_TELA = "\033[2J\033[H";
inline constexpr const char* ESCONDER_CURSOR = "\033[?25l";
inline constexpr const char* MOSTRAR_CURSOR = "\033[?25h";

/// Cor de frente em RGB (24 bits). A maioria dos terminais modernos entende.
inline std::string frente(int vermelho, int verde, int azul) {
    return "\033[38;2;" + std::to_string(vermelho) + ';' + std::to_string(verde) + ';' +
           std::to_string(azul) + 'm';
}

/// Cor de fundo em RGB (24 bits).
inline std::string fundo(int vermelho, int verde, int azul) {
    return "\033[48;2;" + std::to_string(vermelho) + ';' + std::to_string(verde) + ';' +
           std::to_string(azul) + 'm';
}

}  // namespace gcs::terminal
