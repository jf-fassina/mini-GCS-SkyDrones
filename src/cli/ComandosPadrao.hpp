// Fábrica dos comandos que já vêm no esqueleto.
#pragma once

#include "cli/Comando.hpp"

namespace gcs {

/// Monta o registro com os comandos de exemplo.
///
/// TODO (aluno): registre aqui os seus comandos novos — `armar`, `decolar`,
/// `pousar`, `rtl`, `missao`, `gravar`, `limpar`... As classes ficam em
/// ComandosPadrao.cpp; copie uma delas como modelo.
RegistroDeComandos comandos_padrao();

} // namespace gcs
