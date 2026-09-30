# User Stories — Simulador de ECU (Checkpoint C5)

Documento derivado exclusivamente das responsabilidades já registradas em
`cartoes_crc.md`. Papéis considerados:

- **Piloto/motorista** — quem opera o veículo durante a simulação.
- **Engenheiro de calibração** — quem prepara os mapas de torque e os cenários de condução.
- **Engenheiro de segurança/manutenção** — quem monitora limites elétricos/térmicos e analisa a telemetria.

## US01 — Alternar mapa de motor durante a condução
**Como** piloto, **quero** alternar entre os mapas Eco, MidTerm e Sport com o veículo em tração, **para** ajustar o comportamento do carro à situação atual.

- **Prioridade:** Alta
- **Critérios de aceitação:**
  1. Com o veículo em `DriveState`, a seleção de um novo mapa faz o `TorqueMapper` passar a usar a tabela correspondente no próximo tick.
  2. Com o veículo em `IdleState`, a tentativa de troca é bloqueada por `VehicleState` e a tabela ativa permanece inalterada.
  3. Toda troca aceita é gravada pelo `DataLogger` com o timestamp do ciclo.

## US02 — Traduzir posição do pedal em torque
**Como** piloto, **quero** que a leitura bruta do pedal do acelerador seja convertida em torque conforme o mapa vigente, **para** que a resposta do carro siga a calibração escolhida.

- **Prioridade:** Alta
- **Critérios de aceitação:**
  1. Para um valor de pedal entre dois pontos da tabela, o torque devolvido é a interpolação linear desses pontos.
  2. Nenhum torque devolvido excede o teto máximo de segurança do motor definido no `TorqueMapper`.
  3. A rampa (*ramping*) limita a variação de torque entre ticks consecutivos ao limite de tração mecânica.

## US03 — Entrar em Fault por subtensão da bateria
**Como** engenheiro de segurança, **quero** que o veículo entre em `FaultState` quando a tensão geral cair abaixo de 60V, **para** proteger a bateria e o motorista.

- **Prioridade:** Alta
- **Critérios de aceitação:**
  1. Ao ler tensão < 60V, o `SafetyMonitor` sinaliza falha ao `ECUController` no mesmo tick.
  2. O `ECUController` executa a transição para `FaultState` no ciclo em que a falha foi sinalizada.
  3. O `DataLogger` grava o evento como erro crítico, com timestamp e motivo.

## US04 — Alertar e proteger por temperatura
**Como** engenheiro de segurança, **quero** receber alerta térmico aos 60°C e disparo de falha crítica aos 80°C em motor ou bateria, **para** agir antes de dano permanente.

- **Prioridade:** Alta
- **Critérios de aceitação:**
  1. Ao atingir 60°C, o `SafetyMonitor` emite um *warning* que o `DataLogger` registra com timestamp.
  2. Ao atingir 80°C, o `SafetyMonitor` dispara falha crítica que leva o veículo a `FaultState`.
  3. Após a falha crítica, o input do pedal passa a ser ignorado pelo `FaultState`.

## US05 — Fault imediato ao abrir o Shutdown System
**Como** engenheiro de segurança, **quero** que qualquer abertura do *Shutdown System* force a transição para `FaultState`, **para** cumprir o requisito básico de segurança elétrica.

- **Prioridade:** Alta
- **Critérios de aceitação:**
  1. Enquanto o *Shutdown System* estiver aberto, o `SafetyMonitor` reporta a condição continuamente ao `ECUController`.
  2. O `ECUController` transita para `FaultState` no primeiro tick em que a abertura for detectada.
  3. O `DataLogger` registra a abertura como evento crítico com timestamp.

## US06 — Carregar cenário de condução via CSV
**Como** engenheiro de calibração, **quero** apontar um arquivo CSV com o cenário de condução, **para** rodar simulações reproduzíveis a partir de dados de pista.

- **Prioridade:** Média
- **Critérios de aceitação:**
  1. O `SensorManager` abre o CSV cujo caminho foi informado sem provocar crash.
  2. Linhas com formatação inválida são puladas e a simulação prossegue com as linhas subsequentes.
  3. A cada tick, o `SensorManager` avança para a próxima linha e disponibiliza pedal, temperaturas, pressão de freio e tensão.

## US07 — Exportar telemetria em texto
**Como** engenheiro de calibração, **quero** que a simulação grave a telemetria em arquivo texto, **para** analisar o comportamento do sistema após a corrida.

- **Prioridade:** Média
- **Critérios de aceitação:**
  1. Cada ciclo grava pedal, torque e temperatura em uma linha do arquivo configurado, anexando o timestamp.
  2. Trocas de mapa (Eco/MidTerm/Sport) aparecem no log com o timestamp da transição.
  3. Eventos críticos (subtensão, sobretemperatura, abertura do Shutdown, timeout de pré-carga) são gravados mesmo em caso de encerramento anormal.

## US08 — Bloquear tração por timeout de pré-carga
**Como** engenheiro de manutenção, **quero** que o veículo não avance para tração se a sequência de pré-carga estourar o *timeout*, **para** evitar acionar o inversor em condição elétrica inválida.

- **Prioridade:** Média
- **Critérios de aceitação:**
  1. O `SafetyMonitor` acompanha o *timeout* da pré-carga desde o início da simulação.
  2. Em caso de estouro, a transição `IdleState` → `DriveState` é bloqueada por `VehicleState`.
  3. O motivo do bloqueio é registrado pelo `DataLogger`.

## Requisitos Não Funcionais

- **RNF01 — Linguagem:** todo o código compila em C++11 estrito (`-std=c++11 -Wall -Wextra`, zero warnings), sem recursos de C++14/17/20.
- **RNF02 — Plataforma:** o binário deve ser executável em Linux, usando somente STL (única exceção: `doctest`, para testes).
- **RNF03 — Persistência:** entrada (cenários) e saída (telemetria) usam apenas arquivos de texto (`.csv`, `.txt`). Sem banco de dados nem JSON via biblioteca externa.
- **RNF04 — Robustez de parsing:** o `SensorManager` deve ignorar linhas mal formatadas do CSV sem interromper a simulação, sinalizando `TelemetryParseError` quando o consumidor precisar reagir à linha inválida.
- **RNF05 — Testabilidade:** cada classe do domínio (`TorqueMapper`, `SafetyMonitor`, estados de `VehicleState` etc.) deve ser instanciável e testável de forma isolada com `doctest`.
- **RNF06 — Modularidade:** classes agrupadas em namespaces (`ecu::core`, `ecu::state`, `ecu::control`, `ecu::io`, `ecu::safety`, `ecu::exceptions`), um cabeçalho por classe e implementação separada em `.cpp`.

## Rastreabilidade User Stories → Classes

| ID   | Classes principais                                                                                       |
|------|-----------------------------------------------------------------------------------------------------------|
| US01 | `TorqueMapper`, `VehicleState`, `IdleState`, `DriveState`, `DataLogger`                                   |
| US02 | `TorqueMapper`, `SensorManager`                                                                           |
| US03 | `SafetyMonitor`, `ECUController`, `FaultState`, `DataLogger`, `UndervoltageFault`                         |
| US04 | `SafetyMonitor`, `DataLogger`, `FaultState`, `OvertemperatureFault`                                       |
| US05 | `SafetyMonitor`, `ECUController`, `FaultState`, `DataLogger`, `ShutdownOpenFault`                         |
| US06 | `SensorManager`, `TelemetryParseError`                                                                    |
| US07 | `DataLogger`, `ECUController`, `SensorManager`                                                            |
| US08 | `SafetyMonitor`, `VehicleState`, `IdleState`, `DriveState`, `DataLogger`, `PrechargeTimeoutFault`         |
