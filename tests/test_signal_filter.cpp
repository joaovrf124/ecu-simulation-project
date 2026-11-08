#include "third_party/doctest.h"
#include "sensors/SignalFilter.hpp"
#include <stdexcept>

TEST_CASE("SignalFilter - Filtragem digital EMA para sinais de sensores") {
    SUBCASE("Rejeita parametro alpha invalido") {
        CHECK_THROWS_AS(ecu::sensors::SignalFilter(0.0), std::invalid_argument);
        CHECK_THROWS_AS(ecu::sensors::SignalFilter(-0.5), std::invalid_argument);
        CHECK_THROWS_AS(ecu::sensors::SignalFilter(1.2), std::invalid_argument);
    }

    SUBCASE("Primeira amostra inicializa o filtro sem atraso e proximas suavizam") {
        ecu::sensors::SignalFilter filter(0.5);
        CHECK_FALSE(filter.isInitialized());

        // Primeira amostra (ex: 20% de pedal)
        CHECK(filter.filter(20.0) == doctest::Approx(20.0));
        CHECK(filter.isInitialized());

        // Degrau para 40%: com alpha = 0.5, saida deve ser 0.5*40 + 0.5*20 = 30.0
        CHECK(filter.filter(40.0) == doctest::Approx(30.0));

        // Mantendo 40%: saida vai para 0.5*40 + 0.5*30 = 35.0
        CHECK(filter.filter(40.0) == doctest::Approx(35.0));
    }

    SUBCASE("Reset limpa estado interno do filtro") {
        ecu::sensors::SignalFilter filter(0.25);
        filter.filter(100.0);
        filter.reset();

        CHECK_FALSE(filter.isInitialized());
        CHECK(filter.lastValue() == doctest::Approx(0.0));
    }
}