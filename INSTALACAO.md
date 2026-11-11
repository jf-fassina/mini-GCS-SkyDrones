# Instalação, compilação e execução

Guia do zero até ver o drone na tela. Testado em **Ubuntu 24.04** (inclusive sob
WSL2). No macOS, troque `apt` por `brew`; no Windows, use WSL2 — é o caminho
menos dolorido, principalmente por causa do SITL.

Índice:

1. [Requisitos](#1-requisitos)
2. [Instalar o MAVSDK (C++)](#2-instalar-o-mavsdk-c)
3. [Instalar o ArduPilot e o SITL](#3-instalar-o-ardupilot-e-o-sitl)
4. [Compilar a mini GCS](#4-compilar-a-mini-gcs)
5. [Executar](#5-executar)
6. [Problemas comuns](#6-problemas-comuns)

---

## 1. Requisitos

| Item | Versão | Para quê |
|---|---|---|
| Compilador C++ | C++17 (g++ ≥ 9, clang ≥ 10) | compilar a GCS |
| CMake | ≥ 3.16 | build |
| MAVSDK C++ | 4.x (testado na 4.0.4) | falar MAVLink com o drone |
| Git + Python 3 | qualquer recente | clonar e rodar o SITL |
| ArduPilot | branch `Copter-4.5` ou `master` | o simulador |

Pacotes básicos de build:

```bash
sudo apt update
sudo apt install -y build-essential cmake git python3 python3-pip
```

---

## 2. Instalar o MAVSDK (C++)

O MAVSDK **não** vem nos repositórios do Ubuntu. Use o `.deb` oficial.

### 2.1 Instalação no sistema (recomendada)

Baixe o pacote da **sua** distribuição em
<https://github.com/mavlink/MAVSDK/releases> (`libmavsdk-dev_<versão>_<distro>_<arch>.deb`):

```bash
cd /tmp
VERSAO=4.0.4
DISTRO=ubuntu24.04        # ou ubuntu22.04, debian12, debian13...
wget "https://github.com/mavlink/MAVSDK/releases/download/v${VERSAO}/libmavsdk-dev_${VERSAO}_${DISTRO}_amd64.deb"
sudo dpkg -i "libmavsdk-dev_${VERSAO}_${DISTRO}_amd64.deb"
```

Confira:

```bash
ls /usr/include/mavsdk/mavsdk.hpp      # cabeçalhos
ls /usr/lib/cmake/MAVSDK               # pacote do CMake
```

Com isso o `find_package(MAVSDK REQUIRED)` do nosso `CMakeLists.txt` acha tudo
sozinho — não precisa configurar mais nada.

### 2.2 Instalação sem `sudo` (laboratório, máquina compartilhada)

Dá para instalar só para o seu usuário, extraindo o `.deb` em `~/.local`:

```bash
cd /tmp
dpkg -x libmavsdk-dev_4.0.4_ubuntu24.04_amd64.deb /tmp/mavsdk-pkg
mkdir -p ~/.local
cp -a /tmp/mavsdk-pkg/usr/include ~/.local/
cp -a /tmp/mavsdk-pkg/usr/lib     ~/.local/
ldconfig -n ~/.local/lib                                   # cria o link libmavsdk.so.4
ln -sf libmavsdk.so.4.0.4 ~/.local/lib/libmavsdk.so        # link de desenvolvimento
```

Confira:

```bash
ls ~/.local/include/mavsdk/mavsdk.hpp
ls ~/.local/lib/cmake/MAVSDK
```

O `CMakeLists.txt` já procura em `~/.local` e grava o caminho da biblioteca no
executável (RPATH), então nem `LD_LIBRARY_PATH` é necessário.

### 2.3 Compilando do código-fonte (só se precisar)

```bash
sudo apt install -y libcurl4-openssl-dev libtinyxml2-dev
git clone https://github.com/mavlink/MAVSDK.git --recursive
cd MAVSDK
cmake -S . -B build/default -DCMAKE_BUILD_TYPE=Release -DSUPERBUILD=ON
cmake --build build/default -j$(nproc)
sudo cmake --install build/default
```

Leva uns 15–30 min. Só vale a pena se você precisar de um patch que não está em
release.

---

## 3. Instalar o ArduPilot e o SITL

O SITL (*Software In The Loop*) é o ArduPilot de verdade compilado para rodar no
PC. É nele que vocês vão voar.

### 3.1 Clonar o repositório oficial

```bash
cd ~
git clone --recurse-submodules https://github.com/ArduPilot/ardupilot.git
cd ardupilot
```

Se esquecer o `--recurse-submodules` (acontece com todos):

```bash
git submodule update --init --recursive
```

Opcionalmente, fixe uma versão estável em vez do `master`:

```bash
git checkout Copter-4.5
git submodule update --init --recursive
```

### 3.2 Instalar as dependências

O próprio repositório traz o script que instala tudo (compilador ARM, `waf`,
MAVProxy, bibliotecas Python):

```bash
./Tools/environment_install/install-prereqs-ubuntu.sh -y
```

O script altera o seu `~/.profile` (adiciona as ferramentas ao `PATH`). Recarregue:

```bash
source ~/.profile
```

Feche e abra o terminal se `sim_vehicle.py` continuar não encontrado. Ele mora em
`<ardupilot>/Tools/autotest/sim_vehicle.py` — sempre dá para chamar pelo caminho
completo.

### 3.3 Primeira execução (compila o SITL)

```bash
cd ~/ardupilot/ArduCopter
sim_vehicle.py -w --console --map
```

- `-w` limpa os parâmetros gravados: use **só na primeira vez** (ou quando o
  drone ficar em um estado estranho).
- A primeira execução compila o ArduCopter com o `waf` e demora alguns minutos.
- `--console` abre o painel de status do MAVProxy; `--map`, o mapa. Os dois
  precisam de ambiente gráfico (no WSL2, o WSLg já resolve). Sem display, omita.

Nas vezes seguintes basta:

```bash
cd ~/ardupilot/ArduCopter
sim_vehicle.py --console --map
```

### 3.4 Qual endereço a GCS deve usar

Esse é o detalhe que mais consome tempo de laboratório. **O SITL abre um servidor
TCP em `127.0.0.1:5760` que aceita um cliente só.** Quem pega essa porta é o
MAVProxy, quando ele está no ar.

| Como você subiu o SITL | Endereço para a mini GCS |
|---|---|
| `sim_vehicle.py --no-mavproxy` | `tcp:127.0.0.1:5760` (nosso padrão) |
| `sim_vehicle.py` (com MAVProxy) | peça uma saída extra e conecte nela — veja abaixo |

Com MAVProxy no ar, suba o SITL pedindo um canal dedicado para a nossa GCS:

```bash
sim_vehicle.py -v ArduCopter --console --map --out=udpout:127.0.0.1:14552
```

e conecte com:

```bash
./build/mini_gcs --endereco udpin://:14552
```

Assim o MAVProxy, o Mission Planner (`udp:14550`) e a sua GCS convivem no mesmo
simulador.

### 3.5 Fazer o drone voar (para o gráfico ter o que mostrar)

No **console do MAVProxy**, com o SITL rodando:

```
mode guided
arm throttle
takeoff 20
```

Espere o `GPS: OK` / `EKF` estabilizar antes de armar. Depois experimente
`mode circle` para a atitude variar e o horizonte artificial ganhar vida.

---

## 4. Compilar a mini GCS

```bash
git clone <url-do-seu-fork> pucrs-gcs      # ou use o diretório que já tem
cd pucrs-gcs
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

O executável fica em `build/mini_gcs`.

Recompilar depois de editar um arquivo:

```bash
cmake --build build -j$(nproc)
```

Começar de novo do zero (quando mudar o CMakeLists ou trocar de MAVSDK):

```bash
rm -rf build && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)
```

MAVSDK em um prefixo fora do padrão (nem `/usr`, nem `~/.local`):

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=/caminho/do/prefixo
```

Para desenvolver, compile em `Debug` — os avisos e o depurador ficam úteis:

```bash
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug && cmake --build build-debug -j$(nproc)
```

O `CMakeLists.txt` também gera `build/compile_commands.json`; aponte o seu editor
(clangd, VS Code) para esse arquivo e você ganha autocompletar do MAVSDK.

---

## 5. Executar

Com o SITL rodando em outro terminal:

```bash
./build/mini_gcs                              # tcp:127.0.0.1:5760 (padrão)
./build/mini_gcs --endereco udpin://:14552    # canal dedicado (veja 3.4)
./build/mini_gcs --simulado                   # telemetria falsa, sem SITL
./build/mini_gcs --ajuda
```

Sessão típica:

```
$ ./build/mini_gcs
[drone] conectando em tcpout://127.0.0.1:5760 ...
[drone] conectado.

=== mini GCS — esqueleto da disciplina de OO (Skydrones) ===
veículo: drone   digite 'ajuda' para começar

gcs> status
drone: modo=Stabilized  armado=nao em_voo=nao alt_rel=  0.0 m ...
gcs> plotar altitude      # gráfico ao vivo; Enter volta para o console
gcs> horizonte            # horizonte artificial; Enter volta
gcs> modo RTL
gcs> sair
```

Comandos: `ajuda`, `status`, `modo <NOME>`, `plotar <campo>`, `horizonte`,
`decolar <altura_m>` (exercício) e `sair`.

> **Dica:** rode `--simulado` para trabalhar na interface sem o SITL. Se a sua
> GCS só funcionar com o drone real, apareceu um acoplamento indevido.

---

## 6. Problemas comuns

**`Could not find a package configuration file provided by "MAVSDK"`**
O MAVSDK não está instalado, ou está em um prefixo que o CMake não procura.
Confira `ls /usr/lib/cmake/MAVSDK` e, se instalou em outro lugar, passe
`-DCMAKE_PREFIX_PATH=<prefixo>`.

**`fatal error: plugin_base.hpp: No such file or directory`**
Você está compilando "na mão", sem o CMake. Os cabeçalhos dos plugins exigem
**dois** diretórios de include: `-I<prefixo>/include -I<prefixo>/include/mavsdk`.
O target `MAVSDK::mavsdk` já faz isso — use o CMake.

**`error while loading shared libraries: libmavsdk.so.4`**
A biblioteca não está no caminho do carregador. Com instalação no sistema, rode
`sudo ldconfig`. Com instalação em `~/.local`, recompile do zero (o CMakeLists
grava o RPATH) ou exporte `LD_LIBRARY_PATH=$HOME/.local/lib`.

**`nenhum autopiloto respondeu em tcpout://127.0.0.1:5760`**
O SITL não está rodando, ou o MAVProxy já ocupou a porta 5760. Veja a seção 3.4.

**O gráfico fica em 0,0 m e o horizonte nivelado**
Está tudo certo: o drone está parado no chão. Faça ele voar (seção 3.5).

**O desenho sai sem cor ou todo quebrado**
O terminal precisa entender ANSI *truecolor*. Use o Windows Terminal, o
GNOME Terminal, o iTerm2 — não o `cmd.exe` clássico.

**`sim_vehicle.py: command not found`**
Faltou `source ~/.profile` depois do `install-prereqs-ubuntu.sh`, ou o terminal
é anterior à instalação. Como alternativa, chame pelo caminho completo:
`~/ardupilot/Tools/autotest/sim_vehicle.py -v ArduCopter --console`.
