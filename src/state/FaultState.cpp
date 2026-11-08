#include "state/FaultState.hpp"

#include "control/TorqueMapper.hpp"
#include "core/ECUController.hpp"
#include "io/DataLogger.hpp"
#include "io/SensorManager.hpp"

namespace ecu {
namespace state {

FaultState::FaultState(const std::string& reason)
    : m_reason(reason) {}

FaultState::~FaultState() noexcept {}

void FaultState::onEnter(ecu::core::ECUController& controller) {
    // Zera a rampa do mapeador para interromper qualquer requisicao em curso
    // e grava o motivo da falha com o tick corrente da simulacao. O
    // DataLogger so e usado se ja estiver aberto; caso contrario, apenas
    // ignora silenciosamente para nao mascarar outra excecao em cadeia.
    controller.torqueMapper().reset();
    if (controller.dataLogger().isOpen()) {
        const unsigned long t = controller.sensorManager().currentTick();
        controller.dataLogger().logError(t, m_reason);
    }
}

void FaultState::onExit(ecu::core::ECUController& /*controller*/) {
    // Estado terminal: nunca sai, nada a fazer.
}

bool FaultState::acceptsThrottleInput() const noexcept {
    return false;
}

bool FaultState::acceptsTorqueMapSwitch() const noexcept {
    return false;
}

bool FaultState::canTransitionTo(const VehicleState& /*target*/) const noexcept {
    // Terminal: nenhuma saida autorizada.
    return false;
}

const char* FaultState::name() const noexcept {
    return "Fault";
}

const std::string& FaultState::reason() const noexcept {
    return m_reason;
}

} // namespace state
} // namespace ecu
