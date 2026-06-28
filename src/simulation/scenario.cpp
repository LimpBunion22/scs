#include "simulation/scenario.h"

namespace scs::simulation {

Scenario make_default_vertical_slice_scenario() {
    Scenario scenario;
    scenario.name = "vertical_slice_core";
    scenario.seed = 0x5c5c0001;
    scenario.fixed_step_seconds = 1.0;

    scenario.entities.push_back(domain::EntityState{
        domain::EntityId{1},
        domain::EntityKind::CombatGroup,
        domain::Allegiance::Friendly,
        "Blue Command Group",
        domain::Vec2{-1'000'000.0, 0.0},
        domain::Vec2{18.0, 0.0},
        750'000.0,
    });

    scenario.entities.push_back(domain::EntityState{
        domain::EntityId{2},
        domain::EntityKind::CombatGroup,
        domain::Allegiance::Hostile,
        "Red Command Group",
        domain::Vec2{1'000'000.0, 150'000.0},
        domain::Vec2{-16.0, -0.25},
        650'000.0,
    });

    return scenario;
}

} // namespace scs::simulation
