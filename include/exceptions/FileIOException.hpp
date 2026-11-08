#ifndef ECU_EXCEPTIONS_FILEIOEXCEPTION_HPP
#define ECU_EXCEPTIONS_FILEIOEXCEPTION_HPP

#include <string>

#include "exceptions/EcuException.hpp"

namespace ecu {
namespace exceptions {

/**
 * @brief Exceção lançada quando uma operação de arquivo (abrir, gravar,
 *        fechar) falha por motivos de I/O — e não por conteúdo mal
 *        formatado.
 *
 * É usada por `SensorManager` ao não conseguir abrir o CSV de entrada,
 * pelo `DataLogger` ao não conseguir abrir ou escrever no arquivo de
 * saída, e por qualquer componente que precise sinalizar que o problema
 * está no acesso ao arquivo (caminho inexistente, sem permissão, disco
 * cheio, stream fechado quando se esperava aberto) em vez de no formato
 * do conteúdo (para esse caso, `TelemetryParseError` continua sendo a
 * exceção adequada).
 *
 * Deriva de `EcuException`; portanto também é `std::exception`, e pode
 * ser capturada em um `catch` genérico ou especificamente antes das
 * demais derivadas no `main` quando a UI quiser dar uma mensagem
 * dedicada para falhas de I/O.
 */
class FileIOException : public EcuException {
public:
    /**
     * @brief Constrói a falha com uma mensagem descritiva.
     * @param message Detalhe do problema (caminho envolvido, operação,
     *                motivo quando disponível).
     */
    explicit FileIOException(const std::string& message =
        "Falha de I/O em arquivo");
};

inline FileIOException::FileIOException(const std::string& message)
    : EcuException(message) {}

} // namespace exceptions
} // namespace ecu

#endif // ECU_EXCEPTIONS_FILEIOEXCEPTION_HPP
