// Horizonte artificial (attitude indicator) desenhado no terminal.
#pragma once

#include <string>

#include "visual/Painel.hpp"

namespace gcs {

/// Mostra a atitude do veículo como um instrumento de voo.
///
/// A ideia é a mesma de um horizonte artificial de verdade: a aeronave fica
/// parada no centro da tela e o MUNDO se move. Para cada caractere da tela
/// sabemos a sua posição (x, y) em graus a partir do centro, e decidimos se ali
/// é céu ou chão com uma única conta:
///
///     u = -x * sen(rolagem) + y * cos(rolagem) + arfagem
///
/// `u` é a altura daquele ponto acima da linha do horizonte: u > 0 é céu, u < 0
/// é chão, u ~ 0 é a própria linha do horizonte. Girar a rolagem inclina a
/// linha; aumentar a arfagem (nariz para cima) empurra o horizonte para baixo,
/// mostrando mais céu — exatamente como na janela do cockpit.
class HorizonteArtificial : public Painel {
public:
    explicit HorizonteArtificial(int largura = 72, int altura = 21);

    std::string quadro(const Historico& historico) const override;

private:
    int largura_;
    int altura_;
};

}  // namespace gcs
