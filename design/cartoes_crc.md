## Cartões CRC: Modelagem da ECU

### 1. Classe: `ECUController`
* **Responsabilidades:**
  * Inicializar todos os subsistemas da simulação.
  * Conhecer o estado atual do veículo na Máquina de Estados Finitos (FSM).
  * Executar o loop principal (*Main Loop*) de atualização contínua.
  * Delegar o cálculo final de torque requisitado para a classe mapeadora.
  * Forçar a transição para o estado de *Fault* caso receba sinal de abertura do *Shutdown System*.
* **Colaborações:** `VehicleState`, `SensorManager`, `SafetyMonitor`.

### 2. Classe: `VehicleState`
* **Responsabilidades:**
  * Definir a interface padrão para os estados do carro (*Idle*, *Drive*, *Fault*).
  * Conhecer as regras de transição permitidas a partir do estado momentâneo atual.
  * Bloquear a tentativa de troca de mapas de motor se o veículo estiver no estado desligado/inativo.
  * Validar se o input do pedal deve ser processado ou ignorado no estado atual.
  * Executar rotinas de configuração ao entrar e sair de cada estado.
* **Colaborações:** `ECUController`, `TorqueMapper`, `DataLogger`.

### 3. Classe: `TorqueMapper`
* **Responsabilidades:**
  * Conhecer qual a tabela de mapeamento atual (Eco, MidTerm ou Sport) está selecionada.
  * Receber a porcentagem bruta do pedal do acelerador lida pelos sensores.
  * Calcular a interpolação linear entre os pontos da tabela para encontrar o torque exato.
  * Aplicar o algoritmo de rampa (*ramping*) otimizado para o limite de tração mecânica.
  * Limitar a requisição de torque final ao teto máximo de segurança do motor.
* **Colaborações:** `SensorManager`, `VehicleState`.

### 4. Classe: `SensorManager`
* **Responsabilidades:**
  * Conhecer o caminho e o nome do arquivo CSV de entrada para a simulação.
  * Abrir, ler e validar os dados de telemetria simulada, pulando falhas de formatação.
  * Conhecer e fornecer a posição momentânea do pedal do acelerador.
  * Conhecer e fornecer as temperaturas do motor e bateria, além da pressão do freio e tensão geral.
  * Avançar a leitura para a próxima linha (próximo *tick* de tempo da simulação).
* **Colaborações:** `ECUController`, `SafetyMonitor`.

### 5. Classe: `SafetyMonitor`
* **Responsabilidades:**
  * Validar continuamente se o *Shutdown System* encontra-se fechado e operacional.
  * Monitorar a tensão da bateria, acionando falha se cair abaixo de 60V.
  * Ler a pressão de freio para validar condições de segurança estipuladas.
  * Emitir alertas (*Warnings*) aos 60°C e disparar gatilho de falha crítica aos 80°C no motor/baterias.
  * Monitorar o *timeout* da sequência de pré-carga, impedindo a transição para tração em caso de falha.
* **Colaborações:** `SensorManager`, `ECUController`.

### 6. Classe: `DataLogger`
* **Responsabilidades:**
  * Conhecer o diretório e o nome do arquivo de saída de telemetria do sistema (`.txt`).
  * Gravar as variáveis contínuas (pedal, torque, temperatura) anexando o *timestamp* do ciclo.
  * Registrar a troca entre os modos Eco, MidTerm e Sport com precisão de tempo.
  * Registrar os eventos de erro crítico, alertas termais ou abertura inesperada do *Shutdown System*.
  * Garantir o salvamento físico dos dados no disco mesmo em caso de travamento do controlador.
* **Colaborações:** `ECUController`, `SensorManager`.