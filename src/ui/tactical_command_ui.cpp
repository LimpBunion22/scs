#include "ui/tactical_command_ui.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>
#include <utility>

namespace scs::ui {
namespace {

constexpr double kMinimumKilometersPerCell = 1.0;
constexpr double kMaximumKilometersPerCell = 10'000'000.0;
constexpr std::size_t kCommandLogLimit = 12;

bool has_friendly_entity(const presentation::TacticalSnapshot& snapshot, domain::EntityId id) {
    return std::any_of(snapshot.friendly_entities.begin(),
                       snapshot.friendly_entities.end(),
                       [id](const auto& entity) { return entity.id == id; });
}

bool has_hostile_contact(const presentation::TacticalSnapshot& snapshot, domain::ContactId id) {
    return std::any_of(snapshot.hostile_contacts.begin(),
                       snapshot.hostile_contacts.end(),
                       [id](const auto& contact) { return contact.id == id; });
}

TacticalInputResult finish(TacticalUiState& state,
                           TacticalInputResult result,
                           std::string feedback) {
    result.feedback = std::move(feedback);
    if (!result.feedback.empty()) {
        append_command_log(state, result.feedback);
    }
    return result;
}

bool read_optional_ticks(std::istringstream& stream, domain::Tick& ticks) {
    if (stream.eof()) {
        ticks = 1;
        return true;
    }

    stream >> ticks;
    return !stream.fail() && ticks > 0;
}

} // namespace

std::string tactical_command_help() {
    return "Commands: pan <east_cells> <north_cells>, zoom <in|out|km_per_cell>, "
           "select friendly <id>, select contact <id>, select none, pause, resume, "
           "scale <auto|multiplier>, run [ticks], step [ticks], velocity <vx> <vy>, quit.";
}

gameplay::PlayerTimeScaleInput make_time_scale_input(const TacticalUiState& state) {
    return gameplay::PlayerTimeScaleInput{state.tactical_pause, state.manual_time_scale};
}

void append_command_log(TacticalUiState& state, std::string entry) {
    state.command_log.push_back(std::move(entry));
    if (state.command_log.size() > kCommandLogLimit) {
        state.command_log.erase(state.command_log.begin(),
                                state.command_log.begin() +
                                    static_cast<std::ptrdiff_t>(state.command_log.size() -
                                                               kCommandLogLimit));
    }
}

TacticalInputResult handle_tactical_input(TacticalUiState& state,
                                          const presentation::TacticalSnapshot& snapshot,
                                          std::string_view line) {
    std::istringstream stream{std::string(line)};
    std::string verb;
    stream >> verb;
    if (verb.empty()) {
        return finish(state, TacticalInputResult{}, "No command entered.");
    }

    if (verb == "help") {
        return finish(state, TacticalInputResult{}, tactical_command_help());
    }

    if (verb == "quit" || verb == "q") {
        TacticalInputResult result;
        result.quit = true;
        return finish(state, result, "Quit requested.");
    }

    if (verb == "pan") {
        double east_cells = 0.0;
        double north_cells = 0.0;
        if (!(stream >> east_cells >> north_cells)) {
            return finish(state, TacticalInputResult{}, "Pan requires east and north cell offsets.");
        }

        state.view.center_km.x += east_cells * state.view.kilometers_per_cell;
        state.view.center_km.y += north_cells * state.view.kilometers_per_cell;
        return finish(state, TacticalInputResult{}, "Map view panned.");
    }

    if (verb == "zoom") {
        std::string value;
        if (!(stream >> value)) {
            return finish(state, TacticalInputResult{}, "Zoom requires in, out, or a positive km-per-cell value.");
        }

        if (value == "in") {
            state.view.kilometers_per_cell =
                std::max(kMinimumKilometersPerCell, state.view.kilometers_per_cell * 0.5);
            return finish(state, TacticalInputResult{}, "Map view zoomed in.");
        }
        if (value == "out") {
            state.view.kilometers_per_cell =
                std::min(kMaximumKilometersPerCell, state.view.kilometers_per_cell * 2.0);
            return finish(state, TacticalInputResult{}, "Map view zoomed out.");
        }

        std::istringstream value_stream{value};
        double kilometers_per_cell = 0.0;
        if (!(value_stream >> kilometers_per_cell) || kilometers_per_cell <= 0.0) {
            return finish(state, TacticalInputResult{}, "Zoom value must be positive.");
        }

        state.view.kilometers_per_cell =
            std::min(kMaximumKilometersPerCell,
                     std::max(kMinimumKilometersPerCell, kilometers_per_cell));
        return finish(state, TacticalInputResult{}, "Map view scale set.");
    }

    if (verb == "select") {
        std::string kind;
        stream >> kind;
        if (kind == "none") {
            state.selection = rendering::TacticalSelection{};
            return finish(state, TacticalInputResult{}, "Selection cleared.");
        }

        std::uint32_t id_value = 0;
        if (!(stream >> id_value)) {
            return finish(state, TacticalInputResult{}, "Selection requires a kind and id.");
        }

        if (kind == "friendly") {
            const domain::EntityId id{id_value};
            if (!has_friendly_entity(snapshot, id)) {
                return finish(state, TacticalInputResult{}, "Friendly entity is not visible.");
            }
            state.selection = rendering::TacticalSelection{
                rendering::TacticalSelectionKind::FriendlyEntity,
                id,
                domain::ContactId{},
            };
            return finish(state, TacticalInputResult{}, "Friendly entity selected.");
        }

        if (kind == "contact") {
            const domain::ContactId id{id_value};
            if (!has_hostile_contact(snapshot, id)) {
                return finish(state, TacticalInputResult{}, "Hostile contact is not visible.");
            }
            state.selection = rendering::TacticalSelection{
                rendering::TacticalSelectionKind::HostileContact,
                domain::EntityId{},
                id,
            };
            return finish(state, TacticalInputResult{}, "Hostile contact selected.");
        }

        return finish(state, TacticalInputResult{}, "Selection kind must be friendly, contact, or none.");
    }

    if (verb == "pause") {
        state.tactical_pause = true;
        return finish(state, TacticalInputResult{}, "Tactical pause enabled.");
    }

    if (verb == "resume") {
        state.tactical_pause = false;
        return finish(state, TacticalInputResult{}, "Tactical pause cleared.");
    }

    if (verb == "scale") {
        std::string value;
        if (!(stream >> value)) {
            return finish(state, TacticalInputResult{}, "Scale requires auto or a positive multiplier.");
        }

        if (value == "auto") {
            state.manual_time_scale.reset();
            return finish(state, TacticalInputResult{}, "Automatic time scale restored.");
        }

        std::istringstream value_stream{value};
        double override_scale = 0.0;
        if (!(value_stream >> override_scale) || override_scale <= 0.0) {
            return finish(state, TacticalInputResult{}, "Scale multiplier must be positive.");
        }

        state.manual_time_scale = override_scale;
        return finish(state, TacticalInputResult{}, "Manual time scale override set.");
    }

    if (verb == "run" || verb == "step") {
        domain::Tick ticks = 1;
        if (!read_optional_ticks(stream, ticks)) {
            return finish(state, TacticalInputResult{}, "Tick advance requires a positive tick count.");
        }

        if (verb == "run" && state.tactical_pause) {
            return finish(state, TacticalInputResult{}, "Simulation is paused; resume or step explicitly.");
        }

        TacticalInputResult result;
        result.advance_ticks = ticks;
        return finish(state, result, "Simulation tick advance requested.");
    }

    if (verb == "velocity") {
        double velocity_x = 0.0;
        double velocity_y = 0.0;
        if (!(stream >> velocity_x >> velocity_y)) {
            return finish(state, TacticalInputResult{}, "Velocity requires x and y km/s components.");
        }

        if (state.selection.kind != rendering::TacticalSelectionKind::FriendlyEntity ||
            !has_friendly_entity(snapshot, state.selection.entity)) {
            return finish(state, TacticalInputResult{}, "Select a visible friendly entity before issuing velocity.");
        }

        TacticalInputResult result;
        result.command = domain::set_velocity_at(snapshot.tick,
                                                 state.selection.entity,
                                                 domain::Vec2{velocity_x, velocity_y});
        return finish(state, result, "Velocity command emitted.");
    }

    return finish(state, TacticalInputResult{}, "Unknown command.");
}

} // namespace scs::ui
