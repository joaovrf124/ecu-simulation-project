#ifndef ECU_STATE_IDLESTATE_HPP
#define ECU_STATE_IDLESTATE_HPP

#include "state/VehicleState.hpp"

namespace ecu {
namespace state {

/**
 * @brief Estado inicial e de repouso do veículo (pronto sem tração).
 *
 * Enquanto ativo:
 *  - ignora completamente o input do pedal do acelerador;
 *  - bloqueia a troca dos mapas Eco/MidTerm/Sport;
 *  - autoriza a transição para `DriveState` somente quando o
 *    `SafetyMonitor` confirmar que a sequência de pré-carga foi concluída
 *    dentro do *timeout* configurado.
 */
class IdleState : public VehicleState {
public:
    /**
     * @brief Constrói um `IdleState`.
     */
    IdleState() noexcept;

    /**
     * @brief Destrutor.
     */
    ~IdleState() noexcept override;

    /**
     * @brief Rotina de entrada: prepara o veículo para o repouso seguro.
     * @param controller Controlador que está ativando este estado.
     */
    void onEnter(ecu::core::ECUController& controller) override;

    /**
     * @brief Rotina de saída: registra a saída do repouso.
     * @param controller Controlador que está trocando para outro estado.
     */
    void onExit(ecu::core::ECUController& controller) override;

    /**
     * @brief No estado `Idle`, o pedal é sempre ignorado.
     * @return Sempre `false`.
     */
    bool acceptsThrottleInput() const noexcept override;

    /**
     * @brief No estado `Idle`, a troca de mapa é sempre bloqueada.
     * @return Sempre `false`.
     */
    bool acceptsTorqueMapSwitch() const noexcept override;

    /**
     * @brief Autoriza transição apenas para `DriveState` (após pré-carga
     *        confirmada) ou para `FaultState` (falha crítica).
     * @param target Estado candidato.
     * @return `true` se `target` é `DriveState` ou `FaultState`; `false`
     *         caso contrário.
     */
    bool canTransitionTo(const VehicleState& target) const noexcept override;

    /**
     * @brief Rótulo do estado.
     * @return C-string `"Idle"`.
     */
    const char* name() const noexcept override;
};

} // namespace state
} // namespace ecu

#endif // ECU_STATE_IDLESTATE_HPP
