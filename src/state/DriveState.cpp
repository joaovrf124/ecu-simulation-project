#include "state/DriveState.hpp"

namespace ecu {
namespace state {

DriveState::DriveState() noexcept {
    // TODO C8
}

DriveState::~DriveState() noexcept {
    // TODO C8
}

void DriveState::onEnter(ecu::core::ECUController& /*controller*/) {
    // TODO C8
}

void DriveState::onExit(ecu::core::ECUController& /*controller*/) {
    // TODO C8
}

bool DriveState::acceptsThrottleInput() const noexcept {
    // TODO C8
    return false;
}

bool DriveState::acceptsTorqueMapSwitch() const noexcept {
    // TODO C8
    return false;
}

bool DriveState::canTransitionTo(const VehicleState& /*target*/) const noexcept {
    // TODO C8
    return false;
}

const char* DriveState::name() const noexcept {
    // TODO C8
    return "Drive";
}

} // namespace state
} // namespace ecu
