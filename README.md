# Simulador de ECU

Projeto final da disciplina **Programação e Desenvolvimento de Software 2 (PDS2)** — UFMG.

## Descrição do problema

Uma ECU (Engine Control Unit) é o computador embarcado responsável por gerenciar a propulsão de um veículo em tempo real: ela lê a telemetria do carro, decide como responder ao comando do piloto e monitora continuamente as condições de segurança elétrica e térmica que determinam se o veículo pode ou não seguir em tração.

Este projeto implementa em C++11, executando no terminal, um simulador da ECU de um **veículo elétrico**. A simulação é organizada em torno de:

- Uma **Máquina de Estados Finitos** para o veículo, com os estados **Idle**, **Drive** e **Fault**, aplicando o padrão *State* — cada estado decide se o pedal do acelerador é processado, se a troca de mapas é permitida e para quais estados a transição é aceita.
- Um **mapeador de torque** que converte a posição bruta do pedal em torque final, selecionando entre as tabelas de calibração **Eco**, **MidTerm** e **Sport** por interpolação linear, com rampa mecânica e teto de segurança.
- Um **monitor de segurança** que valida o *Shutdown System*, a tensão da bateria (falha se cair abaixo de 60 V), a temperatura de motor e bateria (alerta aos 60 °C, falha crítica aos 80 °C) e o *timeout* da sequência de pré-carga.
- Um **fluxo de telemetria** lido de um arquivo CSV (posição do pedal, temperaturas, pressão de freio, tensão) e um **logger** que grava a série temporal — pedal, torque, temperaturas, trocas de mapa e eventos críticos — em arquivo de texto para análise posterior.

## Objetivos

- Aplicar os fundamentos de POO (encapsulamento, herança, polimorfismo, tratamento de exceções) em um sistema de porte médio.
- Modelar a Máquina de Estados do veículo com uma classe base abstrata (`VehicleState`) e três estados concretos (`IdleState`, `DriveState`, `FaultState`).
- Estruturar uma hierarquia de exceções própria (`EcuException` e derivadas) para representar falhas específicas do simulador — subtensão, sobretemperatura, abertura do *Shutdown System*, *timeout* de pré-carga e erro de parse da telemetria.
- Praticar modularização, separação entre especificação (`.hpp`) e implementação (`.cpp`), e compilação incremental com `make`.
- Trabalhar com persistência em arquivos de texto (cenário de condução em CSV, log de telemetria em `.txt`).
- Produzir documentação técnica com Doxygen.
- Cobrir o comportamento das classes com testes automatizados (doctest).

## Árvore de diretórios

```
ecu-simulation-project/
├── include/            # Cabeçalhos .hpp — especificação (contrato) das classes
│   ├── core/           #   ECUController: orquestrador e Main Loop
│   ├── state/          #   VehicleState + IdleState/DriveState/FaultState
│   ├── control/        #   TorqueMapper (mapas) e TractionControl (TCS via PID)
│   ├── safety/         #   SafetyMonitor (Shutdown, subtensão, temperatura, pré-carga)
│   ├── sensors/        #   SignalFilter (Filtro Passa-Baixa / EMA)
│   ├── io/             #   SensorManager (CSV de entrada) e DataLogger (telemetria)
│   └── exceptions/     #   Hierarquia EcuException e derivadas
├── src/                # Implementações .cpp — espelham a estrutura de include/
│   ├── main.cpp        #   ponto de entrada
│   ├── core/
│   ├── state/
│   ├── control/
│   ├── safety/
│   ├── sensors/
│   ├── io/
│   └── exceptions/
├── tests/              # Testes com doctest (header único)
├── data/               # Dados de entrada e saída em texto
│   ├── maps/           #   tabelas de calibração
│   ├── scenarios/      #   perfis de condução
│   └── logs/           #   séries temporais em CSV geradas pela execução
├── design/             # Modelagem: user stories, cartões CRC, diagramas UML, decisões de arquitetura
├── build/              # Objetos, binários e saída do Doxygen em build/docs/ (conteúdo ignorado pelo Git)
├── Doxyfile
├── Makefile
├── README.md
└── .gitignore
```

## Compilação e Execução

O projeto é compilado com `g++` em C++11 estrito. O `Makefile` na raiz fornece os alvos:

| Alvo         | Descrição                                                                 |
|--------------|---------------------------------------------------------------------------|
| `make`       | Compila todos os `.cpp` de `src/` (e subpastas) em `build/`, mantendo a estrutura de diretórios, e gera o executável `build/ecu_simulator`. |
| `make run`   | Compila (se necessário) e executa `build/ecu_simulator`.                  |
| `make test`  | Compila os testes de `tests/` com os `.cpp` do projeto (exceto `main.cpp`) e executa `build/tests_runner`. |
| `make clean` | Remove tudo o que foi gerado em `build/`.                                 |
| `make docs`  | Gera a documentação Doxygen em `build/docs/`.                             |

As flags obrigatórias (`-std=c++11 -Wall -Wextra -I include`) já são definidas no `Makefile`, assim como `-MMD -MP` para dependências automáticas de headers.

No Linux, basta ter `g++` e `make` instalados. No Windows, use o MSYS2 (`pacman -S make mingw-w64-x86_64-gcc`) ou invoque `mingw32-make` dentro do MSYS2/MinGW.

## Funcionalidades

*Em construção.* As funcionalidades serão habilitadas progressivamente nos próximos checkpoints — o C7 fixa apenas o contrato das classes e os testes TDD; a lógica interna é implementada no C8.

## Testes

A suíte de testes utiliza [doctest](https://github.com/doctest/doctest) (header único em `tests/third_party/doctest.h`). Cada classe de domínio tem seu próprio arquivo de teste em `tests/`:

- `test_exceptions.cpp`      — hierarquia `EcuException` e derivadas.
- `test_states.cpp`          — `VehicleState`, `IdleState`, `DriveState`, `FaultState`.
- `test_sensor_manager.cpp`  — leitura de CSV, linhas inválidas, erro de abertura.
- `test_torque_mapper.cpp`   — interpolação, rampa, teto de segurança, modos.
- `test_safety_monitor.cpp`  — limites de 60 V, 60 °C, 80 °C e *timeout* de pré-carga.
- `test_data_logger.cpp`     — gravação de amostras, trocas de modo, alertas e erros.
- `test_ecu_controller.cpp`  — estado inicial, transição para `FaultState`, acesso aos colaboradores.

Para rodar:

```
make test
```

### Status atual — fase vermelha do TDD

Este checkpoint (C7) adota a disciplina de TDD: os testes foram escritos **antes** da implementação, a partir dos critérios de aceitação das user stories em `design/user_stories.md`. As classes de domínio em `src/` existem apenas como esqueletos (métodos retornam valores padrão ou nada fazem, marcados com `// TODO C8`).

Como consequência, hoje:

- Os testes de `test_exceptions.cpp` **passam** (as exceções são `inline` nos headers e já estão prontas).
- Os demais testes **falham** intencionalmente — isso é a fase vermelha do ciclo *red → green → refactor* e será resolvida no checkpoint C8, quando a lógica interna de cada classe for implementada.

## Documentação

Artefatos de modelagem — user stories, cartões CRC e diagrama de classes — ficam em `design/`.

A documentação de API é gerada a partir dos comentários Doxygen (`@brief`, `@param`, `@return`, `@throws`) presentes nos cabeçalhos `.hpp`. A saída fica em `build/docs/` e **não é versionada**.

Para gerar a documentação diretamente com o Doxygen:

```
doxygen Doxyfile
```

Uma vez que o `Makefile` esteja disponível, o alvo equivalente será:

```
make docs
```

O `Doxyfile` já vem configurado com `OUTPUT_DIRECTORY = build/docs`, `RECURSIVE = YES`, `USE_MDFILE_AS_MAINPAGE = README.md`, `OUTPUT_LANGUAGE = Brazilian` e `EXTRACT_PRIVATE = YES`.

## Integrantes

Miguel Almeida Faria,  Guilherme Teixera de Paula, Joao Victor Resende Fernandes, Guilherme Augusto de Brito Mendonça 
