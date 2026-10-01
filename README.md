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
│   ├── control/        #   TorqueMapper (mapas Eco/MidTerm/Sport)
│   ├── safety/         #   SafetyMonitor (Shutdown, subtensão, temperatura, pré-carga)
│   ├── io/             #   SensorManager (CSV de entrada) e DataLogger (telemetria)
│   └── exceptions/     #   Hierarquia EcuException e derivadas
├── src/                # Implementações .cpp — espelham a estrutura de include/
│   ├── main.cpp        #   ponto de entrada
│   ├── core/
│   ├── state/
│   ├── control/
│   ├── safety/
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

*Em construção.* Será provido um `Makefile` com os alvos `make`, `make run`, `make test`, `make clean` e `make docs`.

## Funcionalidades

*Em construção.*

## Testes

*Em construção.*

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
