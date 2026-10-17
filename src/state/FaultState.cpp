#include "state/FaultState.hpp"

namespace ecu {
namespace state {

FaultState::FaultState(const std::string& reason)
    : m_reason(reason) {
    // TODO C8
}

FaultState::~FaultState() noexcept {
    // TODO C8
}

void FaultState::onEnter(ecu::core::ECUController& /*controller*/) {
    // TODO C8
}

void FaultState::onExit(ecu::core::ECUController& /*controller*/) {
    // TODO C8
}

bool FaultState::acceptsThrottleInput() const noexcept {
    // TODO C8
    return false;
}

bool FaultState::acceptsTorqueMapSwitch() const noexcept {
    // TODO C8
    return false;
}

bool FaultState::canTransitionTo(const VehicleState& /*target*/) const noexcept {
    // TODO C8
    return false;
}

const char* FaultState::name() const noexcept {
    // TODO C8
    return "Fault";
}

const std::string& FaultState::reason() const noexcept {
    // TODO C8
    return m_reason;
}

} // namespace state
} // namespace ecu
