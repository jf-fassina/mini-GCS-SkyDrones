#include "visual/Painel.hpp"

#include <iostream>
#include <thread>

#include "visual/Terminal.hpp"

namespace gcs {

Renderizador::Renderizador(const Historico& historico, double quadros_por_segundo)
    : historico_(historico),
      intervalo_(std::chrono::milliseconds(
          static_cast<int>(1000.0 / (quadros_por_segundo > 0.0 ? quadros_por_segundo : 1.0)))) {}

void Renderizador::exibir_enquanto(const Painel& painel, const std::atomic<bool>& parar) const {
    std::cout << terminal::LIMPAR_TELA << terminal::ESCONDER_CURSOR;
    while (!parar) {
        // Volta ao canto e reescreve o quadro por cima do anterior.
        std::cout << terminal::INICIO << painel.quadro(historico_) << std::flush;
        std::this_thread::sleep_for(intervalo_);
    }
    std::cout << terminal::MOSTRAR_CURSOR << terminal::LIMPAR_TELA << std::flush;
}

}  // namespace gcs
