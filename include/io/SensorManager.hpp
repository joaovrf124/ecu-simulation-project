#ifndef ECU_IO_SENSORMANAGER_HPP
#define ECU_IO_SENSORMANAGER_HPP

#include <fstream>
#include <string>

namespace ecu {
namespace io {

/**
 * @brief Gerencia a leitura da telemetria simulada a partir de um arquivo
 *        CSV.
 *
 * Responsabilidades (conforme cartão CRC):
 *  - Conhecer o caminho do arquivo CSV de entrada.
 *  - Abrir, ler e validar os dados de telemetria, pulando linhas mal
 *    formatadas sem interromper a simulação.
 *  - Conhecer e fornecer a posição momentânea do pedal do acelerador.
 *  - Conhecer e fornecer as temperaturas de motor e bateria, a pressão de
 *    freio e a tensão geral.
 *  - Avançar a leitura para a próxima linha (próximo tick da simulação).
 *
 * O formato esperado de cada linha do CSV é:
 * `pedal,engineTemp,batteryTemp,brakePressure,voltage` — todos em `double`,
 * separados por vírgula. Linhas em branco ou cujo parse falhe são puladas
 * silenciosamente; a exceção `TelemetryParseError` só é lançada quando o
 * consumidor chama a sobrecarga `advanceStrict()` e deseja reagir à falha.
 */
class SensorManager {
public:
    /**
     * @brief Constrói o gerenciador apontando para o CSV de entrada.
     *
     * O arquivo não é aberto imediatamente; a abertura efetiva ocorre no
     * `open()`.
     *
     * @param filePath Caminho (relativo ou absoluto) para o arquivo CSV.
     */
    explicit SensorManager(const std::string& filePath);

    /**
     * @brief Destrutor — fecha o arquivo se ainda estiver aberto.
     */
    ~SensorManager() noexcept;

    SensorManager(const SensorManager&) = delete;
    SensorManager& operator=(const SensorManager&) = delete;

    /**
     * @brief Abre o arquivo CSV configurado e posiciona-se antes da
     *        primeira linha de dados.
     *
     * @throws ecu::exceptions::TelemetryParseError se o arquivo não puder
     *         ser aberto (não existe, sem permissão, caminho inválido).
     */
    void open();

    /**
     * @brief Avança para a próxima linha útil do CSV, pulando linhas
     *        inválidas em silêncio.
     *
     * @return `true` se uma nova linha válida foi lida; `false` quando a
     *         leitura atinge o fim do arquivo.
     */
    bool advance();

    /**
     * @brief Variante estrita de `advance()`: lança exceção se a linha
     *        corrente estiver mal formatada, em vez de pular.
     *
     * @return `true` se uma nova linha válida foi lida; `false` ao final
     *         do arquivo.
     * @throws ecu::exceptions::TelemetryParseError quando a linha não é
     *         conformante com o formato esperado.
     */
    bool advanceStrict();

    /**
     * @brief Indica se o arquivo CSV ainda tem linhas disponíveis para
     *        leitura.
     * @return `true` se ainda há dados a consumir; `false` caso contrário.
     */
    bool hasNext() const noexcept;

    /**
     * @brief Retorna a leitura mais recente da posição do pedal do
     *        acelerador, em porcentagem no intervalo [0.0, 100.0].
     * @return Último valor de pedal lido do CSV.
     */
    double pedal() const noexcept;

    /**
     * @brief Retorna a leitura mais recente da temperatura do motor, em °C.
     * @return Último valor de temperatura do motor lido do CSV.
     */
    double engineTemperature() const noexcept;

    /**
     * @brief Retorna a leitura mais recente da temperatura da bateria, em
     *        °C.
     * @return Último valor de temperatura da bateria lido do CSV.
     */
    double batteryTemperature() const noexcept;

    /**
     * @brief Retorna a leitura mais recente da pressão do freio.
     * @return Último valor de pressão de freio lido do CSV.
     */
    double brakePressure() const noexcept;

    /**
     * @brief Retorna a leitura mais recente da tensão geral do sistema, em
     *        volts.
     * @return Último valor de tensão lido do CSV.
     */
    double voltage() const noexcept;

    /**
     * @brief Retorna o número do tick corrente (0 antes do primeiro
     *        `advance()`, 1 após a primeira linha válida, e assim por
     *        diante).
     * @return Contador de ticks consumidos.
     */
    unsigned long currentTick() const noexcept;

    /**
     * @brief Caminho do arquivo CSV configurado.
     * @return Referência constante ao caminho fornecido na construção.
     */
    const std::string& filePath() const noexcept;

private:
    std::string m_filePath;
    std::ifstream m_stream;
    bool m_open;
    bool m_hasNext;
    unsigned long m_tick;
    double m_pedal;
    double m_engineTemp;
    double m_batteryTemp;
    double m_brakePressure;
    double m_voltage;
};

} // namespace io
} // namespace ecu

#endif // ECU_IO_SENSORMANAGER_HPP
