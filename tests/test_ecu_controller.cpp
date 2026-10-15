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
