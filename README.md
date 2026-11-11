# mini GCS — esqueleto do trabalho de Orientação a Objetos

Parceria **PUCRS × Skydrones**. Este repositório é o **ponto de partida** do
trabalho da disciplina: uma *ground control station* mínima, em C++, que se
conecta a um drone ArduPilot via MAVSDK, mostra o modo de voo e desenha dados de
telemetria (gráfico e horizonte artificial) em um console interativo.

O código aqui **não é a resposta do trabalho** — é o andaime. Ele resolve a parte
chata (conexão, laço de eventos, threads, desenho) e deixa marcado com
`TODO (aluno)` tudo o que vocês devem implementar. O objetivo da disciplina é
**modelar os dados do drone em objetos**, com um desenho consistente e boas
práticas de OO, não decorar a API do MAVSDK.

---

## 1. Instalação e primeiros passos

O passo a passo completo — instalar o MAVSDK, clonar o ArduPilot, subir o SITL e
compilar — está em **[INSTALACAO.md](INSTALACAO.md)**. O resumo:

| Ferramenta | Versão | Observação |
|---|---|---|
| Compilador C++ | C++17 | g++ ≥ 9 ou clang ≥ 10 |
| CMake | ≥ 3.16 | |
| MAVSDK C++ | 4.x | testado com a 4.0.4 |
| ArduPilot SITL | `Copter-4.5` ou `master` | o simulador |

```bash
# 1. MAVSDK (pacote oficial da sua distro — veja INSTALACAO.md §2)
sudo dpkg -i libmavsdk-dev_4.0.4_ubuntu24.04_amd64.deb

# 2. SITL, em outro terminal
cd ~/ardupilot/ArduCopter && sim_vehicle.py --console --map

# 3. a GCS
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
./build/mini_gcs
```

O SITL abre um servidor TCP em `127.0.0.1:5760` e aceita **um** cliente. Se o
MAVProxy estiver no ar, ele fica com essa porta: suba o simulador com
`--out=udpout:127.0.0.1:14552` e conecte com
`./build/mini_gcs --endereco udpin://:14552` (detalhes em INSTALACAO.md §3.4).

---

## 2. Executar

```bash
./build/mini_gcs                      # conecta em tcp:127.0.0.1:5760
./build/mini_gcs --endereco udpin://:14552
./build/mini_gcs --simulado           # telemetria falsa, sem precisar do SITL
```

> **Dica:** `--simulado` usa a classe `VeiculoSimulado` e permite trabalhar na
> interface sem o simulador ligado. Se a sua GCS só funciona com o SITL, algum
> acoplamento indevido apareceu — a estação deveria ser indiferente a isso.

### Comandos do console

```
ajuda                  lista os comandos disponíveis
status                 mostra o modo de voo e a telemetria atual
modo <NOME>            troca o modo de voo (ex.: modo RTL)
plotar <campo>         abre o gráfico de um campo: altitude, bateria
horizonte              abre o horizonte artificial
decolar <altura_m>     [exercício] arma o drone e decola
sair                   encerra a estação de controle
```

Nos painéis (`plotar`, `horizonte`) a tela é redesenhada ao vivo; **Enter** volta
para o prompt.

---

## 3. Arquitetura

```
src/
├── main.cpp                       escolhe a implementação de Veiculo e sai de cena
├── EstacaoDeControle.{hpp,cpp}    junta veículo + histórico + comandos + painéis
├── modelo/                        OS DADOS DO DRONE  ← o coração do trabalho
│   ├── Telemetria.hpp             Atitude, Posicao, Bateria, EstadoVeiculo
│   ├── Veiculo.{hpp,cpp}          classe base abstrata (+ VeiculoSimulado)
│   ├── Drone.{hpp,cpp}            implementação com MAVSDK (ArduPilot)
│   └── Historico.{hpp,cpp}        buffer circular de estados; alimenta os gráficos
├── cli/
│   ├── Comando.{hpp,cpp}          padrão Command + registro de comandos
│   ├── ComandosPadrao.{hpp,cpp}   os comandos de exemplo
│   └── Console.{hpp,cpp}          laço ler → interpretar → executar
└── visual/
    ├── Terminal.hpp               auxiliares ANSI (cores, cursor)
    ├── Painel.{hpp,cpp}           interface Painel + Renderizador
    ├── GraficoTempo.{hpp,cpp}     série temporal de qualquer grandeza
    └── HorizonteArtificial.{hpp,cpp}  attitude indicator
```

Como os dados caminham:

```
MAVSDK (thread da biblioteca)
   └─ Drone traduz o pacote para EstadoVeiculo
        └─ Veiculo::atualizar() notifica os observadores
             ├─ Historico  (guarda as últimas 2000 amostras)
             └─ (seu observador aqui: log em arquivo, alarme de bateria...)

thread do painel ── Renderizador ── Painel::quadro(historico) ── terminal
thread principal ── Console ── Comando::executar(estacao, argumentos)
```

### Decisões de projeto que valem ser notadas

- **`EstadoVeiculo` é um objeto de valor.** Cada atualização produz um estado
  completo e copiável, em vez de campos espalhados com setters. Copiar é barato e
  elimina a maior parte das dores de cabeça com threads.
- **`Veiculo` é abstrata e não conhece o MAVSDK.** Console, histórico e painéis
  dependem só dela — o que permite trocar `Drone` por `VeiculoSimulado` em uma
  linha do `main.cpp`. **Programe contra a abstração.**
- **O MAVSDK fica escondido atrás de um *pimpl*.** Nem o cabeçalho
  `Drone.hpp` inclui a biblioteca: só `Drone.cpp` a conhece.
- **Observer** liga o veículo a quem consome dados, sem que ele saiba quem são.
- **Command**: cada comando do console é uma classe. Não existe nenhum
  `if (palavra == "...")` gigante — e adicionar um comando não exige mexer no
  console.
- **O painel devolve o quadro como texto** em vez de escrever na tela. Dá para
  testar um painel com um histórico de mentira, e trocar o terminal por uma
  janela de verdade sem reescrever o desenho.
- **O gráfico recebe um `Campo`** (rótulo, unidade, função extratora) em vez de
  saber ler altitude "na mão": a mesma classe plota qualquer grandeza.

---

## 4. O que está pronto e o que é trabalho de vocês

**Pronto (os exemplos que servem de molde):** conexão com o SITL, modo de voo,
atitude, posição, bateria, `armado`/`em_voo`; histórico; comandos `ajuda`,
`status`, `modo`, `plotar`, `horizonte`, `sair`; um gráfico de série temporal;
um horizonte artificial básico.

**A fazer** — cada item tem um `TODO (aluno)` no lugar correspondente:

1. **Modelo** (`modelo/`)
   - ampliar `EstadoVeiculo` (velocidade, taxa de subida, satélites, rádio, home);
   - declarar em `Veiculo` e implementar em `Drone`: `armar`, `desarmar`,
     `decolar`, `pousar`, `retornar_para_casa`, `ir_para`;
   - completar o mapa de modos de voo (`GUIDED`, `AUTO`, `LOITER`...);
   - gerar telemetria sintética em `VeiculoSimulado` para testar sem o SITL.
2. **Comandos** (`cli/ComandosPadrao.cpp`): implementar `decolar` e criar os seus
   (`armar`, `pousar`, `rtl`, `gravar`, `limpar`...). Copie uma classe existente.
3. **Visualização** (`visual/`): novos campos no catálogo de `plotar`; escada de
   arfagem, arco de rolagem e fita de rumo no horizonte; ligar os pontos do
   gráfico; alarmes visuais (bateria baixa, rolagem excessiva).
4. **Dados**: exportar o histórico para CSV/JSON e analisar o voo depois.
5. **Desafio — interface gráfica.** O desenho em terminal é intencionalmente o
   mínimo. Troque-o por uma janela de verdade: a maneira mais curta é
   [Dear ImGui](https://github.com/ocornut/imgui) + [ImPlot](https://github.com/epezent/implot)
   com GLFW, trazidos pelo CMake via `FetchContent`. Qt e SFML também valem.
   **A costura já existe:** implemente `Painel` (ou um `Renderizador`
   equivalente) com o backend novo e nada no modelo precisa mudar. Se precisar
   mudar, o acoplamento está errado — é exatamente isso que estamos avaliando.
6. **Qualidade**: testes de unidade para `Historico` e para os painéis (eles são
   funções puras do histórico, de propósito), e um `const` no lugar certo.

### Critérios de avaliação

Separação de responsabilidades · uso correto de abstração, herança e
polimorfismo (sem herança decorativa) · encapsulamento · nomes e consistência ·
tratamento de erros (exceções, não códigos de retorno espalhados) · a GCS
continuar funcionando com `--simulado`.

---

## 5. Avisos de quem já tropeçou

- **MAVSDK × ArduPilot:** o MAVSDK nasceu no PX4 e não cobre 100% do ArduPilot.
  Alguns modos não têm ação equivalente — para esses, use o plugin
  `MavlinkPassthrough` e envie `MAV_CMD_DO_SET_MODE`.
- **Versões:** este esqueleto usa a API da MAVSDK **4.x**. Na 1.x os
  `subscribe_*` não devolviam *handle*, os cabeçalhos terminavam em `.h` e
  `Battery::remaining_percent` ia de 0 a 1 (na 4.x vai de 0 a 100).
- **Callbacks vêm de outra thread.** Nunca escreva em um dado compartilhado de
  dentro de um callback do MAVSDK sem proteção — veja `Veiculo::atualizar`.
- **Cores no terminal:** o desenho usa ANSI *truecolor*. Em terminais antigos
  pode sair sem cor; o `cmd.exe` clássico não serve (use o Windows Terminal).
- **Segurança:** só voe no SITL. Comandos de armar e decolar em hardware real são
  feitos com o laboratório, com hélices removidas e supervisão.
