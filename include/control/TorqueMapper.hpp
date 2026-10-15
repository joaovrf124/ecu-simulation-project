#ifndef ECU_CONTROL_TORQUEMAPPER_HPP
#define ECU_CONTROL_TORQUEMAPPER_HPP

#include <string>
#include <vector>

namespace ecu {
namespace control {

/**
 * @brief Modos de operação do mapeador de torque — correspondem às
 *        tabelas de calibração disponíveis.
 */
enum class TorqueMode {
    Eco,     ///< Mapa Eco — resposta suave, torque limitado para economia.
    MidTerm, ///< Mapa intermediário — equilíbrio entre resposta e consumo.
    Sport    ///< Mapa esportivo — resposta agressiva, torque máximo.
};

/**
 * @brief Ponto `(pedal, torque)` de uma tabela de mapa de torque.
 *
 * O valor `pedal` deve estar em porcentagem no intervalo [0, 100] e o
 * torque é expresso na unidade escolhida pela calibração.
 */
struct TorquePoint {
    double pedal;  ///< Posição do pedal, em %.
    double torque; ///< Torque associado ao ponto, na unidade da calibração.
};

/**
 * @brief Converte a posição bruta do pedal em torque final, selecionando
 *        a tabela do modo ativo.
 *
 * Responsabilidades (conforme cartão CRC):
 *  - Conhecer qual tabela (Eco/MidTerm/Sport) está selecionada.
 *  - Receber a porcentagem bruta do pedal do acelerador.
 *  - Interpolar linearmente entre os pontos da tabela para obter o torque
 *    exato.
 *  - Aplicar rampa (*ramping*) para limitar a variação entre ticks
 *    consecutivos ao limite de tração mecânica.
 *  - Limitar o torque final ao teto máximo de segurança do motor.
 */
class TorqueMapper {
public:
    /**
     * @brief Teto máximo absoluto de torque imposto por segurança.
     *
     * Nenhum valor devolvido pelo mapeador pode exceder este limite,
     * independentemente do modo ativo ou da tabela carregada.
     */
    static const double kMaxTorque;

    /**
     * @brief Variação máxima permitida de torque entre dois ticks
     *        consecutivos (rampa mecânica).
     */
    static const double kMaxTorqueDelta;

    /**
     * @brief Constrói o mapeador no modo `Eco` por padrão, com tabelas
     *        vazias (a serem carregadas antes do uso).
     */
    TorqueMapper();

    /**
     * @brief Destrutor padrão.
     */
    ~TorqueMapper() noexcept;

    TorqueMapper(const TorqueMapper&) = delete;
    TorqueMapper& operator=(const TorqueMapper&) = delete;

    /**
     * @brief Substitui a tabela de pontos associada a um modo.
     *
     * A tabela deve estar ordenada por `pedal` crescente e cobrir ao menos
     * os extremos 0% e 100% para que a interpolação funcione em todo o
     * domínio.
     *
     * @param mode   Modo cuja tabela será substituída.
     * @param points Lista de pontos `(pedal, torque)` que define o mapa.
     */
    void setMapTable(TorqueMode mode, const std::vector<TorquePoint>& points);

    /**
     * @brief Seleciona o modo ativo utilizado pelo próximo `compute()`.
     * @param mode Novo modo (Eco, MidTerm ou Sport).
     */
    void setMode(TorqueMode mode) noexcept;

    /**
     * @brief Modo atualmente ativo.
     * @return Enum do modo em vigor.
     */
    TorqueMode mode() const noexcept;

    /**
     * @brief Converte o pedal do acelerador em torque final, aplicando
     *        interpolação, rampa e teto de segurança.
     *
     * O valor retornado:
     *  - para `pedalPercent <= 0` devolve o torque do ponto 0% do mapa;
     *  - para `pedalPercent >= 100` devolve o torque do ponto 100%,
     *    limitado por `kMaxTorque`;
     *  - entre os pontos usa interpolação linear;
     *  - a diferença absoluta entre o retorno e o resultado do tick
     *    anterior é limitada a `kMaxTorqueDelta` (rampa).
     *
     * @param pedalPercent Posição do pedal, em %.
     * @return Torque final a ser requisitado ao motor.
     */
    double compute(double pedalPercent);

    /**
     * @brief Zera o estado interno de rampa (torque do tick anterior).
     *
     * Deve ser chamado em transições de estado que invalidem o histórico,
     * como entrada em `FaultState` ou retorno a `IdleState`.
     */
    void reset() noexcept;

    /**
     * @brief Último torque efetivamente devolvido por `compute()`.
     * @return Torque do tick anterior; `0.0` se `compute()` ainda não foi
     *         chamado desde o último `reset()`.
     */
    double lastTorque() const noexcept;

    /**
     * @brief Rótulo textual legível para um modo.
     * @param mode Modo a ser rotulado.
     * @return C-string estática `"Eco"`, `"MidTerm"` ou `"Sport"`.
     */
    static const char* modeLabel(TorqueMode mode) noexcept;

private:
    TorqueMode m_mode;
    std::vector<TorquePoint> m_tableEco;
    std::vector<TorquePoint> m_tableMid;
    std::vector<TorquePoint> m_tableSport;
    double m_lastTorque;
    bool m_hasPrevious;
};

} // namespace control
} // namespace ecu

#endif // ECU_CONTROL_TORQUEMAPPER_HPP
