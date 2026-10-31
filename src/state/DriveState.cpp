#include "state/DriveState.hpp"

#include <typeinfo>

#include "state/FaultState.hpp"
#include "state/IdleState.hpp"

namespace ecu {
namespace state {

DriveState::DriveState() noexcept {}

DriveState::~DriveState() noexcept {}

void DriveState::onEnter(ecu::core::ECUController& /*controller*/) {
    // Entrada em tracao: processamento do pedal e troca de mapas ficam
    // habilitados pelos predicados abaixo; nao ha configuracao extra aqui.
}

void DriveState::onExit(ecu::core::ECUController& /*controller*/) {
    // Sem recursos a liberar.
}

bool DriveState::acceptsThrottleInput() const noexcept {
    return true;
}

bool DriveState::acceptsTorqueMapSwitch() const noexcept {
    return true;
}

bool DriveState::canTransitionTo(const VehicleState& target) const noexcept {
    // Drive pode voltar para Idle (encerramento normal) ou cair em Fault
    // (falha critica). Transicao para outro Drive e proibida.
    return typeid(target) == typeid(FaultState)
        || typeid(target) == typeid(IdleState);
}

const char* DriveState::name() const noexcept {
    return "Drive";
}

} // namespace state
} // namespace ecu
