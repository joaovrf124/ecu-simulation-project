#include "control/TorqueMapper.hpp"

namespace ecu {
namespace control {

// Constantes definidas aqui para que a linkagem seja unica (requisito C++11).
const double TorqueMapper::kMaxTorque      = 250.0;
const double TorqueMapper::kMaxTorqueDelta = 20.0;

TorqueMapper::TorqueMapper()
    : m_mode(TorqueMode::Eco),
      m_tableEco(),
      m_tableMid(),
      m_tableSport(),
      m_lastTorque(0.0),
      m_hasPrevious(false) {
    // TODO C8
}

TorqueMapper::~TorqueMapper() noexcept {
    // TODO C8
}

void TorqueMapper::setMapTable(TorqueMode /*mode*/, const std::vector<TorquePoint>& /*points*/) {
    // TODO C8
}

void TorqueMapper::setMode(TorqueMode mode) noexcept {
    // TODO C8
    m_mode = mode;
}

TorqueMode TorqueMapper::mode() const noexcept {
    // TODO C8
    return m_mode;
}

double TorqueMapper::compute(double /*pedalPercent*/) {
    // TODO C8
    return 0.0;
}

void TorqueMapper::reset() noexcept {
    // TODO C8
    m_lastTorque = 0.0;
    m_hasPrevious = false;
}

double TorqueMapper::lastTorque() const noexcept {
    // TODO C8
    return m_lastTorque;
}

const char* TorqueMapper::modeLabel(TorqueMode mode) noexcept {
    // TODO C8 — rotulo e deterministico, implementado agora para que a
    // documentacao Doxygen e o log de modo nao fiquem vazios na fase vermelha.
    switch (mode) {
        case TorqueMode::Eco:     return "Eco";
        case TorqueMode::MidTerm: return "MidTerm";
        case TorqueMode::Sport:   return "Sport";
    }
    return "Eco";
}

} // namespace control
} // namespace ecu
