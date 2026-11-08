#ifndef ECU_CONTROL_TRACTIONCONTROL_HPP
#define ECU_CONTROL_TRACTIONCONTROL_HPP

namespace ecu {
namespace control {

class TractionControl {
public:
    /**
     * @brief Inicializa o controlador PID do Controle de Tração.
     * @param kp Ganho Proporcional (reage ao erro atual).
     * @param ki Ganho Integral (acumula erros passados).
     * @param kd Ganho Derivativo (reage à taxa de variação do erro).
     * @param targetSlip Escorregamento máximo tolerado em % (ex: 15.0%).
     */
    TractionControl(double kp, double ki, double kd, double targetSlip);
    ~TractionControl() = default;

    /**
     * @brief Calcula a redução de torque baseada na perda de tração.
     * @param frontSpeed Velocidade da roda livre (dianteira) em km/h ou RPM.
     * @param rearSpeed Velocidade da roda de tração (traseira).
     * @param dt Delta de tempo desde a última chamada (em segundos).
     * @return Um fator de escala entre 0.0 (corte total) e 1.0 (sem corte).
     */
    double computeScaleFactor(double frontSpeed, double rearSpeed, double dt);

    /**
     * @brief Zera a memória do PID (usado quando o carro recupera a tração).
     */
    void reset();

private:
    double m_kp;
    double m_ki;
    double m_kd;
    double m_targetSlip;
    double m_integral;
    double m_prevError;
};

} // namespace control
} // namespace ecu

#endif // ECU_CONTROL_TRACTIONCONTROL_HPP