# À fazer

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
