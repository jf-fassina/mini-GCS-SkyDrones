#include "modelo/Historico.hpp"

namespace gcs {

Historico::Historico(std::size_t capacidade) : capacidade_(capacidade) {}

void Historico::registrar(const EstadoVeiculo& estado) {
    std::lock_guard<std::mutex> trava(mutex_);
    estados_.push_back(estado);
    while (estados_.size() > capacidade_) {
        estados_.pop_front();
    }
}

std::size_t Historico::tamanho() const {
    std::lock_guard<std::mutex> trava(mutex_);
    return estados_.size();
}

bool Historico::vazio() const {
    std::lock_guard<std::mutex> trava(mutex_);
    return estados_.empty();
}

EstadoVeiculo Historico::ultimo(bool* ok) const {
    std::lock_guard<std::mutex> trava(mutex_);
    if (estados_.empty()) {
        if (ok != nullptr) {
            *ok = false;
        }
        return EstadoVeiculo{};
    }
    if (ok != nullptr) {
        *ok = true;
    }
    return estados_.back();
}

Historico::Serie Historico::serie(const Extrator& extrator) const {
    std::lock_guard<std::mutex> trava(mutex_);
    Serie resultado;
    if (estados_.empty()) {
        return resultado;
    }
    const Instante t0 = estados_.front().instante;
    resultado.segundos.reserve(estados_.size());
    resultado.valores.reserve(estados_.size());
    for (const EstadoVeiculo& estado : estados_) {
        resultado.segundos.push_back(segundos_entre(t0, estado.instante));
        resultado.valores.push_back(extrator(estado));
    }
    return resultado;
}

void Historico::limpar() {
    std::lock_guard<std::mutex> trava(mutex_);
    estados_.clear();
}

}  // namespace gcs
