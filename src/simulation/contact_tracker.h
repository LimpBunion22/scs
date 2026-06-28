#pragma once

#include <vector>

#include "domain/contact.h"
#include "domain/entity.h"
#include "domain/event.h"
#include "domain/time.h"

namespace scs::simulation {

struct ContactTrackerEvent {
    domain::EventSeverity severity{domain::EventSeverity::Info};
    domain::EventType type{domain::EventType::ContactUpdated};
    domain::EntityId observer;
    domain::ContactId contact;
};

class ContactTracker {
public:
    [[nodiscard]] std::vector<ContactTrackerEvent> update(domain::Tick tick,
                                                          double fixed_step_seconds,
                                                          const std::vector<domain::EntityState>& entities);

    [[nodiscard]] std::vector<domain::ContactSnapshot> snapshots() const;

private:
    struct Track {
        domain::ContactId id;
        domain::EntityId observer;
        domain::EntityId target;
        domain::Vec2 estimated_position_km;
        domain::Vec2 estimated_velocity_km_per_second;
        domain::Tick last_observed_tick{0};
        double confidence{0.0};
        domain::ContactClassification classification{domain::ContactClassification::Unknown};
        double uncertainty_radius_km{0.0};
        bool observed_this_update{false};
    };

    domain::ContactId next_contact_id_{1};
    std::vector<Track> tracks_;

    [[nodiscard]] Track* find_track(domain::EntityId observer, domain::EntityId target);
};

} // namespace scs::simulation
