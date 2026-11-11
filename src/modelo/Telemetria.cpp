#include "modelo/Telemetria.hpp"

#include <iomanip>
#include <sstream>

namespace gcs {

std::string EstadoVeiculo::resumo() const {
    std::ostringstream fluxo;
    fluxo << std::fixed << std::setprecision(1)
          << "modo=" << std::left << std::setw(18) << modo_de_voo << std::right
          << " armado=" << (armado ? "sim" : "nao")
          << " em_voo=" << (em_voo ? "sim" : "nao")
          << " alt_rel=" << std::setw(7) << posicao.altitude_relativa_m << " m"
          << " rolagem=" << std::setw(6) << atitude.rolagem << " deg"
          << " arfagem=" << std::setw(6) << atitude.arfagem << " deg"
          << " bat=" << std::setw(5) << bateria.percentual << " %";
    return fluxo.str();
}

double segundos_entre(Instante inicio, Instante fim) {
    return std::chrono::duration<double>(fim - inicio).count();
}

}  // namespace gcs
