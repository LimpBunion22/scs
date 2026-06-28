#include "simulation/replay.h"

#include "simulation/simulation.h"

namespace scs::simulation {

ReplayResult run_replay(const ReplayInput& input) {
    Simulation simulation(input.scenario);

    for (const auto& command : input.commands) {
        simulation.submit(command);
    }

    simulation.advance(input.ticks_to_run);

    return ReplayResult{simulation.snapshot(), simulation.events()};
}

} // namespace scs::simulation
