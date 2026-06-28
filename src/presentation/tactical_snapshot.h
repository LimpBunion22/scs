#pragma once

#include <vector>

#include "domain/contact.h"
#include "domain/entity.h"
#include "domain/event.h"
#include "domain/snapshot.h"
#include "domain/time.h"
#include "presentation/prediction.h"

namespace scs::presentation {

enum class TacticalTrajectorySourceKind {
    FriendlyEntity,
    HostileContact
};

struct TacticalTrajectory {
    TacticalTrajectorySourceKind source_kind{TacticalTrajectorySourceKind::FriendlyEntity};
    domain::EntityId entity;
    domain::ContactId contact;
    std::vector<TrajectoryPoint> points;
};

struct TacticalSnapshotOptions {
    PredictionConfig prediction;
};

struct TacticalSnapshot {
    domain::Tick tick{0};
    double time_seconds{0.0};
    std::vector<domain::EntitySnapshot> friendly_entities;
    std::vector<domain::ContactSnapshot> hostile_contacts;
    std::vector<domain::Event> events;
    std::vector<TacticalTrajectory> predicted_trajectories;
};

[[nodiscard]] TacticalSnapshot make_tactical_snapshot(const domain::WorldSnapshot& world,
                                                      const std::vector<domain::Event>& events,
                                                      TacticalSnapshotOptions options);

} // namespace scs::presentation
