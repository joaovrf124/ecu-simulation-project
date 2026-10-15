// Testes do SensorManager.
// Cobertura: US06 (CSV de cenario), US07 (telemetria disponibilizada por tick),
// RNF04 (linhas invalidas sao puladas).

#include "third_party/doctest.h"

#include <cstdio>
#include <fstream>
#include <string>

#include "exceptions/TelemetryParseError.hpp"
#include "io/SensorManager.hpp"

using ecu::exceptions::TelemetryParseError;
using ecu::io::SensorManager;

namespace {
// Escreve um CSV temporario em build/ para o teste.
void writeTempCsv(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str());
    out << content;
    out.close();
}
} // namespace

TEST_CASE("SensorManager: abre CSV valido e avanca ticks sequenciais") {
    // US06 - abertura sem crash, US07 - leituras disponiveis
    const std::string path = "build/test_sensor_valid.csv";
    writeTempCsv(path,
                 "10.0,25.0,30.0,0.0,72.0\n"
                 "50.0,55.0,45.0,0.0,70.5\n"
                 "80.0,65.0,55.0,0.0,69.0\n");

    SensorManager sm(path);
    sm.open();

    CHECK(sm.filePath() == path);
    CHECK(sm.hasNext() == true);

    CHECK(sm.advance() == true);
    CHECK(sm.pedal() == doctest::Approx(10.0));
    CHECK(sm.engineTemperature() == doctest::Approx(25.0));
    CHECK(sm.batteryTemperature() == doctest::Approx(30.0));
    CHECK(sm.brakePressure() == doctest::Approx(0.0));
    CHECK(sm.voltage() == doctest::Approx(72.0));
    CHECK(sm.currentTick() == 1u);

    CHECK(sm.advance() == true);
    CHECK(sm.pedal() == doctest::Approx(50.0));
    CHECK(sm.currentTick() == 2u);

    CHECK(sm.advance() == true);
    CHECK(sm.currentTick() == 3u);

    CHECK(sm.advance() == false);
    CHECK(sm.hasNext() == false);

    std::remove(path.c_str());
}

TEST_CASE("SensorManager: linhas mal formatadas sao puladas pelo advance()") {
    // RNF04 - robustez de parsing, US06 - simulacao prossegue
    const std::string path = "build/test_sensor_bad.csv";
    writeTempCsv(path,
                 "10.0,25.0,30.0,0.0,72.0\n"
                 "linha,invalida\n"
                 "\n"
                 "80.0,65.0,55.0,0.0,69.0\n");

    SensorManager sm(path);
    sm.open();

    CHECK(sm.advance() == true);
    CHECK(sm.pedal() == doctest::Approx(10.0));

    // A proxima linha valida deve ser a quarta (as duas intermediarias foram puladas).
    CHECK(sm.advance() == true);
    CHECK(sm.pedal() == doctest::Approx(80.0));

    std::remove(path.c_str());
}

TEST_CASE("SensorManager: advanceStrict() lanca TelemetryParseError em linha invalida") {
    // RNF04 - versao estrita sinaliza o consumidor
    const std::string path = "build/test_sensor_strict.csv";
    writeTempCsv(path,
                 "10.0,25.0,30.0,0.0,72.0\n"
                 "linha,invalida\n");

    SensorManager sm(path);
    sm.open();

    CHECK(sm.advanceStrict() == true);
    CHECK_THROWS_AS(sm.advanceStrict(), TelemetryParseError);

    std::remove(path.c_str());
}

TEST_CASE("SensorManager: open() lanca em arquivo inexistente") {
    // US06 - caminho invalido deve ser reportado
    SensorManager sm("build/arquivo_que_nao_existe_12345.csv");
    CHECK_THROWS_AS(sm.open(), TelemetryParseError);
}

TEST_CASE("SensorManager: tick inicial e zero antes do primeiro advance") {
    const std::string path = "build/test_sensor_tick.csv";
    writeTempCsv(path, "1.0,2.0,3.0,4.0,5.0\n");

    SensorManager sm(path);
    sm.open();
    CHECK(sm.currentTick() == 0u);

    std::remove(path.c_str());
}
