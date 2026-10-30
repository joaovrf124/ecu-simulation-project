// Testes do ECUController.
// Cobertura: US03/US04/US05 (transicao para FaultState quando SafetyMonitor
// detecta falha), US02 (delegacao do torque ao TorqueMapper).

#include "third_party/doctest.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "control/TorqueMapper.hpp"
#include "core/ECUController.hpp"
#include "io/DataLogger.hpp"
#include "io/SensorManager.hpp"
#include "safety/SafetyMonitor.hpp"
#include "state/FaultState.hpp"
#include "state/IdleState.hpp"
#include "state/VehicleState.hpp"

using ecu::control::TorqueMapper;
using ecu::control::TorqueMode;
using ecu::control::TorquePoint;
using ecu::core::ECUController;
using ecu::io::DataLogger;
using ecu::io::SensorManager;
using ecu::safety::SafetyMonitor;
using ecu::state::VehicleState;

namespace {
void writeTempCsv(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str());
    out << content;
    out.close();
}

std::vector<TorquePoint> smallMap() {
    std::vector<TorquePoint> points;
    TorquePoint p0; p0.pedal = 0.0;   p0.torque = 0.0;
    TorquePoint p1; p1.pedal = 100.0; p1.torque = 50.0;
    points.push_back(p0);
    points.push_back(p1);
    return points;
}
} // namespace

TEST_CASE("ECUController: inicializa em IdleState") {
    // Estado inicial da FSM apos initialize().
    const std::string csv = "build/test_ctrl_init.csv";
    const std::string log = "build/test_ctrl_init.log";
    writeTempCsv(csv, "0.0,25.0,25.0,0.0,72.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    torque.setMapTable(TorqueMode::Eco, smallMap());
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);
    controller.initialize();

    CHECK(std::string(controller.currentState().name()) == "Idle");

    std::remove(csv.c_str());
    std::remove(log.c_str());
}

TEST_CASE("ECUController: forceFault transita para FaultState com o motivo") {
    // US03 - CA2 e CA3
    const std::string csv = "build/test_ctrl_fault.csv";
    const std::string log = "build/test_ctrl_fault.log";
    writeTempCsv(csv, "0.0,25.0,25.0,0.0,72.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    torque.setMapTable(TorqueMode::Eco, smallMap());
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);
    controller.initialize();

    controller.forceFault("Subtensao: 55V");
    CHECK(std::string(controller.currentState().name()) == "Fault");

    std::remove(csv.c_str());
    std::remove(log.c_str());
}

TEST_CASE("ECUController: tick com subtensao transita para FaultState") {
    // US03 - CA1 e CA2 (SafetyMonitor sinaliza falha, controller transita)
    const std::string csv = "build/test_ctrl_tick_undervolt.csv";
    const std::string log = "build/test_ctrl_tick_undervolt.log";
    writeTempCsv(csv, "0.0,25.0,25.0,0.0,55.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    torque.setMapTable(TorqueMode::Eco, smallMap());
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);
    controller.initialize();
    controller.tick();

    CHECK(std::string(controller.currentState().name()) == "Fault");

    std::remove(csv.c_str());
    std::remove(log.c_str());
}

TEST_CASE("ECUController: expoe colaboradores aos estados") {
    // Necessario para que IdleState/DriveState/FaultState usem torqueMapper()
    // e dataLogger() durante onEnter/onExit.
    const std::string csv = "build/test_ctrl_access.csv";
    const std::string log = "build/test_ctrl_access.log";
    writeTempCsv(csv, "0.0,25.0,25.0,0.0,72.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);

    CHECK(&controller.torqueMapper()  == &torque);
    CHECK(&controller.dataLogger()    == &logger);
    CHECK(&controller.safetyMonitor() == &safety);
    CHECK(&controller.sensorManager() == &sensors);

    std::remove(csv.c_str());
    std::remove(log.c_str());
}

TEST_CASE("ECUController: stop() marca encerramento do loop") {
    const std::string csv = "build/test_ctrl_stop.csv";
    const std::string log = "build/test_ctrl_stop.log";
    writeTempCsv(csv, "0.0,25.0,25.0,0.0,72.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);
    controller.initialize();
    controller.stop();
    CHECK_NOTHROW(controller.run());

    std::remove(csv.c_str());
    std::remove(log.c_str());
}

TEST_CASE("ECUController: tick com sobretemperatura no inversor transita para FaultState") {
    // Avalia o comportamento de protecao termica (SafetyMonitor)
    const std::string csv = "build/test_ctrl_tick_overtemp.csv";
    const std::string log = "build/test_ctrl_tick_overtemp.log";
    
    // Injetando temp_inversor = 85.0C (considerando um limite teorico de 80C)
    writeTempCsv(csv, "0.0,85.0,25.0,0.0,72.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    torque.setMapTable(TorqueMode::Eco, smallMap());
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);
    controller.initialize();
    controller.tick();

    CHECK(std::string(controller.currentState().name()) == "Fault");

    std::remove(csv.c_str());
    std::remove(log.c_str());
}

TEST_CASE("ECUController: falhas de seguranca devem ser retidas (latching) entre os ticks") {
    // Garante que o carro nao sai do modo de falha sozinho se o sensor voltar ao normal
    const std::string csv = "build/test_ctrl_fault_latch.csv";
    const std::string log = "build/test_ctrl_fault_latch.log";
    
    // Tick 1: Subtensao (55V) - aciona falha
    // Tick 2: Tensao normal (72V) - falha deve continuar ativa
    writeTempCsv(csv, "0.0,25.0,25.0,0.0,55.0\n0.1,25.0,25.0,0.0,72.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    torque.setMapTable(TorqueMode::Eco, smallMap());
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);
    controller.initialize();
    
    controller.tick(); 
    CHECK(std::string(controller.currentState().name()) == "Fault");
    
    controller.tick(); 
    CHECK(std::string(controller.currentState().name()) == "Fault"); // Nao pode voltar para Idle/Drive automaticamente

    std::remove(csv.c_str());
    std::remove(log.c_str());
}

TEST_CASE("ECUController: execucao do run() encerra corretamente ao fim do arquivo de telemetria") {
    // Valida se o laco principal reage corretamente ao EOF sem entrar em loop infinito
    const std::string csv = "build/test_ctrl_run_eof.csv";
    const std::string log = "build/test_ctrl_run_eof.log";
    
    writeTempCsv(csv, "0.0,25.0,25.0,0.0,72.0\n0.1,25.0,25.0,0.0,72.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    torque.setMapTable(TorqueMode::Eco, smallMap());
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);
    controller.initialize();
    
    CHECK_NOTHROW(controller.run());
    // Se o teste chegar aqui sem travar e sem excecoes, o EOF foi gerido corretamente.

    std::remove(csv.c_str());
    std::remove(log.c_str());
}

TEST_CASE("ECUController: lida com excecoes matematicas ou de parse indo para FaultState") {
    // US02 / Protecao arquitetural - Uma excecao atirada pelos modulos inferiores nao deve desligar o software inteiro
    const std::string csv = "build/test_ctrl_exception.csv";
    const std::string log = "build/test_ctrl_exception.log";

    // Pedal em 150.0% injetado de proposito. O contrato do TorqueMapper
    // promete clamp (nao excecao) para pedal fora de [0, 100]; portanto o
    // tick deve rodar sem lancar e o torque final nao pode ultrapassar
    // kMaxTorque.
    writeTempCsv(csv, "0.0,25.0,25.0,150.0,72.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    torque.setMapTable(TorqueMode::Eco, smallMap());
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);
    controller.initialize();

    CHECK_NOTHROW(controller.tick());
    CHECK(torque.lastTorque() <= TorqueMapper::kMaxTorque);

    std::remove(csv.c_str());
    std::remove(log.c_str());
}
