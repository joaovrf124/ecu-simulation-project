#ifndef ECU_EXCEPTIONS_TELEMETRYPARSEERROR_HPP
#define ECU_EXCEPTIONS_TELEMETRYPARSEERROR_HPP

#include <string>

#include "exceptions/EcuException.hpp"

namespace ecu {
namespace exceptions {

/**
 * @brief Exceção lançada pelo `SensorManager` quando uma linha do CSV de
 *        telemetria não pôde ser interpretada.
 *
 * A simulação continua avançando (o `SensorManager` pula a linha inválida);
 * esta exceção existe para que camadas consumidoras possam reagir à linha
 * mal formatada quando desejarem, sem interromper a leitura da corrida.
 */
class TelemetryParseError : public EcuException {
public:
    /**
     * @brief Constrói o erro registrando o motivo/contexto do fracasso de
     *        parse.
     * @param message Texto descritivo (por exemplo, nº da linha e conteúdo
     *                bruto rejeitado).
     */
    explicit TelemetryParseError(const std::string& message);
};

inline TelemetryParseError::TelemetryParseError(const std::string& message)
    : EcuException(message) {}

} // namespace exceptions
} // namespace ecu

#endif // ECU_EXCEPTIONS_TELEMETRYPARSEERROR_HPP
