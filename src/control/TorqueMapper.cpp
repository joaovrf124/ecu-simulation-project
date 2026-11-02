#include "control/TorqueMapper.hpp"

#include <cstddef>

namespace ecu {
namespace control {

// Constantes definidas aqui para que a linkagem seja unica (requisito C++11).
const double TorqueMapper::kMaxTorque      = 250.0;
const double TorqueMapper::kMaxTorqueDelta = 20.0;

namespace {

// Interpolacao linear sobre a tabela ordenada por `pedal` crescente.
// A funcao assume que o chamador ja fez o clamp de `pedal` em [0, 100] e
// que a tabela cobre pelo menos um ponto (vetor nao vazio).
double interpolateTable(const std::vector<TorquePoint>& table, double pedal) {
    if (pedal <= table.front().pedal) {
        return table.front().torque;
    }
    if (pedal >= table.back().pedal) {
        return table.back().torque;
    }
    for (std::size_t i = 1; i < table.size(); ++i) {
        if (pedal <= table[i].pedal) {
            const TorquePoint& p0 = table[i - 1];
            const TorquePoint& p1 = table[i];
            const double range = p1.pedal - p0.pedal;
            if (range <= 0.0) {
                return p0.torque;
            }
            const double t = (pedal - p0.pedal) / range;
            return p0.torque + t * (p1.torque - p0.torque);
        }
    }
    return table.back().torque;
}

} // namespace

TorqueMapper::TorqueMapper()
    : m_mode(TorqueMode::Eco),
      m_tableEco(),
      m_tableMid(),
      m_tableSport(),
      m_lastTorque(0.0),
      m_hasPrevious(false) {}

TorqueMapper::~TorqueMapper() noexcept {}

void TorqueMapper::setMapTable(TorqueMode mode,
                               const std::vector<TorquePoint>& points) {
    switch (mode) {
        case TorqueMode::Eco:     m_tableEco   = points; break;
        case TorqueMode::MidTerm: m_tableMid   = points; break;
        case TorqueMode::Sport:   m_tableSport = points; break;
    }
}

void TorqueMapper::setMode(TorqueMode mode) noexcept {
    m_mode = mode;
}

TorqueMode TorqueMapper::mode() const noexcept {
    return m_mode;
}

double TorqueMapper::compute(double pedalPercent) {
    // Clamp do pedal: valores fora de [0, 100] sao tratados como o extremo
    // mais proximo, conforme contrato documentado no header.
    if (pedalPercent < 0.0) {
        pedalPercent = 0.0;
    } else if (pedalPercent > 100.0) {
        pedalPercent = 100.0;
    }

    // Seleciona a tabela do modo ativo. Se nao houver mapa carregado, o
    // torque e zero (comportamento defensivo para a fase de bring-up).
    const std::vector<TorquePoint>* table = &m_tableEco;
    switch (m_mode) {
        case TorqueMode::Eco:     table = &m_tableEco;   break;
        case TorqueMode::MidTerm: table = &m_tableMid;   break;
        case TorqueMode::Sport:   table = &m_tableSport; break;
    }
    if (table->empty()) {
        return 0.0;
    }

    double torque = interpolateTable(*table, pedalPercent);

    // Teto de seguranca: nenhum torque devolvido pode exceder kMaxTorque.
    if (torque > kMaxTorque) {
        torque = kMaxTorque;
    } else if (torque < 0.0) {
        torque = 0.0;
    }

    // Rampa mecanica: so se aplica a partir do segundo compute() apos um
    // reset() ou construcao. A primeira amostra passa direto para permitir
    // que o motor assuma o valor exato do mapa quando volta a atuar.
    if (m_hasPrevious) {
        const double delta = torque - m_lastTorque;
        if (delta > kMaxTorqueDelta) {
            torque = m_lastTorque + kMaxTorqueDelta;
        } else if (delta < -kMaxTorqueDelta) {
            torque = m_lastTorque - kMaxTorqueDelta;
        }
    }

    m_lastTorque = torque;
    m_hasPrevious = true;
    return torque;
}

void TorqueMapper::reset() noexcept {
    m_lastTorque = 0.0;
    m_hasPrevious = false;
}

double TorqueMapper::lastTorque() const noexcept {
    return m_lastTorque;
}

const char* TorqueMapper::modeLabel(TorqueMode mode) noexcept {
    switch (mode) {
        case TorqueMode::Eco:     return "Eco";
        case TorqueMode::MidTerm: return "MidTerm";
        case TorqueMode::Sport:   return "Sport";
    }
    return "Eco";
}

} // namespace control
} // namespace ecu
