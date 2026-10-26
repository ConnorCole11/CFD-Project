#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <array>

class ConfigReader {
public:
    explicit ConfigReader(const std::string& filename);

    double length() const;
    double width() const;
    double height() const;
    std::array<double, 3> center() const;
    double in_pressure() const;
    double out_pressure() const;

    double resolution() const;

    std::string name() const;

private:
    nlohmann::json config_;
};