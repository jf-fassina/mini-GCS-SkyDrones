#include "visual/HorizonteArtificial.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <optional>
#include <sstream>
#include <vector>

#include "modelo/Telemetria.hpp"
#include "visual/Terminal.hpp"

namespace gcs {
namespace {

/// Quantos graus cada caractere vale. Como um caractere de terminal é mais ou
/// menos duas vezes mais alto do que largo, uma coluna vale metade de uma linha
/// — é isso que mantém o horizonte com a inclinação correta.
constexpr double GRAUS_POR_LINHA = 4.0;
constexpr double GRAUS_POR_COLUNA = 2.0;

constexpr double PI = 3.14159265358979323846;

/// O que há atrás de cada caractere.
enum class Fundo { Ceu, Chao, Horizonte, Caixa };
/// A cor do caractere desenhado em cima do fundo.
enum class Tinta { Padrao, Simbolo, Texto };

struct Celula {
    Fundo fundo = Fundo::Ceu;
    Tinta tinta = Tinta::Padrao;
    char glifo = ' ';
};

const std::string FUNDO_CEU = terminal::fundo(0x2A, 0x78, 0xD6);
const std::string FUNDO_CHAO = terminal::fundo(0x8C, 0x5A, 0x2B);
const std::string FUNDO_HORIZONTE = terminal::fundo(0xFC, 0xFC, 0xFB);
const std::string FUNDO_CAIXA = terminal::fundo(0x1A, 0x1A, 0x19);
const std::string FRENTE_PADRAO = terminal::frente(0xFC, 0xFC, 0xFB);
const std::string FRENTE_SIMBOLO = terminal::frente(0xED, 0xA1, 0x00);
const std::string FRENTE_TEXTO = terminal::frente(0xFC, 0xFC, 0xFB);

const std::string& codigo(Fundo fundo) {
    switch (fundo) {
        case Fundo::Chao: return FUNDO_CHAO;
        case Fundo::Horizonte: return FUNDO_HORIZONTE;
        case Fundo::Caixa: return FUNDO_CAIXA;
        case Fundo::Ceu: break;
    }
    return FUNDO_CEU;
}

const std::string& codigo(Tinta tinta) {
    switch (tinta) {
        case Tinta::Simbolo: return FRENTE_SIMBOLO;
        case Tinta::Texto: return FRENTE_TEXTO;
        case Tinta::Padrao: break;
    }
    return FRENTE_PADRAO;
}

std::string graus(double valor) {
    std::ostringstream fluxo;
    fluxo << std::fixed << std::setprecision(1) << std::showpos << std::setw(7) << valor;
    return fluxo.str();
}

}  // namespace

HorizonteArtificial::HorizonteArtificial(int largura, int altura)
    : Painel("Horizonte artificial"),
      largura_(std::max(40, largura)),
      altura_(std::max(11, altura | 1)) {}  // altura ímpar: existe uma linha central

std::string HorizonteArtificial::quadro(const Historico& historico) const {
    bool recebeu = false;
    const EstadoVeiculo estado = historico.ultimo(&recebeu);
    std::ostringstream saida;
    if (!recebeu) {
        saida << titulo() << terminal::LIMPAR_LINHA << "\n\n"
              << "aguardando telemetria..." << terminal::LIMPAR_LINHA << "\n";
        return saida.str();
    }

    // 1. Decide, caractere por caractere, se ali é céu, chão ou horizonte.
    const double seno = std::sin(estado.atitude.rolagem * PI / 180.0);
    const double cosseno = std::cos(estado.atitude.rolagem * PI / 180.0);
    const double centro_linha = (altura_ - 1) / 2.0;
    const double centro_coluna = (largura_ - 1) / 2.0;

    std::vector<std::vector<Celula>> tela(
        static_cast<std::size_t>(altura_),
        std::vector<Celula>(static_cast<std::size_t>(largura_)));

    for (int linha = 0; linha < altura_; ++linha) {
        for (int coluna = 0; coluna < largura_; ++coluna) {
            const double y = (centro_linha - linha) * GRAUS_POR_LINHA;
            const double x = (coluna - centro_coluna) * GRAUS_POR_COLUNA;
            const double u = -x * seno + y * cosseno + estado.atitude.arfagem;
            Celula& celula = tela[static_cast<std::size_t>(linha)][static_cast<std::size_t>(coluna)];
            if (std::abs(u) <= GRAUS_POR_LINHA * 0.6) {
                celula.fundo = Fundo::Horizonte;
            } else {
                celula.fundo = (u > 0.0) ? Fundo::Ceu : Fundo::Chao;
            }
        }
    }

    // 2. O símbolo da aeronave, sempre fixo no centro: é a referência do piloto.
    const int linha_central = altura_ / 2;
    const int coluna_central = largura_ / 2;
    // Escreve um caractere por cima do desenho. `fundo` nulo mantém o céu/chão
    // que já estava ali — é o que faz o símbolo da aeronave parecer parte do HUD.
    const auto escrever = [&](int linha, int coluna, char glifo, Tinta tinta,
                              std::optional<Fundo> fundo = std::nullopt) {
        if (linha < 0 || linha >= altura_ || coluna < 0 || coluna >= largura_) {
            return;
        }
        Celula& celula = tela[static_cast<std::size_t>(linha)][static_cast<std::size_t>(coluna)];
        celula.glifo = glifo;
        celula.tinta = tinta;
        if (fundo.has_value()) {
            celula.fundo = *fundo;
        }
    };
    for (int distancia = 5; distancia <= 11; ++distancia) {
        escrever(linha_central, coluna_central - distancia, '=', Tinta::Simbolo);
        escrever(linha_central, coluna_central + distancia, '=', Tinta::Simbolo);
    }
    escrever(linha_central, coluna_central, 'o', Tinta::Simbolo);

    // 3. As leituras numéricas, em uma caixa escura para ficarem legíveis tanto
    //    sobre o céu quanto sobre o chão.
    std::vector<std::string> leituras{
        " modo    " + estado.modo_de_voo,
        " rolagem " + graus(estado.atitude.rolagem) + " deg",
        " arfagem " + graus(estado.atitude.arfagem) + " deg",
        " rumo    " + graus(estado.atitude.guinada) + " deg",
    };
    std::size_t largura_caixa = 0;
    for (const std::string& leitura : leituras) {
        largura_caixa = std::max(largura_caixa, leitura.size() + 1);
    }
    for (std::size_t i = 0; i < leituras.size(); ++i) {
        leituras[i].resize(largura_caixa, ' ');  // deixa a caixa retangular
        for (std::size_t j = 0; j < leituras[i].size(); ++j) {
            escrever(static_cast<int>(i) + 1, static_cast<int>(j) + 2, leituras[i][j],
                     Tinta::Texto, Fundo::Caixa);
        }
    }

    // 4. Emite a tela, trocando as cores só quando elas mudam de verdade.
    saida << terminal::NEGRITO << titulo() << terminal::NORMAL << terminal::LIMPAR_LINHA << "\n";
    for (const std::vector<Celula>& linha : tela) {
        const Fundo* fundo_atual = nullptr;
        const Tinta* tinta_atual = nullptr;
        for (const Celula& celula : linha) {
            if (fundo_atual == nullptr || celula.fundo != *fundo_atual) {
                saida << codigo(celula.fundo);
                fundo_atual = &celula.fundo;
            }
            if (tinta_atual == nullptr || celula.tinta != *tinta_atual) {
                saida << codigo(celula.tinta);
                tinta_atual = &celula.tinta;
            }
            saida << celula.glifo;
        }
        saida << terminal::NORMAL << terminal::LIMPAR_LINHA << "\n";
    }
    saida << "\n"
          << terminal::frente(0x82, 0x81, 0x7C) << "[Enter] volta para o console"
          << terminal::NORMAL << terminal::LIMPAR_LINHA << "\n";
    return saida.str();
}

// TODO (aluno): o horizonte está no mínimo necessário. Melhorias naturais:
//  * escada de arfagem: linhas a cada 10 graus, com o número ao lado;
//  * arco de rolagem no topo, com um ponteiro triangular;
//  * fita de rumo (compasso) na base e fitas de altitude/velocidade nas laterais;
//  * pintar de vermelho quando a rolagem passar de um limite de segurança;
//  * indicador de bateria e de modo de voo com destaque quando houver falha.

}  // namespace gcs
