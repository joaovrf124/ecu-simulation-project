// Testes das classes de estado do veiculo.
// Cobertura: US01 (bloqueio de troca de mapa em Idle, aceitacao em Drive),
// US04/US05 (Fault ignora pedal), US08 (regras de transicao).

#include "third_party/doctest.h"

#include <string>

#include "state/DriveState.hpp"
#include "state/FaultState.hpp"
#include "state/IdleState.hpp"
#include "state/VehicleState.hpp"

using ecu::state::DriveState;
using ecu::state::FaultState;
using ecu::state::IdleState;
using ecu::state::VehicleState;

TEST_CASE("IdleState: pedal e troca de mapa sao bloqueados") {
    // US01 - bloqueio de troca de mapa quando inativo
    IdleState idle;
    CHECK(idle.acceptsThrottleInput() == false);
    CHECK(idle.acceptsTorqueMapSwitch() == false);
    CHECK(std::string(idle.name()) == "Idle");
}

TEST_CASE("IdleState: autoriza transicao para Drive e Fault, mas nao para si") {
    // US08 e US05
    IdleState idle;
    DriveState drive;
    FaultState fault("qualquer");
    IdleState other;

    CHECK(idle.canTransitionTo(drive) == true);
    CHECK(idle.canTransitionTo(fault) == true);
    CHECK(idle.canTransitionTo(other) == false);
}

TEST_CASE("DriveState: aceita pedal e troca dinamica de mapa") {
    // US01 - com veiculo em Drive, troca de mapa muda a tabela ativa
    // US02 - pedal e processado
    DriveState drive;
    CHECK(drive.acceptsThrottleInput() == true);
    CHECK(drive.acceptsTorqueMapSwitch() == true);
    CHECK(std::string(drive.name()) == "Drive");
}

TEST_CASE("DriveState: autoriza transicao para Fault e Idle, mas nao para si") {
    // US03/US04/US05
    DriveState drive;
    IdleState idle;
    FaultState fault("critica");
    DriveState other;

    CHECK(drive.canTransitionTo(fault) == true);
    CHECK(drive.canTransitionTo(idle) == true);
    CHECK(drive.canTransitionTo(other) == false);
}

TEST_CASE("FaultState: ignora pedal, bloqueia troca de mapa e e terminal") {
    // US03 / US04 - apos falha critica input e ignorado
    FaultState fault("Subtensao");
    CHECK(fault.acceptsThrottleInput() == false);
    CHECK(fault.acceptsTorqueMapSwitch() == false);
    CHECK(std::string(fault.name()) == "Fault");

    IdleState idle;
    DriveState drive;
    FaultState other("x");
    CHECK(fault.canTransitionTo(idle) == false);
    CHECK(fault.canTransitionTo(drive) == false);
    CHECK(fault.canTransitionTo(other) == false);
}

TEST_CASE("FaultState: guarda o motivo da falha") {
    // US03 - DataLogger registra motivo (reason precisa estar acessivel)
    FaultState fault("Subtensao: 55V");
    CHECK(fault.reason() == "Subtensao: 55V");
}

TEST_CASE("Transicoes referenciadas polimorficamente respeitam cada estado") {
    // VehicleState* deve servir para delegacao do ECUController
    IdleState idle;
    DriveState drive;
    FaultState fault("x");

    VehicleState* ptrIdle  = &idle;
    VehicleState* ptrDrive = &drive;
    VehicleState* ptrFault = &fault;

    CHECK(ptrIdle->acceptsTorqueMapSwitch() == false);
    CHECK(ptrDrive->acceptsTorqueMapSwitch() == true);
    CHECK(ptrFault->acceptsThrottleInput() == false);
}
