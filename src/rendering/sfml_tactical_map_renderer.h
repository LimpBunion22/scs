#pragma once

#include <SFML/Graphics.hpp>

#include "domain/contact.h"
#include "domain/ids.h"
#include "domain/snapshot.h"
#include "presentation/tactical_snapshot.h"
#include "rendering/tactical_map_projection.h"
#include "rendering/tactical_map_renderer.h"

namespace scs::rendering {

struct TacticalMapHighlight {
    domain::EntityId entity;
    domain::ContactId contact;
    domain::MissileId missile;
};

struct SfmlTacticalMapStyle {
    sf::Color background{10, 14, 18};
    sf::Color border{78, 94, 105};
    sf::Color grid_major{35, 45, 54};
    sf::Color grid_minor{24, 31, 38};
    sf::Color friendly{88, 190, 152};
    sf::Color contact{231, 158, 88};
    sf::Color missile{228, 220, 120};
    sf::Color trajectory{92, 121, 151};
    sf::Color uncertainty{214, 125, 92, 94};
    sf::Color selection{240, 245, 246};
    sf::Color hover{123, 175, 220};
};

void draw_sfml_tactical_map(sf::RenderTarget& target,
                            const presentation::TacticalSnapshot& snapshot,
                            TacticalMapProjection projection,
                            TacticalSelection selection,
                            TacticalMapHighlight highlight = TacticalMapHighlight{},
                            const SfmlTacticalMapStyle& style = SfmlTacticalMapStyle{});

} // namespace scs::rendering
