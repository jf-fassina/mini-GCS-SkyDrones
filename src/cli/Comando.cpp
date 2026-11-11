#include "cli/Comando.hpp"

#include <iomanip>
#include <sstream>
#include <utility>

namespace gcs {

std::string Comando::linha_de_ajuda() const {
    std::ostringstream fluxo;
    fluxo << std::left << std::setw(22) << uso() << ' ' << ajuda();
    return fluxo.str();
}

void RegistroDeComandos::registrar(std::unique_ptr<Comando> comando) {
    const std::string nome = comando->nome();
    comandos_[nome] = std::move(comando);
}

Comando& RegistroDeComandos::obter(const std::string& nome) {
    const auto encontrado = comandos_.find(nome);
    if (encontrado == comandos_.end()) {
        throw ErroDeComando("comando desconhecido: '" + nome + "'. Digite 'ajuda'.");
    }
    return *encontrado->second;
}

std::vector<const Comando*> RegistroDeComandos::todos() const {
    std::vector<const Comando*> resultado;
    resultado.reserve(comandos_.size());
    for (const auto& par : comandos_) {  // std::map já itera ordenado pelo nome
        resultado.push_back(par.second.get());
    }
    return resultado;
}

}  // namespace gcs
