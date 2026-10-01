#ifndef ECU_EXCEPTIONS_PRECHARGETIMEOUTFAULT_HPP
#define ECU_EXCEPTIONS_PRECHARGETIMEOUTFAULT_HPP

#include <string>

#include "exceptions/EcuException.hpp"

namespace ecu {
namespace exceptions {

/**
 * @brief Exceção lançada pelo `SafetyMonitor` quando a sequência de
 *        pré-carga estoura o *timeout* configurado.
 *
 * Bloqueia a transição `IdleState` → `DriveState`; o motivo deve ser
 * registrado pelo `DataLogger` para análise posterior.
 */
class PrechargeTimeoutFault : public EcuException {
public:
    /**
     * @brief Constrói a falha com uma mensagem descritiva opcional (por
     *        exemplo, o tempo decorrido até o estouro).
     * @param message Detalhe adicional; se vazio, uma mensagem padrão é
     *                usada.
     */
    explicit PrechargeTimeoutFault(const std::string& message =
        "Timeout da sequencia de pre-carga");
};

inline PrechargeTimeoutFault::PrechargeTimeoutFault(const std::string& message)
    : EcuException(message) {}

} // namespace exceptions
} // namespace ecu

#endif // ECU_EXCEPTIONS_PRECHARGETIMEOUTFAULT_HPP
