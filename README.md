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
| `make run`   | Compila (se necessário) e executa `build/ecu_simulator` com o cenário padrão. |
| `make test`  | Compila os testes de `tests/` com os `.cpp` do projeto (exceto `main.cpp`) e executa `build/tests_runner`. |
| `make clean` | Remove tudo o que foi gerado em `build/`.                                 |
| `make docs`  | Gera a documentação Doxygen em `build/docs/`.                             |

As flags obrigatórias (`-std=c++11 -Wall -Wextra -I include`) já são definidas no `Makefile`, assim como `-MMD -MP` para dependências automáticas de headers.

No Linux, basta ter `g++` e `make` instalados. No Windows, use o MSYS2 (`pacman -S make mingw-w64-x86_64-gcc`) ou invoque `mingw32-make` dentro do MSYS2/MinGW.

### Rodando o simulador com o cenário de exemplo

Da raiz do projeto:

```
make
./build/ecu_simulator
```

O executável sem argumentos usa `data/scenarios/exemplo.csv` como cenário de entrada e grava a telemetria em `data/logs/exemplo.log`. Para apontar para outros arquivos:

```
./build/ecu_simulator <cenario.csv> <arquivo_de_log>
```

Qualquer exceção lançada pelos subsistemas é capturada em `main()` e impressa com mensagem clara; o programa retorna código de saída diferente de zero sem travar.

## Funcionalidades

O checkpoint C8 encerra a fase verde do TDD: todas as classes do núcleo estão implementadas e os 51 testes da suíte doctest passam.

- **Leitura de cenário em CSV** (`SensorManager`): cada linha tem o formato `pedal,engineTemp,batteryTemp,brakePressure,voltage`. Linhas vazias ou mal formatadas são puladas em silêncio (`advance()`); a sobrecarga `advanceStrict()` sinaliza o consumidor com `TelemetryParseError`.
- **Máquina de Estados do veículo** (`VehicleState` + `IdleState`/`DriveState`/`FaultState`): aplica o padrão *State*. `Idle` bloqueia pedal e troca de mapa; `Drive` processa pedal e autoriza troca dinâmica entre Eco/MidTerm/Sport; `Fault` é terminal, zera a requisição de torque e grava o motivo no log.
- **Mapeador de torque** (`TorqueMapper`): interpolação linear entre os pontos da tabela ativa, rampa mecânica (`kMaxTorqueDelta`) entre ticks consecutivos e teto absoluto de segurança (`kMaxTorque`). Clampa pedal para `[0, 100]`.
- **Monitor de segurança** (`SafetyMonitor`): lança `UndervoltageFault` abaixo de 60 V, `OvertemperatureFault` em motor ou bateria a partir de 80 °C, `ShutdownOpenFault` quando o circuito está aberto e `PrechargeTimeoutFault` após estouro do contador de pré-carga. Entre 60 °C e 80 °C emite *warning* térmico sem interromper o ciclo.
- **Logger de telemetria** (`DataLogger`): grava cada registro prefixado por *timestamp* e tag (`SAMPLE`, `MODE`, `WARN`, `ERROR`). Eventos `ERROR` recebem *flush* imediato para garantir persistência mesmo em caso de pane posterior.
- **Orquestrador** (`ECUController`): dirige o *Main Loop* (`run()`/`tick()`), consulta o `SafetyMonitor` em cada iteração, delega o pedal ao `TorqueMapper` quando o estado corrente aceita, grava amostras no `DataLogger` e força transição para `FaultState` (com *latching*: uma vez em Fault, não volta sozinho).
- **Hierarquia de exceções** (`EcuException` e derivadas) para separar falhas semânticas do simulador de erros genéricos de C++.

### Cenário de exemplo

`data/scenarios/exemplo.csv` traz 11 ticks que exercitam o fluxo completo:

1. Dois ticks em repouso (`pedal = 0`).
2. Aceleração progressiva até `pedal = 100 %`.
3. Aquecimento gradual do motor, acionando o *warning* térmico aos 60 °C.
4. Último tick com `engineTemp = 82 °C`, que dispara `OvertemperatureFault` → transição para `FaultState`.

Executando `make run` (ou `./build/ecu_simulator`) a partir da raiz do projeto o simulador lê esse cenário e grava `data/logs/exemplo.log`, com linhas `SAMPLE`, `WARN` e, no final, um `ERROR` registrando o motivo da falha.

Para rodar com outro cenário:

```
./build/ecu_simulator data/scenarios/outro.csv data/logs/outro.log
```

## Testes

A suíte de testes utiliza [doctest](https://github.com/doctest/doctest) (header único em `tests/third_party/doctest.h`). Cada classe de domínio tem seu próprio arquivo de teste em `tests/`:

- `test_exceptions.cpp`      — hierarquia `EcuException` e derivadas.
- `test_states.cpp`          — `VehicleState`, `IdleState`, `DriveState`, `FaultState`.
- `test_sensor_manager.cpp`  — leitura de CSV, linhas inválidas, erro de abertura.
- `test_torque_mapper.cpp`   — interpolação, rampa, teto de segurança, modos.
- `test_safety_monitor.cpp`  — limites de 60 V, 60 °C, 80 °C e *timeout* de pré-carga.
- `test_data_logger.cpp`     — gravação de amostras, trocas de modo, alertas e erros.
- `test_ecu_controller.cpp`  — estado inicial, transição para `FaultState`, acesso aos colaboradores.
- `test_defensive_input.cpp` — validações do C9: `FileIOException`, campos NaN/infinito, faixas físicas, tabela de torque vazia/duplicada/fora de ordem, pré-condições de `DataLogger` e `ECUController`.

Para rodar:

```
make test
```

## Programação defensiva e tratamento de exceções

O checkpoint C9 adiciona uma camada de validação de entrada e separa com mais precisão os tipos de falha propagados pelo simulador.

### Hierarquia de exceções

```
std::exception
└── ecu::exceptions::EcuException (abstrata)
    ├── TelemetryParseError      — conteúdo do CSV mal formatado ou fora da faixa física
    ├── FileIOException          — falha de I/O (abrir, gravar) em qualquer arquivo
    ├── UndervoltageFault        — tensão abaixo de 60 V (US03)
    ├── OvertemperatureFault     — motor ou bateria ≥ 80 °C (US04)
    ├── ShutdownOpenFault        — Shutdown System aberto (US05)
    └── PrechargeTimeoutFault    — timeout da pré-carga (US08)
```

Além dessas, as classes também lançam `std::invalid_argument` (dados de entrada inválidos) e `std::logic_error` (violação de pré-condição) quando o problema é de contrato interno — não é falha de domínio do simulador.

### Validações aplicadas

| Classe           | Validação                                                                                                                                    | Exceção                                 |
|------------------|-----------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------|
| `SensorManager`  | Arquivo inexistente/sem permissão em `open()`                                                                                                 | `FileIOException`                       |
| `SensorManager`  | Arquivo abriu mas está vazio                                                                                                                  | `TelemetryParseError`                   |
| `SensorManager`  | Linha com NaN/infinito, pedal fora de `[0, 100]`, temperatura fora de `[-40, 200] °C`, tensão/pressão negativas ou fora de faixa, nº de campos errado | pulada em `advance()`; `TelemetryParseError` em `advanceStrict()` |
| `TorqueMapper`   | `setMapTable` com tabela vazia, pontos fora de ordem crescente de pedal ou pedais duplicados                                                  | `std::invalid_argument`                 |
| `DataLogger`     | `open()` em caminho inacessível                                                                                                               | `FileIOException`                       |
| `DataLogger`     | `logSample/logModeChange/logWarning/logError` chamado sem `open()` prévio                                                                     | `FileIOException`                       |
| `ECUController`  | `tick()` chamado antes de `initialize()`                                                                                                      | `std::logic_error`                      |
| `ECUController`  | `transitionTo(nullptr)`                                                                                                                       | `std::invalid_argument`                 |

### Tratamento no `main`

O `main()` captura cada família separadamente e devolve um código de saída específico, para que scripts externos possam distinguir o tipo de falha sem recorrer ao texto:

| Código | Caso                                                                        |
|--------|-----------------------------------------------------------------------------|
| 0      | Simulação concluída (mesmo que termine em `FaultState`).                   |
| 10     | Argumento de cenário aponta para arquivo inexistente.                      |
| 11     | `FileIOException` em qualquer componente.                                   |
| 12     | `TelemetryParseError` (ex.: cenário sem linhas úteis).                     |
| 13     | `std::invalid_argument`.                                                   |
| 14     | `std::logic_error`.                                                        |
| 1      | `EcuException` não classificada acima.                                     |
| 2      | `std::exception` genérica.                                                 |
| 3      | Exceção desconhecida (`catch (...)`).                                      |

O programa também aceita `--help`/`-h` para imprimir a sintaxe de uso.

### Cenário corrompido

Para demonstrar que o simulador não quebra com entradas sujas, o diretório `data/scenarios/` inclui `corrompido.csv`, com uma mistura de linhas válidas, linha vazia, texto não numérico, pedal e tensão fora da faixa e número errado de campos.

```
./build/ecu_simulator data/scenarios/corrompido.csv data/logs/corrompido.log
```

O simulador consome o arquivo até o fim: pula silenciosamente as linhas inválidas, processa as válidas até uma leitura legítima de sobretemperatura (`engineTemp = 85 °C`) disparar `OvertemperatureFault`, e grava o resultado em `data/logs/corrompido.log` sem crash.

### Status atual — fase verde do TDD (C8)

Os testes foram escritos no C7 a partir dos critérios de aceitação das user stories em `design/user_stories.md`. No C8 as classes do núcleo foram implementadas até fazer todos os casos passarem — todos os 51 TEST_CASE da suíte doctest estão em verde, sem *warnings* com `-Wall -Wextra`.

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
