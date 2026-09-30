#ifndef ECU_CORE_ECUCONTROLLER_HPP
#define ECU_CORE_ECUCONTROLLER_HPP

#include <memory>
#include <string>

// Declarações antecipadas — os headers das classes colaboradoras serão
// escritos pelos demais integrantes do grupo em seus módulos respectivos.
namespace ecu {
namespace state    { class VehicleState; }
namespace io       { class SensorManager; class DataLogger; }
namespace control  { class TorqueMapper; }
namespace safety   { class SafetyMonitor; }
}

namespace ecu {
namespace core {

/**
 * @brief Orquestrador central da simulação de ECU.
 *
 * Responsabilidades (conforme cartão CRC):
 *  - Inicializar todos os subsistemas da simulação.
 *  - Conhecer o estado atual do veículo na Máquina de Estados Finitos.
 *  - Executar o *Main Loop* de atualização contínua (um tick por
 *    iteração).
 *  - Delegar o cálculo final de torque ao `TorqueMapper`.
 *  - Forçar transição para `FaultState` caso receba sinal de abertura do
 *    *Shutdown System* (ou qualquer outra falha crítica notificada pelo
 *    `SafetyMonitor`).
 *
 * O controlador mantém referências aos colaboradores externos
 * (`SensorManager`, `SafetyMonitor`, `TorqueMapper`, `DataLogger`) — cuja
 * criação e vida útil ficam a cargo do chamador — e é dono exclusivo do
 * `VehicleState` corrente, gerenciado via `std::unique_ptr` para permitir
 * troca polimórfica.
 */
class ECUController {
public:
    /**
     * @brief Constrói o controlador acoplado aos colaboradores externos.
     *
     * As referências recebidas devem permanecer válidas durante toda a
     * vida útil do `ECUController`.
     *
     * @param sensors Fonte de telemetria bruta (pedal, temperaturas,
     *                pressão de freio, tensão).
     * @param safety  Monitor que valida condições elétricas/térmicas e o
     *                *Shutdown System*.
     * @param torque  Mapeador de torque responsável pelo cálculo final.
     * @param logger  Registrador de telemetria e eventos críticos.
     */
    ECUController(ecu::io::SensorManager& sensors,
                  ecu::safety::SafetyMonitor& safety,
                  ecu::control::TorqueMapper& torque,
                  ecu::io::DataLogger& logger);

    /**
     * @brief Destrutor. Definido fora da classe para que o `unique_ptr`
     *        de `VehicleState` possa lidar com o tipo incompleto no
     *        header.
     */
    ~ECUController();

    ECUController(const ECUController&) = delete;
    ECUController& operator=(const ECUController&) = delete;

    /**
     * @brief Inicializa todos os subsistemas e coloca o veículo em
     *        `IdleState`.
     *
     * @throws ecu::exceptions::EcuException Em caso de falha crítica
     *         detectada durante a inicialização (por exemplo, arquivo de
     *         telemetria inválido no `SensorManager`).
     */
    void initialize();

    /**
     * @brief Executa uma iteração do *Main Loop*.
     *
     * A cada tick o controlador:
     *  1. avança a leitura no `SensorManager`;
     *  2. consulta o `SafetyMonitor` e, se necessário, força `FaultState`;
     *  3. delega ao estado atual o tratamento do pedal e a interação com
     *     o `TorqueMapper`;
     *  4. grava a linha de telemetria no `DataLogger`.
     */
    void tick();

    /**
     * @brief Executa o *Main Loop* completo até esgotar a telemetria ou
     *        receber solicitação de parada.
     *
     * @throws ecu::exceptions::EcuException Propaga falhas críticas não
     *         tratadas internamente pelo controlador.
     */
    void run();

    /**
     * @brief Solicita o encerramento cooperativo do *Main Loop* na próxima
     *        iteração.
     */
    void stop() noexcept;

    /**
     * @brief Realiza a transição para um novo estado, respeitando as
     *        regras declaradas pelo estado atual em `canTransitionTo`.
     *
     * A rotina `onExit` do estado anterior é chamada antes da troca, e
     * `onEnter` do novo estado é chamada logo após — ambas recebem uma
     * referência a este controlador.
     *
     * @param newState Novo estado (posse transferida ao controlador).
     */
    void transitionTo(std::unique_ptr<ecu::state::VehicleState> newState);

    /**
     * @brief Consulta o estado atual do veículo.
     * @return Referência constante ao objeto de estado corrente.
     */
    const ecu::state::VehicleState& currentState() const;

    /**
     * @brief Força a transição imediata para `FaultState` com o motivo
     *        informado.
     *
     * Utilizado pelo `SafetyMonitor` (via callback do próprio controlador)
     * quando uma falha crítica precisa interromper o ciclo em curso —
     * abertura do Shutdown System, subtensão, sobretemperatura ou
     * timeout de pré-carga.
     *
     * @param reason Descrição textual da falha, gravada pelo `DataLogger`.
     */
    void forceFault(const std::string& reason);

    /**
     * @brief Acesso somente-leitura ao `TorqueMapper` para uso dos
     *        estados durante suas rotinas.
     * @return Referência ao mapeador de torque associado.
     */
    ecu::control::TorqueMapper& torqueMapper() const noexcept;

    /**
     * @brief Acesso somente-leitura ao `DataLogger` para uso dos estados
     *        durante suas rotinas.
     * @return Referência ao registrador associado.
     */
    ecu::io::DataLogger& dataLogger() const noexcept;

    /**
     * @brief Acesso somente-leitura ao `SafetyMonitor` para uso dos
     *        estados durante suas rotinas.
     * @return Referência ao monitor de segurança associado.
     */
    ecu::safety::SafetyMonitor& safetyMonitor() const noexcept;

    /**
     * @brief Acesso somente-leitura ao `SensorManager` para uso dos
     *        estados durante suas rotinas.
     * @return Referência ao gerenciador de sensores associado.
     */
    ecu::io::SensorManager& sensorManager() const noexcept;

private:
    ecu::io::SensorManager& m_sensors;
    ecu::safety::SafetyMonitor& m_safety;
    ecu::control::TorqueMapper& m_torque;
    ecu::io::DataLogger& m_logger;
    std::unique_ptr<ecu::state::VehicleState> m_currentState;
    bool m_running;
};

} // namespace core
} // namespace ecu

#endif // ECU_CORE_ECUCONTROLLER_HPP
