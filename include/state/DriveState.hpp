#ifndef ECU_STATE_DRIVESTATE_HPP
#define ECU_STATE_DRIVESTATE_HPP

#include "state/VehicleState.hpp"

namespace ecu {
namespace state {

/**
 * @brief Estado de tração ativa do veículo.
 *
 * Enquanto ativo:
 *  - encaminha o input do pedal ao `TorqueMapper`;
 *  - permite a troca dinâmica entre os mapas Eco, MidTerm e Sport;
 *  - transita para `FaultState` sob qualquer sinalização crítica do
 *    `SafetyMonitor` (subtensão, sobretemperatura, abertura do Shutdown
 *    System, timeout de pré-carga).
 */
class DriveState : public VehicleState {
public:
    /**
     * @brief Constrói um `DriveState`.
     */
    DriveState() noexcept;

    /**
     * @brief Destrutor.
     */
    ~DriveState() noexcept override;

    /**
     * @brief Rotina de entrada: habilita processamento de pedal e mapa
     *        vigente.
     * @param controller Controlador que está ativando este estado.
     */
    void onEnter(ecu::core::ECUController& controller) override;

    /**
     * @brief Rotina de saída: registra o motivo da saída de tração.
     * @param controller Controlador que está trocando para outro estado.
     */
    void onExit(ecu::core::ECUController& controller) override;

    /**
     * @brief No estado `Drive`, o pedal é sempre processado.
     * @return Sempre `true`.
     */
    bool acceptsThrottleInput() const noexcept override;

    /**
     * @brief No estado `Drive`, a troca de mapa é sempre autorizada.
     * @return Sempre `true`.
     */
    bool acceptsTorqueMapSwitch() const noexcept override;

    /**
     * @brief Autoriza transição para `FaultState` (falha crítica) ou de
     *        volta a `IdleState` (encerramento normal).
     * @param target Estado candidato.
     * @return `true` se `target` é `FaultState` ou `IdleState`; `false`
     *         caso contrário.
     */
    bool canTransitionTo(const VehicleState& target) const noexcept override;

    /**
     * @brief Rótulo do estado.
     * @return C-string `"Drive"`.
     */
    const char* name() const noexcept override;
};

} // namespace state
} // namespace ecu

#endif // ECU_STATE_DRIVESTATE_HPP
