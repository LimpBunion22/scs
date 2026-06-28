#include "simulation/contact_tracker.h"

#include <algorithm>

namespace scs::simulation {
namespace {

constexpr double observed_uncertainty_radius_km = 0.0;
constexpr double stale_uncertainty_growth_km_per_second = 100.0;
constexpr double stale_confidence_decay_per_second = 0.05;

bool is_opposed(domain::Allegiance observer, domain::Allegiance target) {
    return (observer == domain::Allegiance::Friendly && target == domain::Allegiance::Hostile) ||
           (observer == domain::Allegiance::Hostile && target == domain::Allegiance::Friendly);
}

bool is_inside_sensor_range(const domain::EntityState& observer, const domain::EntityState& target) {
    if (observer.sensor_range_km <= 0.0) {
        return false;
    }

    const auto offset = target.position_km - observer.position_km;
    const double range_squared = observer.sensor_range_km * observer.sensor_range_km;
    return domain::magnitude_squared(offset) <= range_squared;
}

domain::ContactClassification classify(const domain::EntityState& target) {
    if (target.kind == domain::EntityKind::CombatGroup) {
        return domain::ContactClassification::HostileCombatGroup;
    }
    return domain::ContactClassification::Unknown;
}

std::vector<const domain::EntityState*> ordered_entities(const std::vector<domain::EntityState>& entities) {
    std::vector<const domain::EntityState*> ordered;
    ordered.reserve(entities.size());
    for (const auto& entity : entities) {
        ordered.push_back(&entity);
    }

    std::sort(ordered.begin(), ordered.end(), [](const auto* lhs, const auto* rhs) {
        return lhs->id < rhs->id;
    });
    return ordered;
}

} // namespace

std::vector<ContactTrackerEvent> ContactTracker::update(domain::Tick tick,
                                                         double fixed_step_seconds,
                                                         const std::vector<domain::EntityState>& entities) {
    for (auto& track : tracks_) {
        track.observed_this_update = false;
    }

    std::vector<ContactTrackerEvent> events;
    const auto ordered = ordered_entities(entities);

    // ponytail: This O(n^2) sensor sweep is fine for the two-group slice; replace it
    // with spatial partitioning when entity counts make the scan measurable.
    for (const auto* observer : ordered) {
        for (const auto* target : ordered) {
            if (observer->id == target->id ||
                !is_opposed(observer->allegiance, target->allegiance) ||
                !is_inside_sensor_range(*observer, *target)) {
                continue;
            }

            auto* track = find_track(observer->id, target->id);
            const bool is_new_contact = track == nullptr;
            if (is_new_contact) {
                const auto id = next_contact_id_;
                next_contact_id_ = domain::ContactId{next_contact_id_.value + 1};
                tracks_.push_back(Track{
                    id,
                    observer->id,
                    target->id,
                    target->position_km,
                    target->velocity_km_per_second,
                    tick,
                    1.0,
                    classify(*target),
                    observed_uncertainty_radius_km,
                    true,
                });
                track = &tracks_.back();
            } else {
                track->estimated_position_km = target->position_km;
                track->estimated_velocity_km_per_second = target->velocity_km_per_second;
                track->last_observed_tick = tick;
                track->confidence = 1.0;
                track->classification = classify(*target);
                track->uncertainty_radius_km = observed_uncertainty_radius_km;
                track->observed_this_update = true;
            }

            events.push_back(ContactTrackerEvent{
                is_new_contact ? domain::EventSeverity::Advisory : domain::EventSeverity::Info,
                is_new_contact ? domain::EventType::ContactDetected : domain::EventType::ContactUpdated,
                observer->id,
                track->id,
            });
        }
    }

    for (auto& track : tracks_) {
        if (track.observed_this_update) {
            continue;
        }

        track.estimated_position_km += track.estimated_velocity_km_per_second * fixed_step_seconds;
        track.uncertainty_radius_km += stale_uncertainty_growth_km_per_second * fixed_step_seconds;
        track.confidence = std::max(0.0, track.confidence - stale_confidence_decay_per_second * fixed_step_seconds);
    }

    return events;
}

std::vector<domain::ContactSnapshot> ContactTracker::snapshots() const {
    std::vector<domain::ContactSnapshot> result;
    result.reserve(tracks_.size());

    for (const auto& track : tracks_) {
        result.push_back(domain::ContactSnapshot{
            track.id,
            track.observer,
            track.estimated_position_km,
            track.estimated_velocity_km_per_second,
            track.last_observed_tick,
            track.confidence,
            track.classification,
            track.uncertainty_radius_km,
        });
    }

    return result;
}

ContactTracker::Track* ContactTracker::find_track(domain::EntityId observer, domain::EntityId target) {
    const auto found = std::find_if(tracks_.begin(), tracks_.end(), [observer, target](const auto& track) {
        return track.observer == observer && track.target == target;
    });
    return found == tracks_.end() ? nullptr : &(*found);
}

} // namespace scs::simulation
