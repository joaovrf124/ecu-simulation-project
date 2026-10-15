#ifndef ECU_SAFETY_SAFETYMONITOR_HPP
#define ECU_SAFETY_SAFETYMONITOR_HPP

namespace ecu {
namespace io { class SensorManager; }
}

namespace ecu {
namespace safety {

/**
 * @brief Vigia contínuo das condições elétricas e térmicas do veículo.
 *
 * Responsabilidades (conforme cartão CRC):
 *  - Validar continuamente se o *Shutdown System* está fechado.
 *  - Monitorar a tensão da bateria, acionando falha abaixo de 60 V
 *    (`kUndervoltageThresholdVolts`).
 *  - Ler a pressão de freio para validar condições de segurança.
 *  - Emitir *warning* aos 60 °C (`kTemperatureWarningCelsius`) e disparar
 *    falha crítica aos 80 °C (`kTemperatureFaultCelsius`) em motor ou
 *    bateria.
 *  - Monitorar o *timeout* da sequência de pré-carga
 *    (`kPrechargeTimeoutTicks`), impedindo a transição `Idle → Drive` em
 *    caso de estouro.
 *
 * As falhas são comunicadas lançando a exceção da hierarquia
 * `EcuException` apropriada; cabe ao `ECUController` capturar e forçar a
 * transição para `FaultState`.
 */
class SafetyMonitor {
public:
    /**
     * @brief Limite inferior de tensão da bateria, em volts. Abaixo dele é
     *        gerada `UndervoltageFault`.
     */
    static const double kUndervoltageThresholdVolts;

    /**
     * @brief Limite de alerta térmico, em °C. Igual ou acima dispara
     *        *warning* registrado pelo `DataLogger`.
     */
    static const double kTemperatureWarningCelsius;

    /**
     * @brief Limite de falha crítica térmica, em °C. Igual ou acima
     *        dispara `OvertemperatureFault`.
     */
    static const double kTemperatureFaultCelsius;

    /**
     * @brief Número máximo de ticks aceitos para a sequência de pré-carga.
     *        Além disso é gerada `PrechargeTimeoutFault`.
     */
    static const unsigned long kPrechargeTimeoutTicks;

    /**
     * @brief Constrói o monitor acoplado ao `SensorManager` que fornece a
     *        telemetria.
     *
     * A referência recebida deve permanecer válida durante toda a vida
     * útil do monitor.
     *
     * @param sensors Fonte de leituras de pedal, temperaturas, pressão de
     *                freio e tensão.
     */
    explicit SafetyMonitor(ecu::io::SensorManager& sensors);

    /**
     * @brief Destrutor padrão.
     */
    ~SafetyMonitor() noexcept;

    SafetyMonitor(const SafetyMonitor&) = delete;
    SafetyMonitor& operator=(const SafetyMonitor&) = delete;

    /**
     * @brief Avalia as condições elétricas e térmicas atuais do veículo.
     *
     * @throws ecu::exceptions::UndervoltageFault     se a tensão lida for
     *         inferior a `kUndervoltageThresholdVolts`.
     * @throws ecu::exceptions::OvertemperatureFault  se a temperatura do
     *         motor ou da bateria for maior ou igual a
     *         `kTemperatureFaultCelsius`.
     * @throws ecu::exceptions::ShutdownOpenFault     se o *Shutdown
     *         System* for reportado aberto.
     */
    void check();

    /**
     * @brief Avança a contagem interna de ticks de pré-carga.
     *
     * Deve ser chamado uma vez por tick enquanto o veículo estiver em
     * `IdleState`, até que `markPrechargeComplete()` seja invocado.
     *
     * @throws ecu::exceptions::PrechargeTimeoutFault se o contador superar
     *         `kPrechargeTimeoutTicks` sem conclusão da pré-carga.
     */
    void tickPrecharge();

    /**
     * @brief Sinaliza que a sequência de pré-carga foi concluída com
     *        sucesso dentro do *timeout*. Habilita a transição para
     *        `DriveState`.
     */
    void markPrechargeComplete() noexcept;

    /**
     * @brief Indica se o *Shutdown System* está considerado aberto na
     *        avaliação mais recente.
     * @return `true` se aberto; `false` se fechado.
     */
    bool isShutdownOpen() const noexcept;

    /**
     * @brief Força o estado do *Shutdown System*. Útil para testes e para
     *        o cenário onde o sinal provém de outra fonte que não o CSV.
     * @param open `true` para marcar como aberto; `false` para fechado.
     */
    void setShutdownOpen(bool open) noexcept;

    /**
     * @brief Indica se a última leitura térmica cruzou o patamar de 60 °C
     *        sem atingir o de 80 °C.
     * @return `true` se há *warning* térmico ativo.
     */
    bool hasThermalWarning() const noexcept;

    /**
     * @brief Indica se a pré-carga já foi marcada como concluída.
     * @return `true` após `markPrechargeComplete()`; `false` antes.
     */
    bool isPrechargeComplete() const noexcept;

    /**
     * @brief Número de ticks acumulados aguardando a conclusão da
     *        pré-carga.
     * @return Contador interno de ticks de pré-carga.
     */
    unsigned long prechargeTicks() const noexcept;

private:
    ecu::io::SensorManager& m_sensors;
    bool m_shutdownOpen;
    bool m_thermalWarning;
    bool m_prechargeComplete;
    unsigned long m_prechargeTicks;
};

} // namespace safety
} // namespace ecu

#endif // ECU_SAFETY_SAFETYMONITOR_HPP
