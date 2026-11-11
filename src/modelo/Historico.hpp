// Buffer circular com os últimos estados recebidos do veículo.
//
// É o que alimenta os gráficos. Fica registrado como observador do veículo,
// portanto não sabe de onde a telemetria vem.
#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <mutex>
#include <vector>

#include "modelo/Telemetria.hpp"

namespace gcs {

/// Função que extrai um número de um estado. Exemplo:
///
///     Extrator altitude = [](const EstadoVeiculo& e) {
///         return e.posicao.altitude_relativa_m;
///     };
using Extrator = std::function<double(const EstadoVeiculo&)>;

class Historico {
public:
    /// Uma série temporal pronta para desenhar.
    struct Serie {
        std::vector<double> segundos;  // tempo decorrido desde a amostra mais antiga
        std::vector<double> valores;
        bool vazia() const { return valores.empty(); }
    };

    explicit Historico(std::size_t capacidade = 2000);

    /// Recebe um estado novo. É este método que registramos como observador do
    /// veículo, por isso precisa ser seguro para chamar de outra thread.
    void registrar(const EstadoVeiculo& estado);

    std::size_t tamanho() const;
    bool vazio() const;
    /// Último estado recebido; `ok == false` quando ainda não chegou nada.
    EstadoVeiculo ultimo(bool* ok = nullptr) const;
    /// Extrai uma grandeza de todas as amostras guardadas.
    Serie serie(const Extrator& extrator) const;
    void limpar();

    // TODO (aluno): exportar o histórico para CSV/JSON e analisar o voo depois
    // (`void exportar_csv(const std::string& caminho) const;`).

private:
    mutable std::mutex mutex_;
    std::size_t capacidade_;
    std::deque<EstadoVeiculo> estados_;
};

}  // namespace gcs
