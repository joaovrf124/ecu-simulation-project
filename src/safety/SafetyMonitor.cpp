#include "safety/SafetyMonitor.hpp"

#include "exceptions/OvertemperatureFault.hpp"
#include "exceptions/PrechargeTimeoutFault.hpp"
#include "exceptions/ShutdownOpenFault.hpp"
#include "exceptions/UndervoltageFault.hpp"
#include "io/SensorManager.hpp"

namespace ecu {
namespace safety {

// Constantes dos limites exigidos pelas user stories.
const double        SafetyMonitor::kUndervoltageThresholdVolts = 60.0;
const double        SafetyMonitor::kTemperatureWarningCelsius  = 60.0;
const double        SafetyMonitor::kTemperatureFaultCelsius    = 80.0;
const unsigned long SafetyMonitor::kPrechargeTimeoutTicks      = 1000;

SafetyMonitor::SafetyMonitor(ecu::io::SensorManager& sensors)
    : m_sensors(sensors),
      m_shutdownOpen(false),
      m_thermalWarning(false),
      m_prechargeComplete(false),
      m_prechargeTicks(0) {}

SafetyMonitor::~SafetyMonitor() noexcept {}

void SafetyMonitor::check() {
    // Ordem das verificacoes vai do sinal mais inegociavel (shutdown fisico
    // aberto) para a leitura analogica (tensao, temperatura). O warning
    // termico so e reavaliado quando nenhuma falha critica foi disparada.
    if (m_shutdownOpen) {
        throw ecu::exceptions::ShutdownOpenFault();
    }

    const double voltage = m_sensors.voltage();
    if (voltage < kUndervoltageThresholdVolts) {
        throw ecu::exceptions::UndervoltageFault();
    }

    const double engineTemp  = m_sensors.engineTemperature();
    const double batteryTemp = m_sensors.batteryTemperature();
    if (engineTemp  >= kTemperatureFaultCelsius ||
        batteryTemp >= kTemperatureFaultCelsius) {
        throw ecu::exceptions::OvertemperatureFault();
    }

    // Warning termico: ativo enquanto a temperatura estiver na faixa
    // [60, 80) C, inativo caso contrario. Fica latched apenas enquanto a
    // condicao persistir — se a temperatura cair, o aviso cessa.
    m_thermalWarning = (engineTemp  >= kTemperatureWarningCelsius)
                    || (batteryTemp >= kTemperatureWarningCelsius);
}

void SafetyMonitor::tickPrecharge() {
    if (m_prechargeComplete) {
        return;
    }
    ++m_prechargeTicks;
    if (m_prechargeTicks > kPrechargeTimeoutTicks) {
        throw ecu::exceptions::PrechargeTimeoutFault();
    }
}

void SafetyMonitor::markPrechargeComplete() noexcept {
    m_prechargeComplete = true;
}

bool SafetyMonitor::isShutdownOpen() const noexcept {
    return m_shutdownOpen;
}

void SafetyMonitor::setShutdownOpen(bool open) noexcept {
    m_shutdownOpen = open;
}

bool SafetyMonitor::hasThermalWarning() const noexcept {
    return m_thermalWarning;
}

bool SafetyMonitor::isPrechargeComplete() const noexcept {
    return m_prechargeComplete;
}

unsigned long SafetyMonitor::prechargeTicks() const noexcept {
    return m_prechargeTicks;
}

} // namespace safety
} // namespace ecu
