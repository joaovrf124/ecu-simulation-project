#include "io/SensorManager.hpp"

#include <sstream>
#include <string>

#include "exceptions/FileIOException.hpp"
#include "exceptions/TelemetryParseError.hpp"

namespace ecu {
namespace io {

namespace {

// Converte uma linha CSV `pedal,engineTemp,batteryTemp,brakePressure,voltage`
// em cinco doubles. Devolve false se a linha estiver vazia, com numero errado
// de campos, ou se algum campo nao for numerico. Nao altera os parametros em
// caso de falha.
bool parseCsvLine(const std::string& line,
                  double& pedal, double& engineTemp, double& batteryTemp,
                  double& brakePressure, double& voltage) {
    if (line.empty()) {
        return false;
    }
    std::string normalized = line;
    for (std::string::size_type i = 0; i < normalized.size(); ++i) {
        if (normalized[i] == ',') {
            normalized[i] = ' ';
        }
    }
    std::istringstream iss(normalized);
    double a, b, c, d, e;
    if (!(iss >> a >> b >> c >> d >> e)) {
        return false;
    }
    // Rejeita linhas com campos sobrando (ex.: "1,2,3,4,5,6").
    std::string extra;
    if (iss >> extra) {
        return false;
    }
    pedal = a;
    engineTemp = b;
    batteryTemp = c;
    brakePressure = d;
    voltage = e;
    return true;
}

} // namespace

SensorManager::SensorManager(const std::string& filePath)
    : m_filePath(filePath),
      m_stream(),
      m_open(false),
      m_hasNext(false),
      m_tick(0),
      m_pedal(0.0),
      m_engineTemp(0.0),
      m_batteryTemp(0.0),
      m_brakePressure(0.0),
      m_voltage(0.0) {}

SensorManager::~SensorManager() noexcept {
    if (m_stream.is_open()) {
        m_stream.close();
    }
}

void SensorManager::open() {
    m_stream.open(m_filePath.c_str());
    if (!m_stream.is_open()) {
        // Problema de I/O (arquivo nao existe/sem permissao): FileIOException.
        throw ecu::exceptions::FileIOException(
            "Nao foi possivel abrir o CSV de telemetria: " + m_filePath);
    }
    m_open = true;
    // peek() nao consome o caractere; apenas inspeciona se ha conteudo.
    m_hasNext = (m_stream.peek() != EOF);
}

bool SensorManager::advance() {
    if (!m_open) {
        return false;
    }
    std::string line;
    while (std::getline(m_stream, line)) {
        double p, et, bt, bp, v;
        if (parseCsvLine(line, p, et, bt, bp, v)) {
            m_pedal = p;
            m_engineTemp = et;
            m_batteryTemp = bt;
            m_brakePressure = bp;
            m_voltage = v;
            ++m_tick;
            m_hasNext = (m_stream.peek() != EOF);
            return true;
        }
        // Linha invalida: ignora silenciosamente (RNF04) e segue para a
        // proxima.
    }
    m_hasNext = false;
    return false;
}

bool SensorManager::advanceStrict() {
    if (!m_open) {
        return false;
    }
    std::string line;
    if (!std::getline(m_stream, line)) {
        m_hasNext = false;
        return false;
    }
    double p, et, bt, bp, v;
    if (!parseCsvLine(line, p, et, bt, bp, v)) {
        throw ecu::exceptions::TelemetryParseError(
            "Linha de telemetria mal formatada: " + line);
    }
    m_pedal = p;
    m_engineTemp = et;
    m_batteryTemp = bt;
    m_brakePressure = bp;
    m_voltage = v;
    ++m_tick;
    m_hasNext = (m_stream.peek() != EOF);
    return true;
}

bool SensorManager::hasNext() const noexcept {
    return m_hasNext;
}

double SensorManager::pedal() const noexcept {
    return m_pedal;
}

double SensorManager::engineTemperature() const noexcept {
    return m_engineTemp;
}

double SensorManager::batteryTemperature() const noexcept {
    return m_batteryTemp;
}

double SensorManager::brakePressure() const noexcept {
    return m_brakePressure;
}

double SensorManager::voltage() const noexcept {
    return m_voltage;
}

unsigned long SensorManager::currentTick() const noexcept {
    return m_tick;
}

const std::string& SensorManager::filePath() const noexcept {
    return m_filePath;
}

} // namespace io
} // namespace ecu
