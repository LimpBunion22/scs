#pragma once

#include <optional>
#include <string>

#include "domain/contact.h"
#include "domain/entity.h"
#include "domain/vector2.h"
#include "gameplay/time_scale_policy.h"
#include "presentation/tactical_snapshot.h"

namespace scs::rendering {

enum class TacticalSelectionKind {
    None,
    FriendlyEntity,
    HostileContact
};

struct TacticalSelection {
    TacticalSelectionKind kind{TacticalSelectionKind::None};
    domain::EntityId entity;
    domain::ContactId contact;
};

struct TacticalMapView {
    domain::Vec2 center_km;
    double kilometers_per_cell{100'000.0};
    int width_cells{61};
    int height_cells{21};
};

[[nodiscard]] std::string render_tactical_map(
    const presentation::TacticalSnapshot& snapshot,
    TacticalMapView view,
    TacticalSelection selection = TacticalSelection{},
    std::optional<gameplay::TimeScaleRecommendation> time_scale = std::nullopt);

} // namespace scs::rendering
