#include "third_party/doctest.h"
#include "control/TractionControl.hpp"

TEST_CASE("Traction Control - Analise de Escorregamento e Corte") {
    // Inicializa o TCS com kp=1.0, ki=0.0, kd=0.0 e limite de slip a 15%
    ecu::control::TractionControl tcs(1.0, 0.0, 0.0, 15.0);

    SUBCASE("Nao corta torque se o escorregamento for menor que 15%") {
        // Dianteira a 50 km/h, Traseira a 55 km/h -> Slip de 10%
        double scale = tcs.computeScaleFactor(50.0, 55.0, 0.1);
        CHECK(scale == 1.0); // 1.0 significa 100% de torque (sem corte)
    }

    SUBCASE("Reduz o torque se a roda destracionar acima de 15%") {
        // Dianteira a 50 km/h, Traseira a 75 km/h -> Slip de 50%
        double scale = tcs.computeScaleFactor(50.0, 75.0, 0.1);
        CHECK(scale < 1.0); // Fator deve ser menor que 1.0 (corte aplicado)
    }

    SUBCASE("Carro parado ou dados invalidos nao ativam o corte") {
        double scale = tcs.computeScaleFactor(0.0, 20.0, 0.1);
        CHECK(scale == 1.0);
    }
}