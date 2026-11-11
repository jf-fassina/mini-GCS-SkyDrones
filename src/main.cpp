// Ponto de entrada da mini ground control station.
//
//   ./mini_gcs                                 conecta em tcp:127.0.0.1:5760
//   ./mini_gcs --endereco udpin://:14550       outro enlace
//   ./mini_gcs --simulado                      telemetria falsa, sem SITL
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "EstacaoDeControle.hpp"
#include "cli/ComandosPadrao.hpp"
#include "modelo/Drone.hpp"
#include "modelo/Veiculo.hpp"

namespace {

void mostrar_uso(const std::string& programa) {
  std::cout << "uso: " << programa << " [--endereco <url>] [--simulado]\n"
            << "  --endereco <url>  enlace MAVLink (padrão: "
            << gcs::Drone::ENDERECO_PADRAO << ")\n"
            << "  --simulado        usa telemetria sintética, sem o SITL\n"
            << "  --ajuda           mostra esta mensagem\n";
}

} // namespace

int main(int argc, char** argv) {
  const std::vector<std::string> argumentos(argv + 1, argv + argc);
  std::string endereco = gcs::Drone::ENDERECO_PADRAO;
  bool simulado = false;

  for (std::size_t i = 0; i < argumentos.size(); ++i) {
    const std::string& argumento = argumentos[i];
    if (argumento == "--simulado") {
      simulado = true;
    } else if (argumento == "--endereco" && i + 1 < argumentos.size()) {
      endereco = argumentos[++i];
    } else if (argumento == "--ajuda" || argumento == "-h") {
      mostrar_uso(argv[0]);
      return 0;
    } else {
      std::cerr << "argumento desconhecido: " << argumento << "\n\n";
      mostrar_uso(argv[0]);
      return 2;
    }
  }

  try {
    // Aqui — e só aqui — decidimos com qual implementação de Veiculo vamos
    // trabalhar. O resto do programa não faz ideia da diferença.
    std::unique_ptr<gcs::Veiculo> veiculo;
    if (simulado) {
      veiculo = std::make_unique<gcs::VeiculoSimulado>();
    } else {
      veiculo = std::make_unique<gcs::Drone>("drone", endereco);
    }

    gcs::EstacaoDeControle estacao(std::move(veiculo), gcs::comandos_padrao());
    estacao.executar();
  } catch (const std::exception& erro) {
    std::cerr << "erro fatal: " << erro.what() << '\n';
    return 1;
  }
  return 0;
}
