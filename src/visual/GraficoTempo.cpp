#include "visual/GraficoTempo.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include "visual/Terminal.hpp"

namespace gcs {
namespace {

// Uma série só, então não há legenda: o título já diz o que está desenhado.
// A grade e os eixos ficam discretos para a linha de dados ter destaque.
const std::string COR_SERIE = terminal::frente(0x2A, 0x78, 0xD6);
const std::string COR_APAGADA = terminal::frente(0x82, 0x81, 0x7C);

constexpr char MARCA[] = "*";

std::string numero(double valor, int largura = 8, int decimais = 1) {
    std::ostringstream fluxo;
    fluxo << std::fixed << std::setprecision(decimais) << std::setw(largura) << valor;
    return fluxo.str();
}

}  // namespace

GraficoTempo::GraficoTempo(Campo campo, int largura, int altura, double janela_s)
    : Painel(campo.rotulo + " (" + campo.unidade + ")"),
      campo_(std::move(campo)),
      largura_(std::max(20, largura)),
      altura_(std::max(5, altura)),
      janela_s_(janela_s) {}

std::string GraficoTempo::quadro(const Historico& historico) const {
    std::ostringstream saida;
    const std::string cabecalho =
        std::string(terminal::NEGRITO) + titulo() + terminal::NORMAL;

    const Historico::Serie serie = historico.serie(campo_.extrator);
    if (serie.vazia()) {
        saida << cabecalho << terminal::LIMPAR_LINHA << "\n\n"
              << "aguardando telemetria..." << terminal::LIMPAR_LINHA << "\n";
        return saida.str();
    }

    // 1. Recorta a janela de tempo mais recente.
    const double fim = serie.segundos.back();
    const double inicio = std::max(0.0, fim - janela_s_);
    std::vector<double> tempos;
    std::vector<double> valores;
    for (std::size_t i = 0; i < serie.segundos.size(); ++i) {
        if (serie.segundos[i] >= inicio) {
            tempos.push_back(serie.segundos[i]);
            valores.push_back(serie.valores[i]);
        }
    }

    // 2. Escala vertical, com uma folga para a linha não encostar na borda.
    double minimo = *std::min_element(valores.begin(), valores.end());
    double maximo = *std::max_element(valores.begin(), valores.end());
    const double folga = std::max(0.5, (maximo - minimo) * 0.15);
    minimo -= folga;
    maximo += folga;

    // 3. Pinta os pontos em uma grade de caracteres.
    std::vector<std::string> grade(static_cast<std::size_t>(altura_),
                                   std::string(static_cast<std::size_t>(largura_), ' '));
    const double duracao = std::max(1e-6, fim - inicio);
    const double amplitude = std::max(1e-6, maximo - minimo);
    for (std::size_t i = 0; i < tempos.size(); ++i) {
        const int coluna = static_cast<int>(
            std::lround((tempos[i] - inicio) / duracao * (largura_ - 1)));
        const int linha = static_cast<int>(
            std::lround((maximo - valores[i]) / amplitude * (altura_ - 1)));
        if (coluna >= 0 && coluna < largura_ && linha >= 0 && linha < altura_) {
            grade[static_cast<std::size_t>(linha)][static_cast<std::size_t>(coluna)] = MARCA[0];
        }
    }

    // 4. Monta o texto final. Cada linha termina com LIMPAR_LINHA para apagar
    //    o que sobrou do quadro anterior.
    saida << cabecalho << "   atual: " << numero(valores.back(), 0) << ' '
          << campo_.unidade << terminal::LIMPAR_LINHA << "\n";
    for (int linha = 0; linha < altura_; ++linha) {
        std::string rotulo(8, ' ');
        if (linha == 0) {
            rotulo = numero(maximo);
        } else if (linha == altura_ - 1) {
            rotulo = numero(minimo);
        } else if (linha == altura_ / 2) {
            rotulo = numero((maximo + minimo) / 2.0);
        }
        saida << COR_APAGADA << rotulo << " |" << terminal::NORMAL << COR_SERIE
              << grade[static_cast<std::size_t>(linha)] << terminal::NORMAL
              << terminal::LIMPAR_LINHA << "\n";
    }
    saida << COR_APAGADA << std::string(9, ' ') << '+'
          << std::string(static_cast<std::size_t>(largura_), '-') << terminal::NORMAL
          << terminal::LIMPAR_LINHA << "\n";
    saida << COR_APAGADA << std::string(10, ' ') << numero(inicio, 0) << " s"
          << std::string(static_cast<std::size_t>(std::max(1, largura_ - 20)), ' ')
          << numero(fim, 0) << " s" << terminal::NORMAL << terminal::LIMPAR_LINHA << "\n";
    saida << "\n" << COR_APAGADA << "[Enter] volta para o console" << terminal::NORMAL
          << terminal::LIMPAR_LINHA << "\n";
    return saida.str();
}

const std::map<std::string, GraficoTempo::Campo>& campos_disponiveis() {
    static const std::map<std::string, GraficoTempo::Campo> catalogo{
        {"altitude",
         {"Altitude relativa", "m",
          [](const EstadoVeiculo& estado) { return estado.posicao.altitude_relativa_m; }}},
        {"bateria",
         {"Bateria", "%",
          [](const EstadoVeiculo& estado) { return estado.bateria.percentual; }}},
    };
    return catalogo;
}

std::string nomes_dos_campos() {
    std::string nomes;
    for (const auto& par : campos_disponiveis()) {
        nomes += (nomes.empty() ? "" : ", ") + par.first;
    }
    return nomes;
}

// TODO (aluno): ideias de evolução para este painel —
//  * ligar os pontos com uma linha (interpolar entre colunas vizinhas);
//  * marcar o instante da decolagem com uma coluna vertical;
//  * mostrar duas grandezas comparáveis no mesmo gráfico (com legenda!);
//  * descobrir a largura real do terminal em vez de usar um valor fixo.

}  // namespace gcs
