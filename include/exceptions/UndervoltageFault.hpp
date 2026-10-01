#ifndef ECU_EXCEPTIONS_UNDERVOLTAGEFAULT_HPP
#define ECU_EXCEPTIONS_UNDERVOLTAGEFAULT_HPP

#include <string>

#include "exceptions/EcuException.hpp"

namespace ecu {
namespace exceptions {

/**
 * @brief Exceção lançada pelo `SafetyMonitor` quando a tensão geral cai
 *        abaixo do limite de 60 V.
 *
 * Sinaliza subtensão de bateria; o `ECUController` deve reagir forçando a
 * transição para `FaultState` e o `DataLogger` deve registrar o evento
 * como erro crítico.
 */
class UndervoltageFault : public EcuException {
public:
    /**
     * @brief Constrói a falha com uma mensagem descritiva opcional (por
     *        exemplo, o valor medido).
     * @param message Detalhe adicional; se vazio, uma mensagem padrão é
     *                usada.
     */
    explicit UndervoltageFault(const std::string& message =
        "Tensao da bateria abaixo do limite de 60V");
};

inline UndervoltageFault::UndervoltageFault(const std::string& message)
    : EcuException(message) {}

} // namespace exceptions
} // namespace ecu

#endif // ECU_EXCEPTIONS_UNDERVOLTAGEFAULT_HPP
