cmake_minimum_required(VERSION 3.16)
project(mini_gcs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # ajuda o clangd/VS Code a entender o projeto

# MAVSDK (C++) — instalação explicada em INSTALACAO.md.
# Com o pacote .deb instalado no sistema, o find_package acha sozinho em
# /usr/lib/cmake/MAVSDK. Também procuramos em ~/.local, para quem instalou sem
# sudo; e dá para apontar outro prefixo com -DCMAKE_PREFIX_PATH=<caminho>.
list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/.local")
find_package(MAVSDK REQUIRED)
find_package(Threads REQUIRED)
message(STATUS "MAVSDK ${MAVSDK_VERSION} encontrado em ${MAVSDK_DIR}")

add_executable(mini_gcs
    src/main.cpp
    src/EstacaoDeControle.cpp
    src/modelo/Telemetria.cpp
    src/modelo/Veiculo.cpp
    src/modelo/Drone.cpp
    src/modelo/Historico.cpp
    src/cli/Comando.cpp
    src/cli/ComandosPadrao.cpp
    src/cli/Console.cpp
    src/visual/Painel.cpp
    src/visual/GraficoTempo.cpp
    src/visual/HorizonteArtificial.cpp
)

# Os #include do projeto são relativos a src/ (ex.: "modelo/Veiculo.hpp").
target_include_directories(mini_gcs PRIVATE src)

target_link_libraries(mini_gcs PRIVATE MAVSDK::mavsdk Threads::Threads)

target_compile_options(mini_gcs PRIVATE -Wall -Wextra -Wpedantic)

# Se o MAVSDK estiver fora dos diretórios padrão do sistema (p.ex. em ~/.local),
# gravamos o caminho da biblioteca no executável para que ele rode sem precisar
# de LD_LIBRARY_PATH.
get_filename_component(MAVSDK_LIB_DIR "${MAVSDK_DIR}/../.." ABSOLUTE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${MAVSDK_LIB_DIR}" _indice_sistema)
if(_indice_sistema EQUAL -1)
    set_target_properties(mini_gcs PROPERTIES BUILD_RPATH "${MAVSDK_LIB_DIR}")
endif()
