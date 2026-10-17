#include "safety/SafetyMonitor.hpp"

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
      m_prechargeTicks(0) {
    // TODO C8
}

SafetyMonitor::~SafetyMonitor() noexcept {
    // TODO C8
}

void SafetyMonitor::check() {
    // TODO C8
}

void SafetyMonitor::tickPrecharge() {
    // TODO C8
}

void SafetyMonitor::markPrechargeComplete() noexcept {
    // TODO C8
    m_prechargeComplete = true;
}

bool SafetyMonitor::isShutdownOpen() const noexcept {
    // TODO C8
    return m_shutdownOpen;
}

void SafetyMonitor::setShutdownOpen(bool open) noexcept {
    // TODO C8
    m_shutdownOpen = open;
}

bool SafetyMonitor::hasThermalWarning() const noexcept {
    // TODO C8
    return m_thermalWarning;
}

bool SafetyMonitor::isPrechargeComplete() const noexcept {
    // TODO C8
    return m_prechargeComplete;
}

unsigned long SafetyMonitor::prechargeTicks() const noexcept {
    // TODO C8
    return m_prechargeTicks;
}

} // namespace safety
} // namespace ecu
