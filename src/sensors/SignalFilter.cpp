#include "sensors/SignalFilter.hpp"
#include <stdexcept>

namespace ecu {
namespace sensors {

SignalFilter::SignalFilter(double alpha)
    : m_alpha(alpha),
      m_lastValue(0.0),
      m_initialized(false) {
    if (alpha <= 0.0 || alpha > 1.0) {
        throw std::invalid_argument("O fator alpha do filtro deve estar no intervalo (0.0, 1.0].");
    }
}

SignalFilter::~SignalFilter() noexcept {
}

double SignalFilter::filter(double rawSample) noexcept {
    if (!m_initialized) {
        m_lastValue = rawSample;
        m_initialized = true;
        return m_lastValue;
    }

    m_lastValue = (m_alpha * rawSample) + ((1.0 - m_alpha) * m_lastValue);
    return m_lastValue;
}

void SignalFilter::reset() noexcept {
    m_lastValue = 0.0;
    m_initialized = false;
}

double SignalFilter::alpha() const noexcept {
    return m_alpha;
}

double SignalFilter::lastValue() const noexcept {
    return m_lastValue;
}

bool SignalFilter::isInitialized() const noexcept {
    return m_initialized;
}

} // namespace sensors
} // namespace ecu