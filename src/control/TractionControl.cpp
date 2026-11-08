#include "control/TractionControl.hpp"
#include <algorithm> // Necessário para std::min e std::max

namespace ecu {
namespace control {

TractionControl::TractionControl(double kp, double ki, double kd, double targetSlip)
    : m_kp(kp), m_ki(ki), m_kd(kd), m_targetSlip(targetSlip), m_integral(0.0), m_prevError(0.0) {}

double TractionControl::computeScaleFactor(double frontSpeed, double rearSpeed, double dt) {
    if (frontSpeed <= 0.1) return 1.0; // Evita divisão por zero em repouso

    // Calcula a porcentagem de escorregamento da roda traseira em relação à dianteira
    double currentSlip = ((rearSpeed - frontSpeed) / frontSpeed) * 100.0;

    if (currentSlip <= m_targetSlip) {
        reset();
        return 1.0; // Roda com tração normal, sem corte de torque
    }

    double error = currentSlip - m_targetSlip;
    
    m_integral += error * dt;
    m_integral = std::min(m_integral, 50.0); // Limite do integrador (Anti-windup)
    
    double derivative = (error - m_prevError) / dt;
    
    double pidOutput = (m_kp * error) + (m_ki * m_integral) + (m_kd * derivative);
    
    m_prevError = error;

    double scaleFactor = 1.0 - (pidOutput / 100.0);
    
    // Limita o fator de escala entre 10% (corte quase total) e 100% (sem corte)
    return std::max(0.1, std::min(scaleFactor, 1.0));
}

void TractionControl::reset() {
    m_integral = 0.0;
    m_prevError = 0.0;
}

} // namespace control
} // namespace ecu