#ifndef ECU_EXCEPTIONS_ECUEXCEPTION_HPP
#define ECU_EXCEPTIONS_ECUEXCEPTION_HPP

#include <exception>
#include <string>

namespace ecu {
namespace exceptions {

/**
 * @brief Classe base abstrata para toda falha específica do simulador de ECU.
 *
 * Deriva de `std::exception` e é a raiz da hierarquia utilizada por
 * `SensorManager`, `SafetyMonitor` e `ECUController`. O construtor é
 * protegido e o destrutor é virtual puro (definido inline), de modo que a
 * classe não pode ser instanciada diretamente; apenas suas derivadas
 * concretas (`TelemetryParseError`, `UndervoltageFault`,
 * `OvertemperatureFault`, `ShutdownOpenFault`, `PrechargeTimeoutFault`)
 * podem ser lançadas.
 *
 * A mensagem descritiva é armazenada internamente e exposta através do
 * método `what()`, cumprindo o contrato de `std::exception`.
 */
class EcuException : public std::exception {
public:
    /**
     * @brief Destrutor virtual puro; tornar a classe formalmente abstrata.
     */
    virtual ~EcuException() noexcept = 0;

    /**
     * @brief Retorna a mensagem descritiva da falha em C-string.
     * @return Ponteiro para uma C-string terminada em nulo, válida enquanto
     *         a instância da exceção existir.
     */
    const char* what() const noexcept override;

protected:
    /**
     * @brief Construtor protegido usado pelas derivadas para registrar a
     *        mensagem.
     * @param message Texto que descreve a falha específica.
     */
    explicit EcuException(const std::string& message);

private:
    std::string m_message;
};

inline EcuException::~EcuException() noexcept {}

inline EcuException::EcuException(const std::string& message)
    : m_message(message) {}

inline const char* EcuException::what() const noexcept {
    return m_message.c_str();
}

} // namespace exceptions
} // namespace ecu

#endif // ECU_EXCEPTIONS_ECUEXCEPTION_HPP
