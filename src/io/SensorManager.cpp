#include "io/SensorManager.hpp"

namespace ecu {
namespace io {

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
      m_voltage(0.0) {
    // TODO C8
}

SensorManager::~SensorManager() noexcept {
    // TODO C8
}

void SensorManager::open() {
    // TODO C8
}

bool SensorManager::advance() {
    // TODO C8
    return false;
}

bool SensorManager::advanceStrict() {
    // TODO C8
    return false;
}

bool SensorManager::hasNext() const noexcept {
    // TODO C8
    return m_hasNext;
}

double SensorManager::pedal() const noexcept {
    // TODO C8
    return m_pedal;
}

double SensorManager::engineTemperature() const noexcept {
    // TODO C8
    return m_engineTemp;
}

double SensorManager::batteryTemperature() const noexcept {
    // TODO C8
    return m_batteryTemp;
}

double SensorManager::brakePressure() const noexcept {
    // TODO C8
    return m_brakePressure;
}

double SensorManager::voltage() const noexcept {
    // TODO C8
    return m_voltage;
}

unsigned long SensorManager::currentTick() const noexcept {
    // TODO C8
    return m_tick;
}

const std::string& SensorManager::filePath() const noexcept {
    // TODO C8
    return m_filePath;
}

} // namespace io
} // namespace ecu
