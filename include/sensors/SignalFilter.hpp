#ifndef ECU_SENSORS_SIGNALFILTER_HPP
#define ECU_SENSORS_SIGNALFILTER_HPP

namespace ecu {
namespace sensors {

/**
 * @brief Filtro digital passa-baixa de Media Movel Exponencial (EMA) para sensores da ECU.
 *
 * Suaviza leituras ruidosas de pedal, tensao e temperatura antes de entrega-las
 * a malha de controle.
 */
class SignalFilter {
public:
    /**
     * @brief Constroi o filtro com fator de suavizacao alpha.
     * @param alpha Fator no intervalo (0.0, 1.0]. Valores menores filtram mais ruido;
     *              1.0 desativa a filtragem (saida igual a entrada).
     * @throws std::invalid_argument se alpha estiver fora de (0.0, 1.0].
     */
    explicit SignalFilter(double alpha = 0.25);
    ~SignalFilter() noexcept;

    /**
     * @brief Processa uma nova amostra bruta do sensor e retorna o valor filtrado.
     * @param rawSample Leitura instantanea do sensor.
     * @return Valor suavizado pelo filtro.
     */
    double filter(double rawSample) noexcept;

    /**
     * @brief Reinicia o historico do filtro.
     */
    void reset() noexcept;

    double alpha() const noexcept;
    double lastValue() const noexcept;
    bool isInitialized() const noexcept;

private:
    double m_alpha;
    double m_lastValue;
    bool m_initialized;
};

} // namespace sensors
} // namespace ecu

#endif // ECU_SENSORS_SIGNALFILTER_HPP