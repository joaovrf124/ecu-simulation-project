#include "control/TractionControl.hpp"
#include <algorithm>

namespace ecu {
namespace control {

TractionControl::TractionControl(double kp, double ki, double kd, double targetSlip)
    : m_kp(kp), m_ki(ki), m_kd(kd), m_targetSlip(targetSlip), m_integral(0.0), m_prevError(0.0) {}

double TractionControl::computeScaleFactor(double frontSpeed, double rearSpeed, double dt) {
    // Se o carro está parado ou o tempo for inválido, não há corte de tração
    if (frontSpeed <= 1.0 || dt <= 0.0) {
        reset();
        return 1.0; 
    }

    // Calcula o slip (escorregamento) em porcentagem
    // Ex: Dianteira a 50 km/h, Traseira a 60 km/h -> Slip = 20%
    double currentSlip = ((rearSpeed - frontSpeed) / frontSpeed) * 100.0;

    // Só atua se o escorregamento for maior que o alvo permitido (ex: > 15%)
    if (currentSlip <= m_targetSlip) {
        reset();
        return 1.0; // Libera 100% do torque do TorqueMapper
    }

    // Cálculo do Erro PID
    double error = currentSlip - m_targetSlip;
    
    // Termo Integral (com Anti-windup para não acumular penalidade infinita)
    m_integral += error * dt;
    m_integral = std::min(m_integral, 50.0);

    // Termo Derivativo (evita oscilações bruscas)
    double derivative = (error - m_prevError) / dt;
    m_prevError = error;

    // Saída do PID (representa a penalidade percentual que deve ser aplicada)
    double pidOutput = (m_kp * error) + (m_ki * m_integral) + (m_kd * derivative);

    // Converte a saída num fator multiplicativo para o Torque. 
    // Ex: pidOutput = 20 -> scaleFactor = 0.80 (corta 20% do torque final)
    double scaleFactor = 1.0 - (pidOutput / 100.0);

    // Garante limites de segurança: nunca corta mais que 90% (0.1) e não ultrapassa 100% (1.0)
    return std::max(0.1, std::min(scaleFactor, 1.0));
}

void TractionControl::reset() {
    m_integral = 0.0;
    m_prevError = 0.0;
}

} // namespace control
} // namespace ecu
