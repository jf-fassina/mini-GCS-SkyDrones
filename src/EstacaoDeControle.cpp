#include "EstacaoDeControle.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <utility>

#include "cli/Console.hpp"

namespace gcs {

EstacaoDeControle::EstacaoDeControle(std::unique_ptr<Veiculo> veiculo,
                                     RegistroDeComandos comandos)
    : veiculo_(std::move(veiculo)),
      historico_(),
      comandos_(std::move(comandos)),
      renderizador_(historico_) {}

void EstacaoDeControle::executar() {
    // O histórico passa a receber cada atualização de telemetria. O veículo não
    // sabe o que é um histórico: só conhece a assinatura do observador.
    veiculo_->registrar_observador(
        [this](const EstadoVeiculo& estado) { historico_.registrar(estado); });

    veiculo_->conectar();
    veiculo_->iniciar_telemetria();
    esperar_primeira_telemetria();

    Console console(*this);
    console.executar();
    std::cout << "encerrando...\n";
}

void EstacaoDeControle::esperar_primeira_telemetria(double tempo_limite_s) {
    // Sem isto, um `status` digitado logo após a conexão mostraria o estado
    // inicial vazio, porque os primeiros pacotes ainda estariam a caminho.
    const auto inicio = std::chrono::steady_clock::now();
    while (historico_.vazio() &&
           segundos_entre(inicio, std::chrono::steady_clock::now()) < tempo_limite_s) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void EstacaoDeControle::exibir_painel(const Painel& painel) {
    // Uma thread desenha o painel sem parar; a thread principal fica presa no
    // getline esperando o Enter. Quando ele chega, pedimos para a outra parar.
    std::atomic<bool> parar{false};
    std::thread desenho([this, &painel, &parar] {
        renderizador_.exibir_enquanto(painel, parar);
    });

    std::string ignorado;
    std::getline(std::cin, ignorado);

    parar = true;
    desenho.join();
}

}  // namespace gcs
