#ifndef ECU_STATE_VEHICLESTATE_HPP
#define ECU_STATE_VEHICLESTATE_HPP

namespace ecu {
namespace core { class ECUController; }
}

namespace ecu {
namespace state {

/**
 * @brief Interface abstrata dos estados do veículo (Idle, Drive, Fault)
 *        na Máquina de Estados Finitos da ECU.
 *
 * Implementa o papel de "State" no padrão de projeto homônimo: o
 * `ECUController` mantém uma referência polimórfica a `VehicleState` e
 * delega as decisões dependentes de estado (autorização de troca de mapa,
 * processamento de pedal, permissão de transição e rotinas de
 * entrada/saída) para o objeto concreto ativo.
 *
 * Cada estado concreto (`IdleState`, `DriveState`, `FaultState`) implementa
 * as regras específicas do seu contexto operacional.
 */
class VehicleState {
public:
    /**
     * @brief Destrutor virtual — necessário para permitir destruição
     *        polimórfica através de ponteiro para a base.
     */
    virtual ~VehicleState() noexcept;

    /**
     * @brief Rotina executada ao entrar no estado.
     *
     * Chamada pelo `ECUController` no momento da transição, antes de o
     * estado começar a receber ticks. Cada estado usa este ponto para
     * configurar limites, resetar contadores ou emitir registros iniciais.
     *
     * @param controller Referência ao controlador que hospeda o estado.
     */
    virtual void onEnter(ecu::core::ECUController& controller) = 0;

    /**
     * @brief Rotina executada ao sair do estado.
     *
     * Chamada pelo `ECUController` imediatamente antes da substituição do
     * estado ativo, para liberar recursos ou emitir registros de saída.
     *
     * @param controller Referência ao controlador que hospeda o estado.
     */
    virtual void onExit(ecu::core::ECUController& controller) = 0;

    /**
     * @brief Indica se o input do pedal do acelerador deve ser processado
     *        no estado atual.
     * @return `true` se o pedal deve ser encaminhado ao `TorqueMapper`,
     *         `false` se deve ser ignorado.
     */
    virtual bool acceptsThrottleInput() const noexcept = 0;

    /**
     * @brief Indica se a troca dos mapas de motor (Eco/MidTerm/Sport)
     *        pode ocorrer no estado atual.
     * @return `true` se o `TorqueMapper` pode receber um novo mapa,
     *         `false` se a troca deve ser bloqueada.
     */
    virtual bool acceptsTorqueMapSwitch() const noexcept = 0;

    /**
     * @brief Consulta se uma transição para outro estado é permitida a
     *        partir do estado atual.
     * @param target Estado candidato ao qual se pretende transitar.
     * @return `true` se a transição é aceita pelo estado corrente;
     *         `false` caso contrário.
     */
    virtual bool canTransitionTo(const VehicleState& target) const noexcept = 0;

    /**
     * @brief Nome legível do estado (para *logs* e mensagens).
     * @return C-string com o rótulo do estado, válida durante a vida útil
     *         do objeto.
     */
    virtual const char* name() const noexcept = 0;

protected:
    /**
     * @brief Construtor padrão protegido — apenas classes derivadas podem
     *        instanciar.
     */
    VehicleState() noexcept;

    VehicleState(const VehicleState&) = delete;
    VehicleState& operator=(const VehicleState&) = delete;
};

inline VehicleState::VehicleState() noexcept {}
inline VehicleState::~VehicleState() noexcept {}

} // namespace state
} // namespace ecu

#endif // ECU_STATE_VEHICLESTATE_HPP
