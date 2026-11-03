// Testes de programacao defensiva e tratamento de excecoes (C9).
// Cobertura: validacao de entrada no SensorManager (NaN/inf, pedal fora
// de [0, 100], tensao negativa, temperatura absurda, arquivo vazio),
// setMapTable do TorqueMapper (vazia/duplicada/fora de ordem),
// pre-condicoes do DataLogger (gravar sem open) e do ECUController
// (tick antes de initialize, transitionTo com estado nulo), alem da
// hierarquia FileIOException.

#include "third_party/doctest.h"

#include <cstdio>
#include <exception>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "control/TorqueMapper.hpp"
#include "core/ECUController.hpp"
#include "exceptions/EcuException.hpp"
#include "exceptions/FileIOException.hpp"
#include "exceptions/TelemetryParseError.hpp"
#include "io/DataLogger.hpp"
#include "io/SensorManager.hpp"
#include "safety/SafetyMonitor.hpp"
#include "state/DriveState.hpp"
#include "state/VehicleState.hpp"

using ecu::control::TorqueMapper;
using ecu::control::TorqueMode;
using ecu::control::TorquePoint;
using ecu::core::ECUController;
using ecu::exceptions::EcuException;
using ecu::exceptions::FileIOException;
using ecu::exceptions::TelemetryParseError;
using ecu::io::DataLogger;
using ecu::io::SensorManager;
using ecu::safety::SafetyMonitor;

namespace {
void writeTempCsv(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str());
    out << content;
    out.close();
}

std::vector<TorquePoint> validLinearMap() {
    std::vector<TorquePoint> points;
    TorquePoint p0; p0.pedal = 0.0;   p0.torque = 0.0;
    TorquePoint p1; p1.pedal = 100.0; p1.torque = 50.0;
    points.push_back(p0);
    points.push_back(p1);
    return points;
}
} // namespace

// ---------------------------------------------------------------------------
// Hierarquia de excecoes
// ---------------------------------------------------------------------------

TEST_CASE("FileIOException: capturavel como EcuException e std::exception") {
    bool asEcu = false;
    try {
        throw FileIOException("falha de I/O X");
    } catch (const EcuException& e) {
        asEcu = (std::string(e.what()) == "falha de I/O X");
    }
    CHECK(asEcu);

    bool asStd = false;
    try {
        throw FileIOException("falha Y");
    } catch (const std::exception& e) {
        asStd = (std::string(e.what()) == "falha Y");
    }
    CHECK(asStd);
}

TEST_CASE("FileIOException: construtor sem argumento tem mensagem padrao") {
    try {
        throw FileIOException();
    } catch (const EcuException& e) {
        CHECK(std::string(e.what()).find("I/O") != std::string::npos);
    }
}

// ---------------------------------------------------------------------------
// SensorManager — validacao de entrada
// ---------------------------------------------------------------------------

TEST_CASE("SensorManager: open em arquivo inexistente lanca FileIOException") {
    SensorManager sm("build/defensive_arquivo_que_nao_existe.csv");
    CHECK_THROWS_AS(sm.open(), FileIOException);
}

TEST_CASE("SensorManager: open em arquivo vazio lanca TelemetryParseError") {
    const std::string path = "build/defensive_vazio.csv";
    writeTempCsv(path, "");
    SensorManager sm(path);
    CHECK_THROWS_AS(sm.open(), TelemetryParseError);
    std::remove(path.c_str());
}

TEST_CASE("SensorManager: NaN em campo faz advance() pular a linha") {
    const std::string path = "build/defensive_nan.csv";
    writeTempCsv(path,
                 "10.0,25.0,30.0,0.0,72.0\n"
                 "nan,25.0,30.0,0.0,72.0\n"
                 "20.0,25.0,30.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();
    CHECK(sm.advance() == true);
    CHECK(sm.pedal() == doctest::Approx(10.0));
    // A linha com NaN e pulada; o proximo advance pega direto a 3a linha.
    CHECK(sm.advance() == true);
    CHECK(sm.pedal() == doctest::Approx(20.0));
    std::remove(path.c_str());
}

TEST_CASE("SensorManager: NaN em advanceStrict lanca TelemetryParseError") {
    const std::string path = "build/defensive_nan_strict.csv";
    writeTempCsv(path, "nan,25.0,30.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();
    CHECK_THROWS_AS(sm.advanceStrict(), TelemetryParseError);
    std::remove(path.c_str());
}

TEST_CASE("SensorManager: pedal fora de [0, 100] e pulado por advance") {
    const std::string path = "build/defensive_pedal.csv";
    writeTempCsv(path,
                 "10.0,25.0,30.0,0.0,72.0\n"
                 "150.0,25.0,30.0,0.0,72.0\n"   // pedal invalido
                 "-5.0,25.0,30.0,0.0,72.0\n"    // pedal negativo
                 "80.0,25.0,30.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();
    CHECK(sm.advance() == true);
    CHECK(sm.pedal() == doctest::Approx(10.0));
    CHECK(sm.advance() == true);
    CHECK(sm.pedal() == doctest::Approx(80.0));
    std::remove(path.c_str());
}

TEST_CASE("SensorManager: tensao negativa lanca em advanceStrict") {
    const std::string path = "build/defensive_voltage.csv";
    writeTempCsv(path, "10.0,25.0,30.0,0.0,-5.0\n");

    SensorManager sm(path);
    sm.open();
    CHECK_THROWS_AS(sm.advanceStrict(), TelemetryParseError);
    std::remove(path.c_str());
}

TEST_CASE("SensorManager: temperatura absurda lanca em advanceStrict") {
    const std::string path = "build/defensive_temp.csv";
    writeTempCsv(path, "10.0,500.0,30.0,0.0,72.0\n");  // engineTemp absurdo

    SensorManager sm(path);
    sm.open();
    CHECK_THROWS_AS(sm.advanceStrict(), TelemetryParseError);
    std::remove(path.c_str());
}

TEST_CASE("SensorManager: campos a mais sao rejeitados em advance") {
    const std::string path = "build/defensive_extra.csv";
    writeTempCsv(path,
                 "10.0,25.0,30.0,0.0,72.0,99.9\n"   // 6 campos
                 "20.0,25.0,30.0,0.0,72.0\n");      // 5 campos (OK)

    SensorManager sm(path);
    sm.open();
    CHECK(sm.advance() == true);
    CHECK(sm.pedal() == doctest::Approx(20.0));
    std::remove(path.c_str());
}

// ---------------------------------------------------------------------------
// TorqueMapper — validacao de entrada
// ---------------------------------------------------------------------------

TEST_CASE("TorqueMapper: setMapTable com tabela vazia lanca invalid_argument") {
    TorqueMapper mapper;
    std::vector<TorquePoint> empty;
    CHECK_THROWS_AS(mapper.setMapTable(TorqueMode::Eco, empty),
                    std::invalid_argument);
}

TEST_CASE("TorqueMapper: setMapTable com pontos fora de ordem lanca") {
    TorqueMapper mapper;
    std::vector<TorquePoint> points;
    TorquePoint a; a.pedal = 50.0; a.torque = 50.0;
    TorquePoint b; b.pedal = 20.0; b.torque = 20.0;  // pedal recua
    points.push_back(a);
    points.push_back(b);
    CHECK_THROWS_AS(mapper.setMapTable(TorqueMode::Sport, points),
                    std::invalid_argument);
}

TEST_CASE("TorqueMapper: setMapTable com pedais duplicados lanca") {
    TorqueMapper mapper;
    std::vector<TorquePoint> points;
    TorquePoint a; a.pedal = 10.0; a.torque = 10.0;
    TorquePoint b; b.pedal = 10.0; b.torque = 20.0;  // pedal duplicado
    points.push_back(a);
    points.push_back(b);
    CHECK_THROWS_AS(mapper.setMapTable(TorqueMode::MidTerm, points),
                    std::invalid_argument);
}

TEST_CASE("TorqueMapper: tabela valida nao lanca") {
    TorqueMapper mapper;
    CHECK_NOTHROW(mapper.setMapTable(TorqueMode::Eco, validLinearMap()));
}

// ---------------------------------------------------------------------------
// DataLogger — gravar sem abrir
// ---------------------------------------------------------------------------

TEST_CASE("DataLogger: logSample sem open lanca FileIOException") {
    DataLogger logger("build/defensive_log_notopen.txt");
    CHECK(logger.isOpen() == false);
    CHECK_THROWS_AS(logger.logSample(1, 0.0, 0.0, 0.0), FileIOException);
}

TEST_CASE("DataLogger: logWarning sem open lanca FileIOException") {
    DataLogger logger("build/defensive_log_notopen.txt");
    CHECK_THROWS_AS(logger.logWarning(1, "x"), FileIOException);
}

TEST_CASE("DataLogger: logError sem open lanca FileIOException") {
    DataLogger logger("build/defensive_log_notopen.txt");
    CHECK_THROWS_AS(logger.logError(1, "x"), FileIOException);
}

TEST_CASE("DataLogger: logModeChange sem open lanca FileIOException") {
    DataLogger logger("build/defensive_log_notopen.txt");
    CHECK_THROWS_AS(logger.logModeChange(1, "Eco"), FileIOException);
}

TEST_CASE("DataLogger: open em diretorio inexistente lanca FileIOException") {
    // Caminho dentro de um diretorio que nao existe forca falha de I/O.
    DataLogger logger("build/dir_inexistente/log.txt");
    CHECK_THROWS_AS(logger.open(), FileIOException);
}

// ---------------------------------------------------------------------------
// ECUController — pre-condicoes
// ---------------------------------------------------------------------------

TEST_CASE("ECUController: tick antes de initialize lanca std::logic_error") {
    const std::string csv = "build/defensive_ctrl_tick.csv";
    const std::string log = "build/defensive_ctrl_tick.log";
    writeTempCsv(csv, "0.0,25.0,25.0,0.0,72.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);
    // Nao chama initialize() de proposito.
    CHECK_THROWS_AS(controller.tick(), std::logic_error);

    std::remove(csv.c_str());
    std::remove(log.c_str());
}

TEST_CASE("ECUController: transitionTo(nullptr) lanca std::invalid_argument") {
    const std::string csv = "build/defensive_ctrl_null.csv";
    const std::string log = "build/defensive_ctrl_null.log";
    writeTempCsv(csv, "0.0,25.0,25.0,0.0,72.0\n");

    SensorManager sensors(csv);
    sensors.open();
    SafetyMonitor safety(sensors);
    TorqueMapper torque;
    DataLogger logger(log);
    logger.open();

    ECUController controller(sensors, safety, torque, logger);
    controller.initialize();

    std::unique_ptr<ecu::state::VehicleState> empty;
    CHECK_THROWS_AS(controller.transitionTo(std::move(empty)),
                    std::invalid_argument);

    std::remove(csv.c_str());
    std::remove(log.c_str());
}
