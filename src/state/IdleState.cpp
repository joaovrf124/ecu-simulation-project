#include "state/IdleState.hpp"

namespace ecu {
namespace state {

IdleState::IdleState() noexcept {
    // TODO C8
}

IdleState::~IdleState() noexcept {
    // TODO C8
}

void IdleState::onEnter(ecu::core::ECUController& /*controller*/) {
    // TODO C8
}

void IdleState::onExit(ecu::core::ECUController& /*controller*/) {
    // TODO C8
}

bool IdleState::acceptsThrottleInput() const noexcept {
    // TODO C8
    return false;
}

bool IdleState::acceptsTorqueMapSwitch() const noexcept {
    // TODO C8
    return false;
}

bool IdleState::canTransitionTo(const VehicleState& /*target*/) const noexcept {
    // TODO C8
    return false;
}

const char* IdleState::name() const noexcept {
    // TODO C8
    return "Idle";
}

} // namespace state
} // namespace ecu
