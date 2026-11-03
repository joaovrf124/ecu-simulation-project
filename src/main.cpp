// Programa de linha de comando que executa a simulacao completa a partir de
// um cenario CSV e grava o log em `data/logs/`.
//
// Uso:
//     ./build/ecu_simulator [cenario.csv] [arquivo_de_log]
// Se nenhum argumento for passado, utiliza `data/scenarios/exemplo.csv` e
// grava em `data/logs/exemplo.log`.
//
// Codigos de saida:
//     0  — simulacao concluida (mesmo se terminou em FaultState).
//     1  — EcuException nao classificada abaixo (nao deveria ocorrer).
//     2  — std::exception generica (incluindo bad_alloc).
//     3  — excecao desconhecida (catch ...).
//    10  — cenario informado nao encontrado (uso incorreto da CLI).
//    11  — FileIOException: falha de I/O ao ler/gravar arquivo.
//    12  — TelemetryParseError: cenario sem dados utilizaveis.
//    13  — std::invalid_argument: argumento invalido para alguma classe.
//    14  — std::logic_error: violacao de pre-condicao interna.

#include <exception>
#include <fstream>
#include <iostream>
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

namespace {

const char* kDefaultScenario = "data/scenarios/exemplo.csv";
const char* kDefaultLog      = "data/logs/exemplo.log";

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

void printUsage(const char* prog) {
    std::cerr << "Uso: " << prog
              << " [cenario.csv] [arquivo_de_log]\n"
              << "Padrao: cenario=" << kDefaultScenario
              << ", log=" << kDefaultLog << '\n';
}

bool fileExists(const std::string& path) {
    std::ifstream probe(path.c_str());
    return probe.is_open();
}

} // namespace

int main(int argc, char* argv[]) {
    // Suporte explicito a --help/-h: mostra o uso e sai sem rodar.
    if (argc > 1) {
        const std::string first = argv[1];
        if (first == "--help" || first == "-h") {
            printUsage(argv[0]);
            return 0;
        }
    }

    const std::string scenarioPath = (argc > 1)
        ? std::string(argv[1])
        : std::string(kDefaultScenario);
    const std::string logPath = (argc > 2)
        ? std::string(argv[2])
        : std::string(kDefaultLog);

    // Validacao antecipada de CLI: se o cenario nao existe, imprime o uso
    // antes de qualquer alocacao, para dar diagnostico claro ao usuario.
    if (!fileExists(scenarioPath)) {
        std::cerr << "[ECU] Cenario nao encontrado: " << scenarioPath << '\n';
        printUsage(argv[0]);
        return 10;
    }

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

    } catch (const ecu::exceptions::FileIOException& e) {
        std::cerr << "[ECU] Erro de I/O: " << e.what() << '\n';
        return 11;
    } catch (const ecu::exceptions::TelemetryParseError& e) {
        std::cerr << "[ECU] Erro de parse da telemetria: " << e.what() << '\n';
        return 12;
    } catch (const ecu::exceptions::EcuException& e) {
        std::cerr << "[ECU] Falha do simulador: " << e.what() << '\n';
        return 1;
    } catch (const std::invalid_argument& e) {
        std::cerr << "[ECU] Argumento invalido: " << e.what() << '\n';
        return 13;
    } catch (const std::logic_error& e) {
        std::cerr << "[ECU] Violacao de contrato interno: " << e.what() << '\n';
        return 14;
    } catch (const std::exception& e) {
        std::cerr << "[ECU] Erro inesperado: " << e.what() << '\n';
        return 2;
    } catch (...) {
        std::cerr << "[ECU] Erro desconhecido.\n";
        return 3;
    }
}
