// Comandos do console — um exemplo do padrão de projeto *Command*.
#pragma once

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace gcs {

class EstacaoDeControle; // declarado à frente: o comando só precisa da
                         // referência

/// Uso incorreto de um comando. O console mostra a mensagem e segue em frente.
class ErroDeComando : public std::runtime_error {
public:
  explicit ErroDeComando(const std::string& mensagem)
      : std::runtime_error(mensagem) {}
};

/// Contrato de um comando do console.
///
/// Cada comando é uma classe: tem nome, texto de ajuda e sabe se executar. Para
/// criar um comando novo basta escrever a classe e registrá-la em
/// gcs::comandos_padrao() — nada mais no projeto precisa mudar. É o ganho
/// concreto de usar polimorfismo em vez de um `if/else` gigante lendo strings.
class Comando {
public:
  virtual ~Comando() = default;

  /// Palavra que o usuário digita.
  virtual std::string nome() const = 0;
  /// Uma linha explicando o comando (aparece no `ajuda`).
  virtual std::string ajuda() const = 0;
  /// Forma de uso; por padrão, só o nome.
  virtual std::string uso() const { return nome(); }

  /// Executa o comando. `argumentos` são as palavras digitadas após o nome.
  virtual void executar(EstacaoDeControle& estacao,
                        const std::vector<std::string>& argumentos) = 0;

  /// "modo <NOME>          troca o modo de voo" — usada pelo comando `ajuda`.
  std::string linha_de_ajuda() const;
};

/// Coleção de comandos indexada pelo nome.
class RegistroDeComandos {
public:
  void registrar(std::unique_ptr<Comando> comando);

  /// Comando com esse nome; lança ErroDeComando se não existir.
  Comando& obter(const std::string& nome);

  /// Todos os comandos, em ordem alfabética.
  std::vector<const Comando*> todos() const;

private:
  std::map<std::string, std::unique_ptr<Comando>> comandos_;
};

} // namespace gcs
