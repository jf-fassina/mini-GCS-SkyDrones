#include "cli/ComandosPadrao.hpp"

#include <iostream>
#include <memory>

#include "EstacaoDeControle.hpp"
#include "modelo/Veiculo.hpp"
#include "visual/GraficoTempo.hpp"
#include "visual/HorizonteArtificial.hpp"

namespace gcs {
namespace {

// ---------------------------------------------------------------------------
// Comandos de exemplo. Cada um é uma classe pequena; o console não sabe quais
// existem, só pede ao registro o comando com o nome digitado.
// ---------------------------------------------------------------------------

class Ajuda : public Comando {
public:
  std::string nome() const override { return "ajuda"; }
  std::string ajuda() const override { return "lista os comandos disponíveis"; }

  void executar(EstacaoDeControle& estacao,
                const std::vector<std::string>&) override {
    std::cout << "Comandos:\n";
    for (const Comando* comando : estacao.comandos().todos()) {
      std::cout << "  " << comando->linha_de_ajuda() << '\n';
    }
  }
};

class Status : public Comando {
public:
  std::string nome() const override { return "status"; }
  std::string ajuda() const override {
    return "mostra o modo de voo e a telemetria atual";
  }

  void executar(EstacaoDeControle& estacao,
                const std::vector<std::string>&) override {
    std::cout << estacao.veiculo() << '\n';
  }
};

class Modo : public Comando {
public:
  std::string nome() const override { return "modo"; }
  std::string uso() const override { return "modo <NOME>"; }
  std::string ajuda() const override {
    return "troca o modo de voo (ex.: modo RTL)";
  }

  void executar(EstacaoDeControle& estacao,
                const std::vector<std::string>& argumentos) override {
    if (argumentos.size() != 1) {
      throw ErroDeComando("uso: " + uso());
    }
    estacao.veiculo().alterar_modo_de_voo(argumentos.front());
    std::cout << "pedido de troca de modo enviado\n";
  }
};

class Plotar : public Comando {
public:
  std::string nome() const override { return "plotar"; }
  std::string uso() const override { return "plotar <campo>"; }
  std::string ajuda() const override {
    return "abre o gráfico de um campo: " + nomes_dos_campos();
  }

  void executar(EstacaoDeControle& estacao,
                const std::vector<std::string>& argumentos) override {
    if (argumentos.size() != 1) {
      throw ErroDeComando("uso: " + uso());
    }
    const auto& catalogo = campos_disponiveis();
    const auto encontrado = catalogo.find(argumentos.front());
    if (encontrado == catalogo.end()) {
      throw ErroDeComando("campo '" + argumentos.front() +
                          "' não existe. Disponíveis: " + nomes_dos_campos());
    }
    const GraficoTempo grafico{encontrado->second};
    estacao.exibir_painel(grafico);
  }
};

class Horizonte : public Comando {
public:
  std::string nome() const override { return "horizonte"; }
  std::string ajuda() const override { return "abre o horizonte artificial"; }

  void executar(EstacaoDeControle& estacao,
                const std::vector<std::string>&) override {
    const HorizonteArtificial horizonte;
    estacao.exibir_painel(horizonte);
  }
};

/// Exemplo de comando deixado como exercício: o console trata NaoImplementado
/// de forma amigável, então dá para registrar o esqueleto antes de implementar.
class Decolar : public Comando {
public:
  std::string nome() const override { return "decolar"; }
  std::string uso() const override { return "decolar <altura_m>"; }
  std::string ajuda() const override {
    return "[exercício] arma o drone e decola até a altura pedida";
  }

  void executar(EstacaoDeControle&, const std::vector<std::string>&) override {
    // TODO (aluno): validar a altura recebida, chamar veiculo().armar() e
    // veiculo().decolar(altura) — métodos que você vai declarar em Veiculo e
    // implementar em Drone — e avisar o usuário do resultado.
    throw NaoImplementado("decolagem");
  }
};

class Sair : public Comando {
public:
  std::string nome() const override { return "sair"; }
  std::string ajuda() const override { return "encerra a estação de controle"; }

  void executar(EstacaoDeControle& estacao,
                const std::vector<std::string>&) override {
    estacao.encerrar();
  }
};

} // namespace

RegistroDeComandos comandos_padrao() {
  RegistroDeComandos registro;
  registro.registrar(std::make_unique<Ajuda>());
  registro.registrar(std::make_unique<Status>());
  registro.registrar(std::make_unique<Modo>());
  registro.registrar(std::make_unique<Plotar>());
  registro.registrar(std::make_unique<Horizonte>());
  registro.registrar(std::make_unique<Decolar>());
  registro.registrar(std::make_unique<Sair>());
  return registro;
}

} // namespace gcs
