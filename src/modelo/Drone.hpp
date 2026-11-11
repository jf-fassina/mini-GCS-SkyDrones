// Implementação de gcs::Veiculo usando MAVSDK (ArduPilot).
#pragma once

#include <memory>
#include <string>

#include "modelo/Veiculo.hpp"

namespace gcs {

/// Converte "tcp:host:porta" para a URL que o MAVSDK espera.
///
/// O MAVSDK aceita `tcpout://host:porta`, `tcpin://:porta`, `udpin://:porta`,
/// `serial:///dev/ttyUSB0`. O SITL do ArduPilot abre um *servidor* TCP na porta
/// 5760, então a GCS conecta para fora: `tcpout://`. Aceitamos também a forma
/// curta usada na comunidade ArduPilot (`tcp:127.0.0.1:5760`) para não
/// confundir ninguém.
std::string normalizar_endereco(const std::string& endereco);

/// Multirrotor ArduPilot acessado via MAVSDK.
///
/// Esta é a ÚNICA classe do projeto que conhece o MAVSDK — e, graças ao idioma
/// *pimpl* (o ponteiro `impl_` abaixo), nem este cabeçalho inclui a biblioteca.
/// Todo o resto da GCS conversa apenas com a abstração gcs::Veiculo, o que
/// permite trocar o autopiloto, ou testar a interface com gcs::VeiculoSimulado,
/// sem mexer em uma linha do console ou dos painéis.
class Drone : public Veiculo {
public:
  /// Endereço padrão do ArduPilot SITL rodando na própria máquina.
  static constexpr const char* ENDERECO_PADRAO = "tcp:127.0.0.1:5760";
  /// Quanto tempo esperar pelo primeiro contato com o autopiloto.
  static constexpr double TEMPO_LIMITE_S = 30.0;

  explicit Drone(std::string nome = "drone",
                 std::string endereco = ENDERECO_PADRAO);
  ~Drone() override;

  const std::string& endereco() const { return endereco_; }

  void conectar() override;
  void iniciar_telemetria() override;
  void alterar_modo_de_voo(const std::string& modo) override;

private:
  class Impl; // esconde o MAVSDK deste cabeçalho
  std::unique_ptr<Impl> impl_;
  std::string endereco_;
};

} // namespace gcs
