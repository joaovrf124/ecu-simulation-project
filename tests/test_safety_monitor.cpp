// Testes do SafetyMonitor.
// Cobertura: US03 (subtensao < 60V), US04 (warn 60C, fault 80C),
// US05 (Shutdown aberto), US08 (timeout de pre-carga).

#include "third_party/doctest.h"

#include <cstdio>
#include <fstream>
#include <string>

#include "exceptions/OvertemperatureFault.hpp"
#include "exceptions/PrechargeTimeoutFault.hpp"
#include "exceptions/ShutdownOpenFault.hpp"
#include "exceptions/UndervoltageFault.hpp"
#include "io/SensorManager.hpp"
#include "safety/SafetyMonitor.hpp"

using ecu::exceptions::OvertemperatureFault;
using ecu::exceptions::PrechargeTimeoutFault;
using ecu::exceptions::ShutdownOpenFault;
using ecu::exceptions::UndervoltageFault;
using ecu::io::SensorManager;
using ecu::safety::SafetyMonitor;

namespace {
// Grava uma linha simples e devolve um SensorManager posicionado nela.
void writeTempCsv(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str());
    out << content;
    out.close();
}
} // namespace

TEST_CASE("SafetyMonitor: constantes limite expoem os valores do enunciado") {
    // Documenta como os limites exigidos pelas US aparecem no header.
    CHECK(SafetyMonitor::kUndervoltageThresholdVolts == doctest::Approx(60.0));
    CHECK(SafetyMonitor::kTemperatureWarningCelsius  == doctest::Approx(60.0));
    CHECK(SafetyMonitor::kTemperatureFaultCelsius    == doctest::Approx(80.0));
    CHECK(SafetyMonitor::kPrechargeTimeoutTicks      > 0u);
}

TEST_CASE("SafetyMonitor: tensao exatamente 60V nao dispara UndervoltageFault") {
    // US03 - borda inferior do limite (abaixo e que gera falha)
    const std::string path = "build/test_safety_60v.csv";
    writeTempCsv(path, "0.0,25.0,25.0,0.0,60.0\n");

    SensorManager sm(path);
    sm.open();
    sm.advance();

    SafetyMonitor monitor(sm);
    CHECK_NOTHROW(monitor.check());

    std::remove(path.c_str());
}

TEST_CASE("SafetyMonitor: tensao logo abaixo de 60V dispara UndervoltageFault") {
    // US03 - CA1
    const std::string path = "build/test_safety_59v.csv";
    writeTempCsv(path, "0.0,25.0,25.0,0.0,59.9\n");

    SensorManager sm(path);
    sm.open();
    sm.advance();

    SafetyMonitor monitor(sm);
    CHECK_THROWS_AS(monitor.check(), UndervoltageFault);

    std::remove(path.c_str());
}

TEST_CASE("SafetyMonitor: temperatura 80C dispara OvertemperatureFault") {
    // US04 - CA2
    const std::string path = "build/test_safety_80c.csv";
    writeTempCsv(path, "0.0,80.0,25.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();
    sm.advance();

    SafetyMonitor monitor(sm);
    CHECK_THROWS_AS(monitor.check(), OvertemperatureFault);

    std::remove(path.c_str());
}

TEST_CASE("SafetyMonitor: temperatura 60C emite warning sem lancar excecao") {
    // US04 - CA1
    const std::string path = "build/test_safety_60c.csv";
    writeTempCsv(path, "0.0,60.0,25.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();
    sm.advance();

    SafetyMonitor monitor(sm);
    CHECK_NOTHROW(monitor.check());
    CHECK(monitor.hasThermalWarning() == true);

    std::remove(path.c_str());
}

TEST_CASE("SafetyMonitor: temperatura 59C nao gera warning") {
    const std::string path = "build/test_safety_59c.csv";
    writeTempCsv(path, "0.0,59.0,25.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();
    sm.advance();

    SafetyMonitor monitor(sm);
    CHECK_NOTHROW(monitor.check());
    CHECK(monitor.hasThermalWarning() == false);

    std::remove(path.c_str());
}

TEST_CASE("SafetyMonitor: shutdown aberto dispara ShutdownOpenFault") {
    // US05 - CA1 e CA2
    const std::string path = "build/test_safety_shutdown.csv";
    writeTempCsv(path, "0.0,25.0,25.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();
    sm.advance();

    SafetyMonitor monitor(sm);
    monitor.setShutdownOpen(true);
    CHECK(monitor.isShutdownOpen() == true);
    CHECK_THROWS_AS(monitor.check(), ShutdownOpenFault);

    std::remove(path.c_str());
}
TEST_CASE("SafetyMonitor: timeout de pre-carga dispara PrechargeTimeoutFault") {
    // US08 - CA2
    const std::string path = "build/test_safety_precharge.csv";
    writeTempCsv(path, "0.0,25.0,25.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();
    sm.advance();

    SafetyMonitor monitor(sm);
    CHECK(monitor.isPrechargeComplete() == false);

    bool thrown = false;
    for (unsigned long i = 0; i < SafetyMonitor::kPrechargeTimeoutTicks + 2; ++i) {
        try {
            monitor.tickPrecharge();
        } catch (const PrechargeTimeoutFault&) {
            thrown = true;
            break;
        }
    }
    CHECK(thrown);

    std::remove(path.c_str());
}

TEST_CASE("SafetyMonitor: markPrechargeComplete bloqueia o timeout") {
    const std::string path = "build/test_safety_precharge_ok.csv";
    writeTempCsv(path, "0.0,25.0,25.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();
    sm.advance();

    SafetyMonitor monitor(sm);
    monitor.markPrechargeComplete();
    CHECK(monitor.isPrechargeComplete() == true);

    for (unsigned long i = 0; i < SafetyMonitor::kPrechargeTimeoutTicks + 10; ++i) {
        CHECK_NOTHROW(monitor.tickPrecharge());
    }

    std::remove(path.c_str());

}
TEST_CASE("SafetyMonitor: temperatura da bateria a 80C dispara OvertemperatureFault") {
    const std::string path = "build/test_safety_bat_80c.csv";
    writeTempCsv(path, "0.0,25.0,80.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();
    sm.advance();

    SafetyMonitor monitor(sm);
    CHECK_THROWS_AS(monitor.check(), OvertemperatureFault);

    std::remove(path.c_str());
}

TEST_CASE("SafetyMonitor: warning termico e desativado quando a temperatura regressa ao normal") {
    const std::string path = "build/test_safety_thermal_recovery.csv";
    writeTempCsv(path, "0.0,61.0,25.0,0.0,72.0\n0.1,55.0,25.0,0.0,72.0\n");

    SensorManager sm(path);
    sm.open();

    SafetyMonitor monitor(sm);

    sm.advance();
    CHECK_NOTHROW(monitor.check());
    CHECK(monitor.hasThermalWarning() == true);

    sm.advance();
    CHECK_NOTHROW(monitor.check());
    CHECK(monitor.hasThermalWarning() == false);

    std::remove(path.c_str());
}

TEST_CASE("SafetyMonitor: falhas multiplas simultaneas garantem disparo de excecao") {
    const std::string path = "build/test_safety_multiple_faults.csv";
    writeTempCsv(path, "0.0,25.0,85.0,0.0,45.0\n");

    SensorManager sm(path);
    sm.open();
    sm.advance();

    SafetyMonitor monitor(sm);
    
    CHECK_THROWS(monitor.check());

    std::remove(path.c_str());
}
