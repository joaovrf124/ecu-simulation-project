#ifndef ECU_EXCEPTIONS_SHUTDOWNOPENFAULT_HPP
#define ECU_EXCEPTIONS_SHUTDOWNOPENFAULT_HPP

#include <string>

#include "exceptions/EcuException.hpp"

namespace ecu {
namespace exceptions {

/**
 * @brief Exceção lançada pelo `SafetyMonitor` quando o *Shutdown System*
 *        é detectado aberto.
 *
 * A abertura do circuito de shutdown é condição de segurança inegociável:
 * o `ECUController` deve transitar para `FaultState` no mesmo tick em que
 * a exceção é observada, e o `DataLogger` deve gravá-la como evento
 * crítico.
 */
class ShutdownOpenFault : public EcuException {
public:
    /**
     * @brief Constrói a falha com uma mensagem descritiva opcional.
     * @param message Detalhe adicional; se vazio, uma mensagem padrão é
     *                usada.
     */
    explicit ShutdownOpenFault(const std::string& message =
        "Shutdown System aberto");
};

inline ShutdownOpenFault::ShutdownOpenFault(const std::string& message)
    : EcuException(message) {}

} // namespace exceptions
} // namespace ecu

#endif // ECU_EXCEPTIONS_SHUTDOWNOPENFAULT_HPP
