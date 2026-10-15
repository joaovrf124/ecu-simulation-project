#ifndef ECU_IO_DATALOGGER_HPP
#define ECU_IO_DATALOGGER_HPP

#include <fstream>
#include <string>

namespace ecu {
namespace io {

/**
 * @brief Registrador de telemetria e eventos críticos em arquivo texto.
 *
 * Responsabilidades (conforme cartão CRC):
 *  - Conhecer o diretório e o nome do arquivo de saída (`.txt`/`.csv`).
 *  - Gravar as variáveis contínuas (pedal, torque, temperatura) anexando
 *    o *timestamp* do ciclo.
 *  - Registrar as trocas entre os modos Eco, MidTerm e Sport.
 *  - Registrar eventos de erro crítico, alertas termais e abertura
 *    inesperada do *Shutdown System*.
 *  - Garantir o *flush* físico dos dados mesmo em caso de travamento.
 *
 * Cada linha gravada é prefixada por um *timestamp* monotônico (contagem
 * de ticks fornecida pelo chamador), seguido de uma tag indicando o tipo
 * de registro (`SAMPLE`, `MODE`, `WARN`, `ERROR`), e dos campos
 * específicos.
 */
class DataLogger {
public:
    /**
     * @brief Constrói o logger apontando para o arquivo de saída.
     *
     * O arquivo não é aberto imediatamente; a abertura efetiva ocorre em
     * `open()`.
     *
     * @param outputPath Caminho (relativo ou absoluto) do arquivo de
     *                   telemetria.
     */
    explicit DataLogger(const std::string& outputPath);

    /**
     * @brief Destrutor — fecha o arquivo com *flush*, garantindo o
     *        salvamento físico mesmo após falhas no controlador.
     */
    ~DataLogger() noexcept;

    DataLogger(const DataLogger&) = delete;
    DataLogger& operator=(const DataLogger&) = delete;

    /**
     * @brief Abre o arquivo de saída para gravação (modo truncar).
     *
     * @throws ecu::exceptions::EcuException se o arquivo não puder ser
     *         aberto para escrita (sem permissão, diretório inexistente).
     */
    void open();

    /**
     * @brief Fecha o arquivo manualmente, forçando o *flush* físico.
     */
    void close() noexcept;

    /**
     * @brief Indica se o arquivo está aberto para gravação.
     * @return `true` se aberto; `false` caso contrário.
     */
    bool isOpen() const noexcept;

    /**
     * @brief Registra uma amostra contínua do ciclo (pedal, torque,
     *        temperatura).
     *
     * @param timestamp   Contagem de tick do ciclo em que a amostra foi
     *                    coletada.
     * @param pedal       Posição do pedal (%).
     * @param torque      Torque calculado para o ciclo.
     * @param temperature Temperatura observada (motor ou bateria).
     */
    void logSample(unsigned long timestamp,
                   double pedal,
                   double torque,
                   double temperature);

    /**
     * @brief Registra a troca do modo de torque ativo.
     *
     * @param timestamp Tick em que a troca ocorreu.
     * @param modeLabel Rótulo legível do novo modo (`"Eco"`, `"MidTerm"`,
     *                  `"Sport"`).
     */
    void logModeChange(unsigned long timestamp, const std::string& modeLabel);

    /**
     * @brief Registra um alerta não-crítico (ex.: temperatura em 60 °C).
     *
     * @param timestamp Tick do alerta.
     * @param message   Mensagem descritiva do alerta.
     */
    void logWarning(unsigned long timestamp, const std::string& message);

    /**
     * @brief Registra um evento de erro crítico (subtensão,
     *        sobretemperatura, abertura do *Shutdown*, timeout de
     *        pré-carga).
     *
     * @param timestamp Tick do evento.
     * @param reason    Mensagem descritiva do erro.
     */
    void logError(unsigned long timestamp, const std::string& reason);

    /**
     * @brief Força o *flush* do stream para o disco.
     *
     * Utilizado como garantia extra de salvamento em pontos onde não há
     * fechamento do arquivo (ex.: antes de uma mudança de estado).
     */
    void flush() noexcept;

    /**
     * @brief Caminho do arquivo de saída configurado.
     * @return Referência constante ao caminho fornecido na construção.
     */
    const std::string& outputPath() const noexcept;

    /**
     * @brief Número de registros gravados desde `open()`.
     * @return Contador acumulado de linhas gravadas.
     */
    unsigned long recordCount() const noexcept;

private:
    std::string m_outputPath;
    std::ofstream m_stream;
    unsigned long m_recordCount;
};

} // namespace io
} // namespace ecu

#endif // ECU_IO_DATALOGGER_HPP
