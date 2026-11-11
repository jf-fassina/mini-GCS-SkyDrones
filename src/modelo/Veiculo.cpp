#include "modelo/Veiculo.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <utility>

namespace gcs {

Veiculo::Veiculo(std::string nome) : nome_(std::move(nome)) {}

EstadoVeiculo Veiculo::estado() const {
  std::lock_guard<std::mutex> trava(mutex_);
  return estado_; // cópia
}

std::string Veiculo::modo_de_voo() const {
  std::lock_guard<std::mutex> trava(mutex_);
  return estado_.modo_de_voo;
}

void Veiculo::registrar_observador(Observador observador) {
  std::lock_guard<std::mutex> trava(mutex_);
  observadores_.push_back(std::move(observador));
}

void Veiculo::atualizar(
    const std::function<void(EstadoVeiculo&)>& modificacao) {
  EstadoVeiculo copia;
  std::vector<Observador> observadores;
  {
    std::lock_guard<std::mutex> trava(mutex_);
    modificacao(estado_);
    estado_.instante = std::chrono::steady_clock::now();
    copia = estado_;
    observadores = observadores_;
  }
  // Os observadores são chamados FORA da trava: se um deles pedir estado()
  // de volta, não travamos o programa.
  for (const Observador& observador : observadores) {
    observador(copia);
  }
}

std::ostream& operator<<(std::ostream& fluxo, const Veiculo& veiculo) {
  return fluxo << veiculo.nome() << ": " << veiculo.estado().resumo();
}

// ---------------------------------------------------------------------------
// VeiculoSimulado
// ---------------------------------------------------------------------------
VeiculoSimulado::VeiculoSimulado(std::string nome) : Veiculo(std::move(nome)) {}

VeiculoSimulado::~VeiculoSimulado() {
  parar_ = true;
  if (thread_.joinable()) {
    thread_.join();
  }
}

void VeiculoSimulado::conectar() {
  atualizar([](EstadoVeiculo& estado) { estado.modo_de_voo = "STABILIZE"; });
}

void VeiculoSimulado::iniciar_telemetria() {
  thread_ = std::thread(&VeiculoSimulado::laco_de_telemetria, this);
}

void VeiculoSimulado::laco_de_telemetria() {
  const Instante inicio = std::chrono::steady_clock::now();
  while (!parar_) {
    const double t = segundos_entre(inicio, std::chrono::steady_clock::now());
    atualizar([t](EstadoVeiculo& estado) {
      estado.em_voo = true;
      estado.armado = true;
      estado.atitude.rolagem = 25.0 * std::sin(t / 2.0);
      estado.atitude.arfagem = 10.0 * std::sin(t / 3.0);
      estado.atitude.guinada = std::fmod(t * 10.0, 360.0);
      estado.posicao.altitude_relativa_m = 10.0 + 5.0 * std::sin(t / 5.0);
      estado.bateria.tensao_v = 12.4;
      estado.bateria.percentual = std::max(0.0, 100.0 - t);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}

void VeiculoSimulado::alterar_modo_de_voo(const std::string& modo) {
  std::string maiusculo;
  for (char caractere : modo) {
    maiusculo +=
        static_cast<char>(std::toupper(static_cast<unsigned char>(caractere)));
  }
  atualizar(
      [&maiusculo](EstadoVeiculo& estado) { estado.modo_de_voo = maiusculo; });
}

} // namespace gcs
