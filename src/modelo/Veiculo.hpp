// Classe base de todos os veículos conhecidos pela estação de controle.
#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <ostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "modelo/Telemetria.hpp"

namespace gcs {

/// Erro ao falar com o veículo (conexão, comando rejeitado...).
class ErroDeVeiculo : public std::runtime_error {
public:
  explicit ErroDeVeiculo(const std::string& mensagem)
      : std::runtime_error(mensagem) {}
};

/// Operação que ainda é exercício da disciplina. O console trata esta exceção
/// de forma amigável, então dá para registrar o esqueleto de um recurso antes
/// de implementá-lo.
class NaoImplementado : public std::logic_error {
public:
  explicit NaoImplementado(const std::string& o_que)
      : std::logic_error(o_que) {}
};

/// Contrato comum a qualquer veículo: multirrotor, asa fixa, barco, simulador.
///
/// A classe base resolve três responsabilidades genéricas:
///
///  1. guardar o último EstadoVeiculo conhecido, com acesso seguro entre
///  threads;
///  2. avisar os interessados a cada atualização (padrão *Observer*) — é assim
///     que o histórico e os painéis recebem dados sem conhecer o MAVSDK;
///  3. declarar as operações que toda subclasse precisa oferecer.
///
/// O que é específico de um protocolo fica nas subclasses: veja gcs::Drone
/// (MAVSDK + ArduPilot) e gcs::VeiculoSimulado (dados falsos para testes).
class Veiculo {
public:
  /// Quem quer ser avisado quando o estado muda.
  /// Atenção: é chamado na thread da telemetria, não na do console.
  using Observador = std::function<void(const EstadoVeiculo&)>;

  explicit Veiculo(std::string nome);
  virtual ~Veiculo() = default;

  Veiculo(const Veiculo&) = delete;
  Veiculo& operator=(const Veiculo&) = delete;

  // --- leitura de estado -------------------------------------------------
  const std::string& nome() const { return nome_; }
  /// Cópia do último estado conhecido (segura para usar em qualquer thread).
  EstadoVeiculo estado() const;
  /// Modo de voo atual, p.ex. "STABILIZE", "GUIDED", "RTL".
  std::string modo_de_voo() const;

  // --- observer ----------------------------------------------------------
  void registrar_observador(Observador observador);

  // --- operações que toda subclasse precisa implementar ------------------
  /// Estabelece a conexão; só retorna quando o veículo responder.
  /// Lança ErroDeVeiculo em caso de falha.
  virtual void conectar() = 0;
  /// Assina os fluxos de telemetria (não bloqueia: os dados chegam por
  /// callback).
  virtual void iniciar_telemetria() = 0;
  /// Pede ao autopiloto a troca do modo de voo.
  virtual void alterar_modo_de_voo(const std::string& modo) = 0;

  // TODO (aluno): declare aqui o resto do contrato que a sua GCS precisa e
  // implemente nas subclasses:
  //
  //     virtual void armar() = 0;
  //     virtual void desarmar() = 0;
  //     virtual void decolar(double altura_m) = 0;
  //     virtual void pousar() = 0;
  //     virtual void retornar_para_casa() = 0;

protected:
  /// Modifica o estado e notifica os observadores.
  ///
  /// Usada pelas subclasses quando a telemetria chega:
  ///
  ///     atualizar([&](EstadoVeiculo& e) { e.modo_de_voo = "GUIDED"; });
  void atualizar(const std::function<void(EstadoVeiculo&)>& modificacao);

private:
  std::string nome_;
  mutable std::mutex mutex_; // protege estado_ e observadores_
  EstadoVeiculo estado_;
  std::vector<Observador> observadores_;
};

/// Permite `std::cout << veiculo` (usado pelo comando `status`).
std::ostream& operator<<(std::ostream& fluxo, const Veiculo& veiculo);

/// Veículo falso: gera telemetria sintética, sem precisar do SITL ligado.
///
/// Serve de segundo exemplo de herança — a estação de controle não sabe (nem
/// precisa saber) se está falando com o ArduPilot ou com esta classe. Use
/// `./mini_gcs --simulado` para desenvolver a interface no avião, no ônibus,
/// em qualquer lugar.
class VeiculoSimulado : public Veiculo {
public:
  explicit VeiculoSimulado(std::string nome = "simulado");
  ~VeiculoSimulado() override;

  void conectar() override;
  void iniciar_telemetria() override;
  void alterar_modo_de_voo(const std::string& modo) override;

private:
  void laco_de_telemetria();

  std::atomic<bool> parar_{false};
  std::thread thread_;
};

} // namespace gcs
