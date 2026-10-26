#include "config_read.hpp"
#include <fstream>
#include <stdexcept>
#include <filesystem>
#include <string>

// namespace fs = std::filesystem;

ConfigReader::ConfigReader(const std::string& filename) {
    std::ifstream file(filename);

    if (!file) {
        throw std::runtime_error(
            "Could not open config file: " + filename
        );
    }

    file >> config_;
}

double ConfigReader::length() const {
    return config_["channel"]["length"].get<double>();
}

double ConfigReader::width() const {
    return config_["channel"]["width"].get<double>();
}

double ConfigReader::height() const {
    return config_["channel"]["height"].get<double>();
}

std::array<double, 3> ConfigReader::center() const {
    return {
        config_["channel"]["center"]["x"], 
        config_["channel"]["center"]["y"], 
        config_["channel"]["center"]["z"]
    };
}

double ConfigReader::in_pressure() const {
    return config_["channel"]["in_pressure"].get<double>();
}

double ConfigReader::out_pressure() const {
    return config_["channel"]["out_pressure"].get<double>();
}

double ConfigReader::resolution() const {
    return config_["mesh"]["resolution"].get<double>();
}

std::string ConfigReader::name() const {
    return config_["mesh"]["name"];
}