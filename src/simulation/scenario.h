#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "domain/entity.h"

namespace scs::simulation {

struct Scenario {
    std::string name;
    std::uint64_t seed{0};
    double fixed_step_seconds{1.0};
    std::vector<domain::EntityState> entities;
};

Scenario make_default_vertical_slice_scenario();

} // namespace scs::simulation
