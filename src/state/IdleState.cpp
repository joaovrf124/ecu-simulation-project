#include "state/IdleState.hpp"

#include <typeinfo>

#include "state/DriveState.hpp"
#include "state/FaultState.hpp"

namespace ecu {
namespace state {

IdleState::IdleState() noexcept {}

IdleState::~IdleState() noexcept {}

void IdleState::onEnter(ecu::core::ECUController& /*controller*/) {
    // Em Idle o veiculo esta em repouso seguro; a decisao de sair daqui
    // depende do SafetyMonitor (checagem de pre-carga) e e tomada pelo
    // ECUController, nao pelo proprio estado.
}

void IdleState::onExit(ecu::core::ECUController& /*controller*/) {
    // Sem recursos a liberar.
}

bool IdleState::acceptsThrottleInput() const noexcept {
    return false;
}

bool IdleState::acceptsTorqueMapSwitch() const noexcept {
    return false;
}

bool IdleState::canTransitionTo(const VehicleState& target) const noexcept {
    // Comparacao por tipo dinamico: Idle so pode sair para Drive (operacao
    // normal) ou Fault (falha critica). Qualquer outro alvo e proibido.
    return typeid(target) == typeid(DriveState)
        || typeid(target) == typeid(FaultState);
}

const char* IdleState::name() const noexcept {
    return "Idle";
}

} // namespace state
} // namespace ecu
