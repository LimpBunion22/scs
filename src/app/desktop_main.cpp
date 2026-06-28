#include <algorithm>
#include <string>

#include <SFML/Graphics.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include "gameplay/time_scale_policy.h"
#include "presentation/tactical_snapshot.h"
#include "rendering/sfml_tactical_map_renderer.h"
#include "rendering/tactical_map_projection.h"
#include "simulation/scenario.h"
#include "simulation/simulation.h"
#include "ui/desktop_interaction.h"
#include "ui/desktop_time_controls.h"
#include "ui/imgui_tactical_panels.h"
#include "ui/tactical_command_ui.h"

namespace {

constexpr float kPanelWidth = 360.0F;

bool inside_viewport(scs::rendering::ScreenPoint point,
                     const scs::rendering::TacticalMapProjection& projection) {
    return point.x >= projection.viewport.left &&
           point.x <= projection.viewport.left + projection.viewport.width &&
           point.y >= projection.viewport.top &&
           point.y <= projection.viewport.top + projection.viewport.height;
}

scs::rendering::TacticalMapHighlight make_highlight(
    const scs::ui::DesktopMapObject& hover) {
    scs::rendering::TacticalMapHighlight highlight;
    if (hover.kind == scs::ui::DesktopMapObjectKind::FriendlyEntity) {
        highlight.entity = hover.entity;
    }
    if (hover.kind == scs::ui::DesktopMapObjectKind::HostileContact) {
        highlight.contact = hover.contact;
    }
    if (hover.kind == scs::ui::DesktopMapObjectKind::MissileTrack) {
        highlight.missile = hover.missile;
    }
    return highlight;
}

scs::rendering::ScreenPoint mouse_point(const sf::Event::MouseMoveEvent& event) {
    return scs::rendering::ScreenPoint{
        static_cast<double>(event.x),
        static_cast<double>(event.y),
    };
}

scs::rendering::ScreenPoint mouse_point(const sf::Event::MouseButtonEvent& event) {
    return scs::rendering::ScreenPoint{
        static_cast<double>(event.x),
        static_cast<double>(event.y),
    };
}

scs::rendering::ScreenPoint mouse_point(const sf::Event::MouseWheelScrollEvent& event) {
    return scs::rendering::ScreenPoint{
        static_cast<double>(event.x),
        static_cast<double>(event.y),
    };
}

} // namespace

int main() {
    sf::RenderWindow window(sf::VideoMode(1280, 720), "Space Combat Strategy");
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window)) {
        return 1;
    }

    auto scenario = scs::simulation::make_playable_engagement_demo_scenario();
    scs::simulation::Simulation simulation(scenario);

    scs::ui::TacticalUiState ui_state;
    ui_state.tactical_pause = true;
    scs::ui::DesktopTimeController time_controller{
        scs::ui::DesktopTimeControllerConfig{simulation.fixed_step_seconds(), 8}};
    scs::ui::DesktopInteractionState desktop_state;
    desktop_state.selection = scs::rendering::TacticalSelection{
        scs::rendering::TacticalSelectionKind::FriendlyEntity,
        scs::domain::EntityId{1},
        scs::domain::ContactId{},
    };
    desktop_state.staged_launcher = scs::domain::EntityId{1};

    scs::rendering::TacticalMapProjection projection{
        scs::domain::Vec2{175.0, 0.0},
        1.0,
        scs::rendering::ScreenRect{0.0, 0.0, 920.0, 720.0},
    };

    bool panning = false;
    scs::rendering::ScreenPoint last_mouse;
    sf::Clock delta_clock;

    while (window.isOpen()) {
        const auto tactical = scs::presentation::make_tactical_snapshot(
            simulation.snapshot(),
            simulation.events(),
            scs::presentation::TacticalSnapshotOptions{scs::presentation::PredictionConfig{8}});
        const auto time_scale = scs::gameplay::recommend_time_scale(
            tactical.tick,
            tactical.events,
            scs::ui::make_time_scale_input(ui_state));

        const sf::Vector2u window_size = window.getSize();
        const double panel_width =
            std::min<double>(kPanelWidth, std::max(0U, window_size.x));
        projection.viewport = scs::rendering::ScreenRect{
            0.0,
            0.0,
            std::max(1.0, static_cast<double>(window_size.x) - panel_width),
            std::max(1.0, static_cast<double>(window_size.y)),
        };

        sf::Event event{};
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::Resized) {
                window.setView(sf::View(sf::FloatRect{
                    0.0F,
                    0.0F,
                    static_cast<float>(event.size.width),
                    static_cast<float>(event.size.height),
                }));
            }

            const bool mouse_captured = ImGui::GetIO().WantCaptureMouse;
            if (!mouse_captured && event.type == sf::Event::MouseWheelScrolled) {
                const auto point = mouse_point(event.mouseWheelScroll);
                if (inside_viewport(point, projection)) {
                    scs::rendering::zoom_projection_around_screen_point(
                        projection,
                        point,
                        event.mouseWheelScroll.delta > 0.0F ? 0.8 : 1.25);
                }
            }

            if (!mouse_captured && event.type == sf::Event::MouseButtonPressed) {
                const auto point = mouse_point(event.mouseButton);
                if (inside_viewport(point, projection)) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        scs::ui::select_desktop_map_object(
                            desktop_state,
                            tactical,
                            projection,
                            point);
                    }
                    if (event.mouseButton.button == sf::Mouse::Right ||
                        event.mouseButton.button == sf::Mouse::Middle) {
                        panning = true;
                        last_mouse = point;
                    }
                }
            }

            if (event.type == sf::Event::MouseButtonReleased &&
                (event.mouseButton.button == sf::Mouse::Right ||
                 event.mouseButton.button == sf::Mouse::Middle)) {
                panning = false;
            }

            if (!mouse_captured && event.type == sf::Event::MouseMoved) {
                const auto point = mouse_point(event.mouseMove);
                if (panning) {
                    scs::rendering::pan_projection_by_pixels(
                        projection,
                        scs::rendering::ScreenPoint{
                            point.x - last_mouse.x,
                            point.y - last_mouse.y,
                        });
                    last_mouse = point;
                }
                if (inside_viewport(point, projection)) {
                    scs::ui::update_desktop_hover(desktop_state, tactical, projection, point);
                } else {
                    desktop_state.hover = scs::ui::DesktopMapObject{};
                }
            }
        }

        const sf::Time frame_delta = delta_clock.restart();
        ImGui::SFML::Update(window, frame_delta);

        const auto panel_result = scs::ui::draw_imgui_tactical_panels(
            ui_state,
            desktop_state,
            tactical,
            time_scale,
            time_controller,
            scenario.name,
            scs::ui::ImguiPanelLayout{
                static_cast<float>(projection.viewport.width),
                0.0F,
                static_cast<float>(panel_width),
                static_cast<float>(window_size.y),
            });

        if (!panel_result.feedback.empty()) {
            scs::ui::append_command_log(ui_state, panel_result.feedback);
        }
        if (panel_result.command.has_value()) {
            const bool accepted = simulation.submit(*panel_result.command);
            scs::ui::append_command_log(ui_state,
                                        accepted ? "Simulation accepted command."
                                                 : "Simulation rejected command.");
        }
        if (panel_result.advance_ticks > 0) {
            simulation.advance(panel_result.advance_ticks);
        }
        const auto continuous_ticks = time_controller.request_ticks(
            frame_delta.asSeconds(),
            time_scale);
        if (continuous_ticks > 0) {
            simulation.advance(continuous_ticks);
        }
        if (panel_result.quit) {
            window.close();
        }

        window.clear(sf::Color{5, 8, 11});
        scs::rendering::draw_sfml_tactical_map(
            window,
            tactical,
            projection,
            desktop_state.selection,
            make_highlight(desktop_state.hover));
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
