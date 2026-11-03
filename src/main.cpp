// Programa de linha de comando que executa a simulacao completa a partir de
// um cenario CSV e grava o log em `data/logs/`.
//
// Uso:
//     ./build/ecu_simulator [cenario.csv] [arquivo_de_log]
// Se nenhum argumento for passado, utiliza `data/scenarios/exemplo.csv` e
// grava em `data/logs/exemplo.log`.

#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "control/TorqueMapper.hpp"
#include "core/ECUController.hpp"
#include "exceptions/EcuException.hpp"
#include "io/DataLogger.hpp"
#include "io/SensorManager.hpp"
#include "safety/SafetyMonitor.hpp"
#include "state/DriveState.hpp"
#include "state/VehicleState.hpp"

namespace {

// Mapa de torque Eco de exemplo: curva suave, usada apenas para que o main
// tenha algo real para mapear. Em producao esses pontos sairiam de uma
// tabela de calibracao em data/maps/.
std::vector<ecu::control::TorquePoint> buildEcoMap() {
    std::vector<ecu::control::TorquePoint> points;
    ecu::control::TorquePoint p0; p0.pedal = 0.0;   p0.torque = 0.0;
    ecu::control::TorquePoint p1; p1.pedal = 50.0;  p1.torque = 90.0;
    ecu::control::TorquePoint p2; p2.pedal = 100.0; p2.torque = 180.0;
    points.push_back(p0);
    points.push_back(p1);
    points.push_back(p2);
    return points;
}

} // namespace

int main(int argc, char* argv[]) {
    const std::string scenarioPath = (argc > 1)
        ? std::string(argv[1])
        : std::string("data/scenarios/exemplo.csv");
    const std::string logPath = (argc > 2)
        ? std::string(argv[2])
        : std::string("data/logs/exemplo.log");

    std::cout << "[ECU] Cenario: " << scenarioPath << '\n';
    std::cout << "[ECU] Log:     " << logPath      << '\n';

    try {
        ecu::io::SensorManager   sensors(scenarioPath);
        ecu::safety::SafetyMonitor safety(sensors);
        ecu::control::TorqueMapper torque;
        ecu::io::DataLogger        logger(logPath);

        torque.setMapTable(ecu::control::TorqueMode::Eco, buildEcoMap());
        torque.setMode(ecu::control::TorqueMode::Eco);

        sensors.open();
        logger.open();

        ecu::core::ECUController controller(sensors, safety, torque, logger);
        controller.initialize();

        // Marca pre-carga concluida e autoriza transicao para Drive, para
        // que o cenario de aceleracao gere torque real no log. Em uma
        // implementacao futura o proprio controller faria essa transicao
        // ao detectar `safety.isPrechargeComplete()`.
        safety.markPrechargeComplete();
        std::unique_ptr<ecu::state::VehicleState> drive(
            new ecu::state::DriveState());
        controller.transitionTo(std::move(drive));

        controller.run();

        std::cout << "[ECU] Simulacao finalizada. Estado final: "
                  << controller.currentState().name()
                  << ". Registros gravados: " << logger.recordCount() << '\n';
        return 0;

    } catch (const ecu::exceptions::EcuException& e) {
        std::cerr << "[ECU] Falha do simulador: " << e.what() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "[ECU] Erro inesperado: " << e.what() << '\n';
        return 2;
    } catch (...) {
        std::cerr << "[ECU] Erro desconhecido." << '\n';
        return 3;
    }
}
