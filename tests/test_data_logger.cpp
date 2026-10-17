// Testes do DataLogger.
// Cobertura: US01 (log de troca de mapa), US03 (log de erro critico),
// US07 (grava com timestamp).

#include "third_party/doctest.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

#include "io/DataLogger.hpp"

using ecu::io::DataLogger;

namespace {
std::string readAll(const std::string& path) {
    std::ifstream in(path.c_str());
    std::stringstream buffer;
    buffer << in.rdbuf();
    return buffer.str();
}
} // namespace

TEST_CASE("DataLogger: abre arquivo e grava amostra com timestamp") {
    // US07 - CA1 (gravar pedal, torque, temperatura, com timestamp)
    const std::string path = "build/test_log_sample.txt";
    std::remove(path.c_str());

    DataLogger logger(path);
    logger.open();
    CHECK(logger.isOpen() == true);
    CHECK(logger.outputPath() == path);

    logger.logSample(42, 10.0, 55.5, 70.0);
    logger.close();

    const std::string content = readAll(path);
    CHECK(content.find("42") != std::string::npos);
    CHECK(content.find("SAMPLE") != std::string::npos);

    std::remove(path.c_str());
}

TEST_CASE("DataLogger: registra troca de modo com timestamp") {
    // US01 - CA3 (toda troca aceita gravada pelo DataLogger)
    const std::string path = "build/test_log_mode.txt";
    std::remove(path.c_str());

    DataLogger logger(path);
    logger.open();
    logger.logModeChange(7, "Sport");
    logger.close();

    const std::string content = readAll(path);
    CHECK(content.find("7") != std::string::npos);
    CHECK(content.find("Sport") != std::string::npos);
    CHECK(content.find("MODE") != std::string::npos);

    std::remove(path.c_str());
}

TEST_CASE("DataLogger: registra alerta (warning) termico") {
    // US04 - CA1 (warning aos 60C registrado com timestamp)
    const std::string path = "build/test_log_warn.txt";
    std::remove(path.c_str());

    DataLogger logger(path);
    logger.open();
    logger.logWarning(13, "temperatura motor em 60C");
    logger.close();

    const std::string content = readAll(path);
    CHECK(content.find("WARN") != std::string::npos);
    CHECK(content.find("13") != std::string::npos);
    CHECK(content.find("60C") != std::string::npos);

    std::remove(path.c_str());
}

TEST_CASE("DataLogger: registra evento de erro critico") {
    // US03 - CA3 (DataLogger grava evento critico com timestamp e motivo)
    const std::string path = "build/test_log_err.txt";
    std::remove(path.c_str());

    DataLogger logger(path);
    logger.open();
    logger.logError(99, "Subtensao: 55V");
    logger.close();

    const std::string content = readAll(path);
    CHECK(content.find("ERROR") != std::string::npos);
    CHECK(content.find("99") != std::string::npos);
    CHECK(content.find("Subtensao") != std::string::npos);

    std::remove(path.c_str());
}

TEST_CASE("DataLogger: recordCount reflete numero de registros gravados") {
    const std::string path = "build/test_log_count.txt";
    std::remove(path.c_str());

    DataLogger logger(path);
    logger.open();
    CHECK(logger.recordCount() == 0u);
    logger.logSample(1, 10.0, 20.0, 30.0);
    logger.logSample(2, 11.0, 21.0, 31.0);
    logger.logModeChange(3, "Eco");
    CHECK(logger.recordCount() == 3u);
    logger.close();

    std::remove(path.c_str());
}
