#include "io/DataLogger.hpp"

namespace ecu {
namespace io {

DataLogger::DataLogger(const std::string& outputPath)
    : m_outputPath(outputPath),
      m_stream(),
      m_recordCount(0) {
    // TODO C8
}

DataLogger::~DataLogger() noexcept {
    // TODO C8
}

void DataLogger::open() {
    // TODO C8
}

void DataLogger::close() noexcept {
    // TODO C8
}

bool DataLogger::isOpen() const noexcept {
    // TODO C8
    return false;
}

void DataLogger::logSample(unsigned long /*timestamp*/,
                           double /*pedal*/,
                           double /*torque*/,
                           double /*temperature*/) {
    // TODO C8
}

void DataLogger::logModeChange(unsigned long /*timestamp*/,
                               const std::string& /*modeLabel*/) {
    // TODO C8
}

void DataLogger::logWarning(unsigned long /*timestamp*/,
                            const std::string& /*message*/) {
    // TODO C8
}

void DataLogger::logError(unsigned long /*timestamp*/,
                          const std::string& /*reason*/) {
    // TODO C8
}

void DataLogger::flush() noexcept {
    // TODO C8
}

const std::string& DataLogger::outputPath() const noexcept {
    // TODO C8
    return m_outputPath;
}

unsigned long DataLogger::recordCount() const noexcept {
    // TODO C8
    return m_recordCount;
}

} // namespace io
} // namespace ecu
