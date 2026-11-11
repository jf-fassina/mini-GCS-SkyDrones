// A estação de controle: junta o veículo, o histórico, os comandos e os
// painéis.
#pragma once

#include <atomic>
#include <memory>

#include "cli/Comando.hpp"
#include "modelo/Historico.hpp"
#include "modelo/Veiculo.hpp"
#include "visual/Painel.hpp"

namespace gcs {

/// Dona de tudo e intermediária entre as camadas.
///
/// Note que ela guarda um `unique_ptr<Veiculo>`, não um `Drone`: a estação
/// funciona igual com o drone de verdade ou com o simulado. Esse é o ponto
/// central do trabalho — programar contra a abstração, não contra a
/// implementação.
class EstacaoDeControle {
public:
  EstacaoDeControle(std::unique_ptr<Veiculo> veiculo,
                    RegistroDeComandos comandos);

  /// Conecta, liga a telemetria e entrega o terminal ao usuário.
  /// Só retorna quando a estação for encerrada.
  void executar();

  Veiculo& veiculo() { return *veiculo_; }
  const Historico& historico() const { return historico_; }
  RegistroDeComandos& comandos() { return comandos_; }

  /// Mostra um painel em tela cheia, atualizando ao vivo, até o usuário
  /// apertar Enter. É a cola entre um comando e a parte "gráfica".
  void exibir_painel(const Painel& painel);

  void encerrar() { rodando_ = false; }
  bool rodando() const { return rodando_; }

private:
  /// Dá um tempo para o primeiro pacote de telemetria chegar, para que o
  /// primeiro `status` já mostre dados de verdade.
  void esperar_primeira_telemetria(double tempo_limite_s = 3.0);

  std::unique_ptr<Veiculo> veiculo_;
  Historico historico_; // precisa ser declarado antes do renderizador_
  RegistroDeComandos comandos_;
  Renderizador renderizador_; // guarda uma referência para historico_
  std::atomic<bool> rodando_{true};
};

} // namespace gcs
