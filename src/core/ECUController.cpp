#include "core/ECUController.hpp"

#include "control/TorqueMapper.hpp"
#include "io/DataLogger.hpp"
#include "io/SensorManager.hpp"
#include "safety/SafetyMonitor.hpp"
#include "state/IdleState.hpp"
#include "state/VehicleState.hpp"

namespace ecu {
namespace core {

ECUController::ECUController(ecu::io::SensorManager& sensors,
                             ecu::safety::SafetyMonitor& safety,
                             ecu::control::TorqueMapper& torque,
                             ecu::io::DataLogger& logger)
    : m_sensors(sensors),
      m_safety(safety),
      m_torque(torque),
      m_logger(logger),
      // Esqueleto da fase vermelha: um IdleState default permite que
      // `currentState()` nao quebre em testes antes do C8.
      m_currentState(new ecu::state::IdleState()),
      m_running(false) {
    // TODO C8
}

ECUController::~ECUController() {
    // TODO C8
}

void ECUController::initialize() {
    // TODO C8
}

void ECUController::tick() {
    // TODO C8
}

void ECUController::run() {
    // TODO C8
}

void ECUController::stop() noexcept {
    // TODO C8
    m_running = false;
}

void ECUController::transitionTo(std::unique_ptr<ecu::state::VehicleState> newState) {
    // TODO C8
    (void)newState;
}

const ecu::state::VehicleState& ECUController::currentState() const {
    // TODO C8 — fase vermelha: dereferenciar unique_ptr nao inicializado quebraria;
    // os testes que dependem disso vao falhar como esperado.
    return *m_currentState;
}

void ECUController::forceFault(const std::string& reason) {
    // TODO C8
    (void)reason;
}

ecu::control::TorqueMapper& ECUController::torqueMapper() const noexcept {
    // TODO C8
    return m_torque;
}

ecu::io::DataLogger& ECUController::dataLogger() const noexcept {
    // TODO C8
    return m_logger;
}

ecu::safety::SafetyMonitor& ECUController::safetyMonitor() const noexcept {
    // TODO C8
    return m_safety;
}

ecu::io::SensorManager& ECUController::sensorManager() const noexcept {
    // TODO C8
    return m_sensors;
}

} // namespace core
} // namespace ecu
