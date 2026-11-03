#include "core/ECUController.hpp"

#include <exception>
#include <string>
#include <typeinfo>

#include "control/TorqueMapper.hpp"
#include "exceptions/EcuException.hpp"
#include "io/DataLogger.hpp"
#include "io/SensorManager.hpp"
#include "safety/SafetyMonitor.hpp"
#include "state/FaultState.hpp"
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
      m_currentState(),
      // m_running comeca em `true` para que `stop()` chamado antes de
      // `run()` tenha efeito imediato (contrato: stop solicita encerramento
      // cooperativo na proxima iteracao).
      m_running(true) {}

ECUController::~ECUController() {}

void ECUController::initialize() {
    // Fase de boot: entra em Idle sem passar por canTransitionTo (ainda nao
    // ha estado anterior). Qualquer excecao em onEnter propaga para o
    // chamador (conforme contrato do header).
    std::unique_ptr<ecu::state::VehicleState> idle(new ecu::state::IdleState());
    if (m_currentState) {
        m_currentState->onExit(*this);
    }
    m_currentState = std::move(idle);
    m_currentState->onEnter(*this);
}

void ECUController::tick() {
    // Latching: Fault e terminal. Qualquer tick posterior e no-op para
    // garantir que o veiculo nao volte sozinho para Idle/Drive mesmo que a
    // leitura seguinte seja normal.
    if (m_currentState &&
        typeid(*m_currentState) == typeid(ecu::state::FaultState)) {
        return;
    }

    try {
        if (!m_sensors.advance()) {
            // Sem mais dados: encerra o loop cooperativamente.
            m_running = false;
            return;
        }

        m_safety.check();

        // Processa pedal apenas se o estado corrente aceita; caso contrario,
        // registra amostra com torque zero para rastreio da telemetria.
        double torqueRequisitado = 0.0;
        if (m_currentState && m_currentState->acceptsThrottleInput()) {
            torqueRequisitado = m_torque.compute(m_sensors.pedal());
        }

        if (m_logger.isOpen()) {
            m_logger.logSample(m_sensors.currentTick(),
                               m_sensors.pedal(),
                               torqueRequisitado,
                               m_sensors.engineTemperature());
        }

        // Warning termico sinalizado pelo SafetyMonitor: registra alerta
        // sem interromper o ciclo.
        if (m_safety.hasThermalWarning() && m_logger.isOpen()) {
            m_logger.logWarning(m_sensors.currentTick(),
                                "temperatura em faixa de alerta");
        }
    } catch (const ecu::exceptions::EcuException& e) {
        forceFault(std::string(e.what()));
    } catch (const std::exception& e) {
        forceFault(std::string(e.what()));
    } catch (...) {
        forceFault("Erro desconhecido no tick");
    }
}

void ECUController::run() {
    while (m_running) {
        tick();
    }
}

void ECUController::stop() noexcept {
    m_running = false;
}

void ECUController::transitionTo(std::unique_ptr<ecu::state::VehicleState> newState) {
    if (!newState) {
        return;
    }
    if (m_currentState) {
        // Respeita o veto do estado atual: se a transicao nao for
        // autorizada, o estado proposto e descartado.
        if (!m_currentState->canTransitionTo(*newState)) {
            return;
        }
        m_currentState->onExit(*this);
    }
    m_currentState = std::move(newState);
    m_currentState->onEnter(*this);
}

const ecu::state::VehicleState& ECUController::currentState() const {
    return *m_currentState;
}

void ECUController::forceFault(const std::string& reason) {
    // Emergencia: ignora canTransitionTo (nenhum estado autoriza saida para
    // Fault por conta propria sob certos cenarios). Mantem onExit/onEnter
    // para que a FSM continue coerente.
    if (m_currentState) {
        m_currentState->onExit(*this);
    }
    m_currentState.reset(new ecu::state::FaultState(reason));
    m_currentState->onEnter(*this);
}

ecu::control::TorqueMapper& ECUController::torqueMapper() const noexcept {
    return m_torque;
}

ecu::io::DataLogger& ECUController::dataLogger() const noexcept {
    return m_logger;
}

ecu::safety::SafetyMonitor& ECUController::safetyMonitor() const noexcept {
    return m_safety;
}

ecu::io::SensorManager& ECUController::sensorManager() const noexcept {
    return m_sensors;
}

} // namespace core
} // namespace ecu
