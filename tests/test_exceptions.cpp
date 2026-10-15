// Testes da hierarquia EcuException.
// Cobertura: US03 (UndervoltageFault), US04 (OvertemperatureFault),
// US05 (ShutdownOpenFault), US06 (TelemetryParseError), US08 (PrechargeTimeoutFault).

#include "third_party/doctest.h"

#include <exception>
#include <string>

#include "exceptions/EcuException.hpp"
#include "exceptions/OvertemperatureFault.hpp"
#include "exceptions/PrechargeTimeoutFault.hpp"
#include "exceptions/ShutdownOpenFault.hpp"
#include "exceptions/TelemetryParseError.hpp"
#include "exceptions/UndervoltageFault.hpp"

using ecu::exceptions::EcuException;
using ecu::exceptions::OvertemperatureFault;
using ecu::exceptions::PrechargeTimeoutFault;
using ecu::exceptions::ShutdownOpenFault;
using ecu::exceptions::TelemetryParseError;
using ecu::exceptions::UndervoltageFault;

TEST_CASE("TelemetryParseError e capturavel como EcuException e std::exception") {
    // US06
    try {
        throw TelemetryParseError("linha 42 invalida");
    } catch (const EcuException& e) {
        CHECK(std::string(e.what()) == "linha 42 invalida");
    }

    try {
        throw TelemetryParseError("linha 42 invalida");
    } catch (const std::exception& e) {
        CHECK(std::string(e.what()) == "linha 42 invalida");
    }
}

TEST_CASE("UndervoltageFault carrega mensagem padrao e customizada") {
    // US03
    CHECK_THROWS_AS(throw UndervoltageFault(), EcuException);
    CHECK_THROWS_AS(throw UndervoltageFault(), std::exception);

    try {
        throw UndervoltageFault();
    } catch (const EcuException& e) {
        CHECK(std::string(e.what()).find("60V") != std::string::npos);
    }

    try {
        throw UndervoltageFault("tensao=55V");
    } catch (const EcuException& e) {
        CHECK(std::string(e.what()) == "tensao=55V");
    }
}

TEST_CASE("OvertemperatureFault captura motor ou bateria acima de 80 C") {
    // US04
    CHECK_THROWS_AS(throw OvertemperatureFault(), EcuException);
    CHECK_THROWS_AS(throw OvertemperatureFault(), std::exception);

    try {
        throw OvertemperatureFault("motor=82C");
    } catch (const std::exception& e) {
        CHECK(std::string(e.what()) == "motor=82C");
    }
}

TEST_CASE("ShutdownOpenFault sinaliza abertura do circuito") {
    // US05
    CHECK_THROWS_AS(throw ShutdownOpenFault(), EcuException);
    CHECK_THROWS_AS(throw ShutdownOpenFault(), std::exception);

    try {
        throw ShutdownOpenFault();
    } catch (const EcuException& e) {
        CHECK(std::string(e.what()).find("Shutdown") != std::string::npos);
    }
}

TEST_CASE("PrechargeTimeoutFault bloqueia Idle->Drive") {
    // US08
    CHECK_THROWS_AS(throw PrechargeTimeoutFault(), EcuException);
    CHECK_THROWS_AS(throw PrechargeTimeoutFault(), std::exception);

    try {
        throw PrechargeTimeoutFault("t=1200ms");
    } catch (const EcuException& e) {
        CHECK(std::string(e.what()) == "t=1200ms");
    }
}

TEST_CASE("Catch generico em EcuException alcanca todas as derivadas") {
    bool caught = false;
    try {
        throw UndervoltageFault("x");
    } catch (const EcuException&) {
        caught = true;
    }
    CHECK(caught);

    caught = false;
    try {
        throw OvertemperatureFault("x");
    } catch (const EcuException&) {
        caught = true;
    }
    CHECK(caught);

    caught = false;
    try {
        throw ShutdownOpenFault("x");
    } catch (const EcuException&) {
        caught = true;
    }
    CHECK(caught);

    caught = false;
    try {
        throw TelemetryParseError("x");
    } catch (const EcuException&) {
        caught = true;
    }
    CHECK(caught);

    caught = false;
    try {
        throw PrechargeTimeoutFault("x");
    } catch (const EcuException&) {
        caught = true;
    }
    CHECK(caught);
}
