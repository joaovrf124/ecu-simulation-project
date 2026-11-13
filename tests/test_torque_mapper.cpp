// Testes do TorqueMapper.
// Cobertura: US01 (modos Eco/MidTerm/Sport), US02 (interpolacao linear,
// pedal 0% e 100%, rampa, teto maximo).

#include "third_party/doctest.h"

#include <vector>

#include "control/TorqueMapper.hpp"

using ecu::control::TorqueMapper;
using ecu::control::TorqueMode;
using ecu::control::TorquePoint;

namespace {
std::vector<TorquePoint> buildLinearMap(double maxTorque) {
    std::vector<TorquePoint> points;
    TorquePoint p0; p0.pedal = 0.0;   p0.torque = 0.0;
    TorquePoint p1; p1.pedal = 50.0;  p1.torque = maxTorque * 0.5;
    TorquePoint p2; p2.pedal = 100.0; p2.torque = maxTorque;
    points.push_back(p0);
    points.push_back(p1);
    points.push_back(p2);
    return points;
}
} // namespace

TEST_CASE("TorqueMapper: modo inicial e Eco e pode ser alterado") {
    // US01 - selecao de modo
    TorqueMapper mapper;
    CHECK(mapper.mode() == TorqueMode::Eco);

    mapper.setMode(TorqueMode::Sport);
    CHECK(mapper.mode() == TorqueMode::Sport);

    mapper.setMode(TorqueMode::MidTerm);
    CHECK(mapper.mode() == TorqueMode::MidTerm);
}

TEST_CASE("TorqueMapper: rotulos textuais dos modos") {
    // US01 - DataLogger usa o rotulo
    CHECK(std::string(TorqueMapper::modeLabel(TorqueMode::Eco))     == "Eco");
    CHECK(std::string(TorqueMapper::modeLabel(TorqueMode::MidTerm)) == "MidTerm");
    CHECK(std::string(TorqueMapper::modeLabel(TorqueMode::Sport))   == "Sport");
}

TEST_CASE("TorqueMapper: pedal 0% devolve 0 e pedal 100% devolve o pico") {
    // US02 - extremos da tabela
    TorqueMapper mapper;
    mapper.setMapTable(TorqueMode::Eco, buildLinearMap(100.0));
    mapper.setMode(TorqueMode::Eco);

    CHECK(mapper.compute(0.0)   == doctest::Approx(0.0));
    mapper.reset();
    CHECK(mapper.compute(100.0) == doctest::Approx(100.0));
}

TEST_CASE("TorqueMapper: interpolacao linear entre dois pontos") {
    // US02 - CA1 (criterio de aceitacao 1 da US02)
    TorqueMapper mapper;
    mapper.setMapTable(TorqueMode::Sport, buildLinearMap(200.0));
    mapper.setMode(TorqueMode::Sport);

    // Entre pedal=0 (torque=0) e pedal=50 (torque=100), pedal=25 => torque=50.
    CHECK(mapper.compute(25.0) == doctest::Approx(50.0));
}

