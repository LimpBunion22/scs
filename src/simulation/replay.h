#pragma once

#include <vector>

#include "domain/command.h"
#include "domain/event.h"
#include "domain/snapshot.h"
#include "domain/time.h"
#include "simulation/scenario.h"

namespace scs::simulation {

struct ReplayInput {
    Scenario scenario;
    std::vector<domain::Command> commands;
    domain::Tick ticks_to_run{0};
};

struct ReplayResult {
    domain::WorldSnapshot final_snapshot;
    std::vector<domain::Event> events;
};

ReplayResult run_replay(const ReplayInput& input);

} // namespace scs::simulation
