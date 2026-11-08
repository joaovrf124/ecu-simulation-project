#ifndef ECU_CONTROL_TRACTIONCONTROL_HPP
#define ECU_CONTROL_TRACTIONCONTROL_HPP

namespace ecu {
namespace control {

class TractionControl {
public:
    TractionControl(double kp, double ki, double kd, double targetSlip);
    ~TractionControl() = default;

    double computeScaleFactor(double frontSpeed, double rearSpeed, double dt);
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