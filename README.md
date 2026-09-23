# Simulador de ECU

Projeto final da disciplina **Programação e Desenvolvimento de Software 2 (PDS2)** — UFMG.

## Descrição do problema

Uma ECU (Engine Control Unit) é o computador embarcado responsável por controlar o funcionamento de um motor de combustão interna: ela lê sensores, executa estratégias de controle (injeção de combustível, ignição, malha fechada lambda, marcha lenta) e comanda atuadores em tempo real, além de detectar falhas e registrar códigos de diagnóstico (DTCs).

Este projeto implementa em C++11, executando no terminal, um simulador dessa ECU acoplado a um modelo simplificado de motor. Cenários de condução e tabelas de calibração são carregados de arquivos de texto; as séries temporais resultantes são exportadas em CSV para análise posterior.

## Objetivos

- Aplicar os fundamentos de POO (encapsulamento, herança, polimorfismo, tratamento de exceções) em um sistema de porte médio.
- Modelar hierarquias de classes abstratas para sensores, atuadores e estratégias de controle.
- Praticar modularização, separação entre especificação (`.hpp`) e implementação (`.cpp`), e compilação incremental com `make`.
- Trabalhar com persistência em arquivos de texto (mapas de calibração, cenários, logs).
- Produzir documentação técnica com Doxygen.
- Cobrir o comportamento das classes com testes automatizados (doctest).

## Árvore de diretórios

```
ecu-simulation-project/
├── include/            # Cabeçalhos .hpp — especificação (contrato) das classes
│   ├── core/           #   núcleo da simulação (motor, orquestrador, tempo)
│   ├── sensors/        #   hierarquia Sensor e sensores concretos (RPM, TPS, MAP, ECT, IAT, O2)
│   ├── actuators/      #   hierarquia Actuator (injetor, bobina, ventoinha, IAC)
│   ├── control/        #   estratégias de controle (injeção, ignição, PID lambda, limitador, limp mode)
│   ├── calibration/    #   tabelas 2D/3D e interpolação
│   ├── diagnostics/    #   DTCs no estilo OBD-II e detecção de falhas
│   ├── io/             #   leitura de mapas/cenários e escrita de logs CSV
│   ├── exceptions/     #   hierarquia de exceções derivada de std::exception
│   └── ui/             #   interface de menu no terminal
├── src/                # Implementações .cpp — espelham a estrutura de include/
│   ├── main.cpp        #   ponto de entrada
│   ├── core/
│   ├── sensors/
│   ├── actuators/
│   ├── control/
│   ├── calibration/
│   ├── diagnostics/
│   ├── io/
│   ├── exceptions/
│   └── ui/
├── tests/              # Testes com doctest (header único)
├── data/               # Dados de entrada e saída em texto
│   ├── maps/           #   tabelas de calibração
│   ├── scenarios/      #   perfis de condução
│   └── logs/           #   séries temporais em CSV geradas pela execução
├── design/             # Modelagem: user stories, cartões CRC, diagramas UML, decisões de arquitetura
├── build/              # Objetos, binários e saída do Doxygen em build/docs/ (conteúdo ignorado pelo Git)
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

Artefatos de modelagem (user stories, cartões CRC e diagramas UML) ficam em `design/`. A documentação de API é gerada pelo Doxygen a partir dos comentários dos cabeçalhos (`@brief`, `@param`, `@return`, `@throws`) e sai em `build/docs/` ao executar `make docs` — esta saída não é versionada.

## Integrantes

Miguel Almeida Faria,  Guilherme Teixera de Paula, Joao Victor Resende Fernandes, Guilherme Augusto de Brito Mendonça 
