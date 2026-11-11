#include "cli/Console.hpp"

#include <iostream>
#include <sstream>
#include <vector>

#include "EstacaoDeControle.hpp"
#include "cli/Comando.hpp"
#include "modelo/Veiculo.hpp"

namespace gcs {
namespace {
constexpr const char* PROMPT = "gcs> ";
}

Console::Console(EstacaoDeControle& estacao) : estacao_(estacao) {}

void Console::executar() {
  mostrar_abertura();
  std::string linha;
  while (estacao_.rodando()) {
    std::cout << PROMPT << std::flush;
    if (!std::getline(std::cin, linha)) { // Ctrl+D
      std::cout << '\n';
      estacao_.encerrar();
      break;
    }
    tratar(linha);
  }
}

void Console::mostrar_abertura() const {
  std::cout
      << "\n=== mini GCS — esqueleto da disciplina de OO (Skydrones) ===\n"
      << "veículo: " << estacao_.veiculo().nome()
      << "   digite 'ajuda' para começar\n\n";
}

void Console::tratar(const std::string& linha) {
  std::istringstream fluxo(linha);
  std::vector<std::string> palavras;
  std::string palavra;
  while (fluxo >> palavra) {
    palavras.push_back(palavra);
  }
  if (palavras.empty()) {
    return;
  }
  const std::string nome = palavras.front();
  const std::vector<std::string> argumentos(palavras.begin() + 1,
                                            palavras.end());

  // O console nunca morre por causa de um comando: traduzimos cada tipo de
  // problema em uma mensagem e voltamos para o prompt.
  try {
    estacao_.comandos().obter(nome).executar(estacao_, argumentos);
  } catch (const ErroDeComando& erro) {
    std::cout << "erro: " << erro.what() << '\n';
  } catch (const NaoImplementado& erro) {
    std::cout << "ainda não implementado (" << erro.what()
              << ") — este é um exercício da disciplina.\n";
  } catch (const std::exception& erro) {
    std::cout << "falha ao executar '" << nome << "': " << erro.what() << '\n';
  }
}

} // namespace gcs
