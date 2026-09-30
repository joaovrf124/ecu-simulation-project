#ifndef ECU_STATE_FAULTSTATE_HPP
#define ECU_STATE_FAULTSTATE_HPP

#include <string>

#include "state/VehicleState.hpp"

namespace ecu {
namespace state {

/**
 * @brief Estado terminal de falha do veículo.
 *
 * Enquanto ativo:
 *  - ignora por completo o input do pedal do acelerador;
 *  - força a requisição de torque enviada ao `TorqueMapper` para zero;
 *  - mantém registro contínuo do motivo da falha via `DataLogger` até o
 *    encerramento da simulação.
 *
 * Uma vez em `FaultState`, o veículo não retorna a estados operacionais
 * dentro da mesma execução — o único movimento permitido é permanecer no
 * próprio estado.
 */
class FaultState : public VehicleState {
public:
    /**
     * @brief Constrói um `FaultState` a partir do motivo que provocou a
     *        entrada em falha.
     * @param reason Descrição textual da falha (ex.: mensagem da exceção
     *               capturada pelo `ECUController`).
     */
    explicit FaultState(const std::string& reason);

    /**
     * @brief Destrutor.
     */
    ~FaultState() noexcept override;

    /**
     * @brief Rotina de entrada: zera a requisição de torque e registra o
     *        evento no `DataLogger`.
     * @param controller Controlador que está ativando este estado.
     */
    void onEnter(ecu::core::ECUController& controller) override;

    /**
     * @brief Rotina de saída — na prática nunca é chamada, pois o estado
     *        é terminal, mas mantém a assinatura completa da interface.
     * @param controller Controlador (ignorado).
     */
    void onExit(ecu::core::ECUController& controller) override;

    /**
     * @brief No estado `Fault`, o pedal é sempre ignorado.
     * @return Sempre `false`.
     */
    bool acceptsThrottleInput() const noexcept override;

    /**
     * @brief No estado `Fault`, a troca de mapa é sempre bloqueada.
     * @return Sempre `false`.
     */
    bool acceptsTorqueMapSwitch() const noexcept override;

    /**
     * @brief `FaultState` não autoriza nenhuma transição de saída.
     * @param target Estado candidato (ignorado).
     * @return Sempre `false`.
     */
    bool canTransitionTo(const VehicleState& target) const noexcept override;

    /**
     * @brief Rótulo do estado.
     * @return C-string `"Fault"`.
     */
    const char* name() const noexcept override;

    /**
     * @brief Descrição da falha que originou a entrada neste estado.
     * @return Referência constante à mensagem gravada no construtor.
     */
    const std::string& reason() const noexcept;

private:
    std::string m_reason;
};

} // namespace state
} // namespace ecu

#endif // ECU_STATE_FAULTSTATE_HPP
