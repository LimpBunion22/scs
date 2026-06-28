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

struct TacticalMissileTrack {
    domain::MissileId id;
    domain::EntityId launcher;
    domain::ContactId target_contact;
    domain::Vec2 position_km;
    domain::Vec2 velocity_km_per_second;
    domain::MissileStatus status{domain::MissileStatus::InFlight};
};

struct TacticalSnapshot {
    domain::Tick tick{0};
    double time_seconds{0.0};
    std::vector<domain::EntitySnapshot> friendly_entities;
    std::vector<domain::ContactSnapshot> hostile_contacts;
    std::vector<TacticalMissileTrack> missile_tracks;
    std::vector<domain::Event> events;
    std::vector<TacticalTrajectory> predicted_trajectories;
};

[[nodiscard]] TacticalSnapshot make_tactical_snapshot(const domain::WorldSnapshot& world,
                                                      const std::vector<domain::Event>& events,
                                                      TacticalSnapshotOptions options);

} // namespace scs::presentation
