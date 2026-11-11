// Gráfico de uma série temporal de telemetria, desenhado com caracteres.
#pragma once

#include <map>
#include <string>

#include "modelo/Historico.hpp"
#include "visual/Painel.hpp"

namespace gcs {

/// Desenha uma grandeza em função do tempo.
///
/// A mesma classe serve para qualquer grandeza: o que muda é a função que extrai
/// o número do estado do veículo. Por isso o gráfico recebe um Campo em vez de
/// saber ler altitude "na mão".
class GraficoTempo : public Painel {
public:
    /// Descreve uma grandeza que pode virar gráfico.
    struct Campo {
        std::string rotulo;
        std::string unidade;
        Extrator extrator;
    };

    explicit GraficoTempo(Campo campo, int largura = 70, int altura = 16,
                          double janela_s = 60.0);

    std::string quadro(const Historico& historico) const override;

private:
    Campo campo_;
    int largura_;
    int altura_;
    double janela_s_;  // quantos segundos mostrar de uma vez
};

/// Catálogo consultado pelo comando `plotar`.
///
/// TODO (aluno): acrescente campos novos aqui — velocidade no solo, taxa de
/// subida, tensão da bateria, número de satélites, rumo... Cada entrada nova já
/// aparece sozinha no `plotar` e no `ajuda`, sem mexer em mais nada.
const std::map<std::string, GraficoTempo::Campo>& campos_disponiveis();

/// "altitude, bateria, ..." — usado nas mensagens de ajuda e de erro.
std::string nomes_dos_campos();

}  // namespace gcs
