#include "presentation/tactical_snapshot.h"

namespace scs::presentation {
namespace {

double infer_seconds_per_tick(const domain::WorldSnapshot& world) {
    if (world.tick > 0) {
        return world.time_seconds / static_cast<double>(world.tick);
    }

    // ponytail: WorldSnapshot does not expose fixed-step length yet. The first
    // vertical slice uses one-second ticks; add fixed_step_seconds to snapshots
    // before supporting non-1s initial projections.
    return 1.0;
}

TacticalTrajectory make_entity_trajectory(const domain::WorldSnapshot& world,
                                          const domain::EntitySnapshot& entity,
                                          double seconds_per_tick,
                                          PredictionConfig config) {
    return TacticalTrajectory{
        TacticalTrajectorySourceKind::FriendlyEntity,
        entity.id,
        domain::ContactId{},
        predict_inertial_trajectory(world.tick,
                                    world.time_seconds,
                                    entity.position_km,
                                    entity.velocity_km_per_second,
                                    seconds_per_tick,
                                    config),
    };
}

TacticalTrajectory make_contact_trajectory(const domain::WorldSnapshot& world,
                                           const domain::ContactSnapshot& contact,
                                           double seconds_per_tick,
                                           PredictionConfig config) {
    return TacticalTrajectory{
        TacticalTrajectorySourceKind::HostileContact,
        domain::EntityId{},
        contact.id,
        predict_inertial_trajectory(world.tick,
                                    world.time_seconds,
                                    contact.estimated_position_km,
                                    contact.estimated_velocity_km_per_second,
                                    seconds_per_tick,
                                    config),
    };
}

} // namespace

TacticalSnapshot make_tactical_snapshot(const domain::WorldSnapshot& world,
                                        const std::vector<domain::Event>& events,
                                        TacticalSnapshotOptions options) {
    TacticalSnapshot result;
    result.tick = world.tick;
    result.time_seconds = world.time_seconds;
    result.events = events;

    const double seconds_per_tick = infer_seconds_per_tick(world);

    for (const auto& entity : world.entities) {
        if (entity.allegiance != domain::Allegiance::Friendly) {
            continue;
        }

        result.friendly_entities.push_back(entity);
        result.predicted_trajectories.push_back(
            make_entity_trajectory(world, entity, seconds_per_tick, options.prediction));
    }

    for (const auto& contact : world.contacts) {
        result.hostile_contacts.push_back(contact);
        result.predicted_trajectories.push_back(
            make_contact_trajectory(world, contact, seconds_per_tick, options.prediction));
    }

    return result;
}

} // namespace scs::presentation
