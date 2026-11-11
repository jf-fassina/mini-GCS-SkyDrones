// Console interativo da estação de controle.
#pragma once

#include <string>

namespace gcs {

class EstacaoDeControle;

/// Laço "ler -> interpretar -> executar" do terminal.
///
/// Roda na thread principal. A telemetria chega em outra thread (a do MAVSDK) e
/// os painéis são desenhados em uma terceira, criada sob demanda por
/// EstacaoDeControle::exibir_painel.
class Console {
public:
    explicit Console(EstacaoDeControle& estacao);

    /// Bloqueia até o usuário digitar `sair` (ou fechar a entrada com Ctrl+D).
    void executar();

private:
    void mostrar_abertura() const;
    void tratar(const std::string& linha);

    EstacaoDeControle& estacao_;
};

}  // namespace gcs
