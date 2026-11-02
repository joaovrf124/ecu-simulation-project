#include "io/DataLogger.hpp"

#include "exceptions/TelemetryParseError.hpp"

namespace ecu {
namespace io {

DataLogger::DataLogger(const std::string& outputPath)
    : m_outputPath(outputPath),
      m_stream(),
      m_recordCount(0) {}

DataLogger::~DataLogger() noexcept {
    // Garante flush fisico no disco mesmo que o chamador tenha omitido o
    // close() apos uma excecao.
    if (m_stream.is_open()) {
        m_stream.flush();
        m_stream.close();
    }
}

void DataLogger::open() {
    // Abre em modo truncar (default do std::ofstream): cada corrida
    // comeca com um log limpo.
    m_stream.open(m_outputPath.c_str());
    if (!m_stream.is_open()) {
        // Nao existe uma excecao especifica de I/O na hierarquia; usamos
        // TelemetryParseError (unica derivada de EcuException ligada a
        // persistencia textual) para honrar o contrato "@throws EcuException".
        throw ecu::exceptions::TelemetryParseError(
            "Nao foi possivel abrir arquivo de log: " + m_outputPath);
    }
    m_recordCount = 0;
}

void DataLogger::close() noexcept {
    if (m_stream.is_open()) {
        m_stream.flush();
        m_stream.close();
    }
}

bool DataLogger::isOpen() const noexcept {
    return m_stream.is_open();
}

void DataLogger::logSample(unsigned long timestamp,
                           double pedal,
                           double torque,
                           double temperature) {
    if (!m_stream.is_open()) {
        return;
    }
    m_stream << timestamp << ",SAMPLE,"
             << pedal << ',' << torque << ',' << temperature << '\n';
    ++m_recordCount;
}

void DataLogger::logModeChange(unsigned long timestamp,
                               const std::string& modeLabel) {
    if (!m_stream.is_open()) {
        return;
    }
    m_stream << timestamp << ",MODE," << modeLabel << '\n';
    ++m_recordCount;
}

void DataLogger::logWarning(unsigned long timestamp,
                            const std::string& message) {
    if (!m_stream.is_open()) {
        return;
    }
    m_stream << timestamp << ",WARN," << message << '\n';
    ++m_recordCount;
}

void DataLogger::logError(unsigned long timestamp,
                          const std::string& reason) {
    if (!m_stream.is_open()) {
        return;
    }
    // Flush imediato: eventos criticos precisam estar em disco mesmo que o
    // controlador quebre no proximo tick (US07 - CA3).
    m_stream << timestamp << ",ERROR," << reason << '\n';
    m_stream.flush();
    ++m_recordCount;
}

void DataLogger::flush() noexcept {
    if (m_stream.is_open()) {
        m_stream.flush();
    }
}

const std::string& DataLogger::outputPath() const noexcept {
    return m_outputPath;
}

unsigned long DataLogger::recordCount() const noexcept {
    return m_recordCount;
}

} // namespace io
} // namespace ecu
