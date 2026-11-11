#include "modelo/Drone.hpp"

#include <mavsdk/mavsdk.hpp>
#include <mavsdk/plugins/action/action.hpp>
#include <mavsdk/plugins/telemetry/telemetry.hpp>

#include <algorithm>
#include <cctype>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>

namespace gcs {
namespace {

std::string para_maiusculas(std::string texto) {
  std::transform(texto.begin(), texto.end(), texto.begin(),
                 [](unsigned char letra) {
                   return static_cast<char>(std::toupper(letra));
                 });
  return texto;
}

/// O MAVSDK imprime os enums com operator<<; é a forma mais simples de
/// transformar um FlightMode em texto.
template <typename T> std::string para_texto(const T& valor) {
  std::ostringstream fluxo;
  fluxo << valor;
  return fluxo.str();
}

} // namespace

std::string normalizar_endereco(const std::string& endereco) {
  if (endereco.find("://") != std::string::npos) {
    return endereco; // já está no formato de URL
  }
  const std::size_t separador = endereco.find(':');
  if (separador == std::string::npos) {
    return endereco;
  }
  std::string esquema = endereco.substr(0, separador);
  const std::string resto = endereco.substr(separador + 1);
  // "tcp" é ambíguo para o MAVSDK; para falar com o SITL queremos "tcpout".
  if (esquema == "tcp") {
    esquema = "tcpout";
  }
  return esquema + "://" + resto;
}

/// Tudo que depende do MAVSDK vive aqui dentro.
class Drone::Impl {
public:
  Impl()
      : nucleo(mavsdk::Mavsdk::Configuration{
            mavsdk::ComponentType::GroundStation}) {}

  mavsdk::Mavsdk nucleo;
  std::shared_ptr<mavsdk::System> sistema;
  std::unique_ptr<mavsdk::Telemetry> telemetria;
  std::unique_ptr<mavsdk::Action> acoes;
};

Drone::Drone(std::string nome, std::string endereco)
    : Veiculo(std::move(nome)), impl_(std::make_unique<Impl>()),
      endereco_(std::move(endereco)) {}

Drone::~Drone() = default; // precisa estar no .cpp por causa do pimpl

void Drone::conectar() {
  const std::string url = normalizar_endereco(endereco_);
  std::cout << "[" << nome() << "] conectando em " << url << " ..."
            << std::endl;

  const mavsdk::ConnectionResult resultado =
      impl_->nucleo.add_any_connection(url);
  if (resultado != mavsdk::ConnectionResult::Success) {
    throw ErroDeVeiculo("não foi possível abrir " + url + ": " +
                        para_texto(resultado));
  }

  // O primeiro autopiloto que aparecer na rede MAVLink é o nosso drone.
  const auto sistema = impl_->nucleo.first_autopilot(TEMPO_LIMITE_S);
  if (!sistema) {
    throw ErroDeVeiculo("nenhum autopiloto respondeu em " + url +
                        ". O SITL do ArduPilot está rodando?");
  }

  impl_->sistema = sistema.value();
  impl_->telemetria = std::make_unique<mavsdk::Telemetry>(impl_->sistema);
  impl_->acoes = std::make_unique<mavsdk::Action>(impl_->sistema);
  std::cout << "[" << nome() << "] conectado." << std::endl;
}

void Drone::iniciar_telemetria() {
  if (!impl_->telemetria) {
    throw ErroDeVeiculo("iniciar_telemetria() antes de conectar()");
  }
  mavsdk::Telemetry& telemetria = *impl_->telemetria;

  // Cada assinatura é um callback chamado pela thread do MAVSDK. Tudo que
  // fazemos é traduzir o dado da biblioteca para o NOSSO modelo — assim o
  // resto do programa não depende do MAVSDK.
  telemetria.subscribe_flight_mode([this](mavsdk::Telemetry::FlightMode modo) {
    const std::string texto = para_texto(modo);
    atualizar([&texto](EstadoVeiculo& estado) { estado.modo_de_voo = texto; });
  });

  telemetria.subscribe_attitude_euler(
      [this](mavsdk::Telemetry::EulerAngle angulo) {
        atualizar([&angulo](EstadoVeiculo& estado) {
          estado.atitude.rolagem = angulo.roll_deg;
          estado.atitude.arfagem = angulo.pitch_deg;
          estado.atitude.guinada = angulo.yaw_deg;
        });
      });

  telemetria.subscribe_position([this](mavsdk::Telemetry::Position posicao) {
    atualizar([&posicao](EstadoVeiculo& estado) {
      estado.posicao.latitude = posicao.latitude_deg;
      estado.posicao.longitude = posicao.longitude_deg;
      estado.posicao.altitude_absoluta_m = posicao.absolute_altitude_m;
      estado.posicao.altitude_relativa_m = posicao.relative_altitude_m;
    });
  });

  telemetria.subscribe_battery([this](mavsdk::Telemetry::Battery bateria) {
    atualizar([&bateria](EstadoVeiculo& estado) {
      estado.bateria.tensao_v = bateria.voltage_v;
      // No MAVSDK 4.x remaining_percent já vem de 0 a 100 (nas versões
      // 1.x vinha de 0 a 1 — cuidado ao trocar de versão).
      estado.bateria.percentual = bateria.remaining_percent;
    });
  });

  telemetria.subscribe_armed([this](bool armado) {
    atualizar([armado](EstadoVeiculo& estado) { estado.armado = armado; });
  });

  telemetria.subscribe_in_air([this](bool em_voo) {
    atualizar([em_voo](EstadoVeiculo& estado) { estado.em_voo = em_voo; });
  });

  // TODO (aluno): assine o que mais a sua GCS precisar. A lista completa está
  // em <mavsdk/plugins/telemetry/telemetry.hpp> — por exemplo:
  //   subscribe_velocity_ned, subscribe_gps_info, subscribe_home,
  //   subscribe_heading, subscribe_rc_status, subscribe_health.
  // Se a taxa de atualização vier baixa, dê uma olhada nos métodos
  // set_rate_* do plugin Telemetry (o ArduPilot não implementa todos).
}

void Drone::alterar_modo_de_voo(const std::string& modo) {
  if (!impl_->acoes) {
    throw ErroDeVeiculo("alterar_modo_de_voo() antes de conectar()");
  }
  mavsdk::Action& acoes = *impl_->acoes;

  // O MAVSDK não expõe "troque para o modo X" de forma genérica: ele oferece
  // ações de alto nível. Mapeamos três delas como amostra.
  //
  // TODO (aluno): complete o mapa (GUIDED, AUTO, LOITER, STABILIZE...). Para
  // os modos sem ação equivalente, use o plugin MavlinkPassthrough e envie
  // MAV_CMD_DO_SET_MODE com o número do modo do ArduPilot.
  const std::map<std::string, std::function<mavsdk::Action::Result()>>
      acoes_por_modo{
          {"RTL", [&acoes] { return acoes.return_to_launch(); }},
          {"HOLD", [&acoes] { return acoes.hold(); }},
          {"LAND", [&acoes] { return acoes.land(); }},
  };

  const std::string alvo = para_maiusculas(modo);
  const auto encontrado = acoes_por_modo.find(alvo);
  if (encontrado == acoes_por_modo.end()) {
    std::string disponiveis;
    for (const auto& par : acoes_por_modo) {
      disponiveis += (disponiveis.empty() ? "" : ", ") + par.first;
    }
    throw NaoImplementado("modo '" + alvo + "'; disponíveis: " + disponiveis);
  }

  const mavsdk::Action::Result resultado = encontrado->second();
  if (resultado != mavsdk::Action::Result::Success) {
    throw ErroDeVeiculo("o autopiloto recusou o modo " + alvo + ": " +
                        para_texto(resultado));
  }
}

// TODO (aluno): implemente aqui os demais comandos, sempre delegando para o
// plugin Action e convertendo o Result em exceção, como acima:
//
//     void Drone::armar() {
//         const auto resultado = impl_->acoes->arm();
//         if (resultado != mavsdk::Action::Result::Success) { ... }
//     }
//
//     void Drone::decolar(double altura_m) {
//         impl_->acoes->set_takeoff_altitude(static_cast<float>(altura_m));
//         const auto resultado = impl_->acoes->takeoff();
//         ...
//     }

} // namespace gcs
