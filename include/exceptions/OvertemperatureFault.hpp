#ifndef ECU_EXCEPTIONS_OVERTEMPERATUREFAULT_HPP
#define ECU_EXCEPTIONS_OVERTEMPERATUREFAULT_HPP

#include <string>

#include "exceptions/EcuException.hpp"

namespace ecu {
namespace exceptions {

/**
 * @brief Exceção lançada pelo `SafetyMonitor` quando a temperatura de
 *        motor ou bateria atinge o limite crítico de 80 °C.
 *
 * Configura falha crítica térmica; deve levar o veículo a `FaultState` e
 * ser gravada pelo `DataLogger`. O aviso intermediário de 60 °C não usa
 * esta exceção — ele é apenas um *warning*.
 */
class OvertemperatureFault : public EcuException {
public:
    /**
     * @brief Constrói a falha com uma mensagem descritiva opcional (por
     *        exemplo, indicando qual componente ultrapassou o limite).
     * @param message Detalhe adicional; se vazio, uma mensagem padrão é
     *                usada.
     */
    explicit OvertemperatureFault(const std::string& message =
        "Temperatura critica (>= 80 C) em motor ou bateria");
};

inline OvertemperatureFault::OvertemperatureFault(const std::string& message)
    : EcuException(message) {}

} // namespace exceptions
} // namespace ecu

#endif // ECU_EXCEPTIONS_OVERTEMPERATUREFAULT_HPP
