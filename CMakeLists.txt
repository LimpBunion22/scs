cmake_minimum_required(VERSION 3.16)

project(space_combat_strategy VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_EXTENSIONS OFF)

option(SCS_BUILD_DESKTOP_UI "Build the optional SFML/Dear ImGui desktop UI" OFF)

add_library(scs_core STATIC
    src/gameplay/time_scale_policy.cpp
    src/presentation/prediction.cpp
    src/presentation/tactical_snapshot.cpp
    src/simulation/contact_tracker.cpp
    src/simulation/missile.cpp
    src/simulation/replay.cpp
    src/simulation/scenario.cpp
    src/simulation/simulation.cpp
)

target_include_directories(scs_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(scs_core PUBLIC cxx_std_17)

if(MSVC)
    target_compile_options(scs_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(scs_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(scs_demo
    src/app/main.cpp
)

add_library(scs_ui STATIC
    src/ui/desktop_panel_model.cpp
    src/rendering/tactical_map_projection.cpp
    src/rendering/tactical_map_renderer.cpp
    src/ui/desktop_interaction.cpp
    src/ui/desktop_time_controls.cpp
    src/ui/tactical_command_ui.cpp
)

target_include_directories(scs_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(scs_ui PUBLIC cxx_std_17)

target_link_libraries(scs_ui PUBLIC scs_core)

if(MSVC)
    target_compile_options(scs_ui PRIVATE /W4 /permissive-)
else()
    target_compile_options(scs_ui PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(scs_demo PRIVATE scs_core scs_ui)

if(SCS_BUILD_DESKTOP_UI)
    include(FetchContent)

    set(SCS_IMGUI_VERSION "v1.91.1" CACHE STRING "Pinned Dear ImGui version for desktop UI")
    set(SCS_IMGUI_SFML_VERSION "v2.6.1" CACHE STRING "Pinned ImGui-SFML version for desktop UI")

    find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG ${SCS_IMGUI_VERSION}
    )
    FetchContent_GetProperties(imgui)
    if(NOT imgui_POPULATED)
        FetchContent_Populate(imgui)
    endif()

    set(IMGUI_DIR "${imgui_SOURCE_DIR}" CACHE PATH "Dear ImGui source directory" FORCE)
    set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "Use the SFML package already found by this project" FORCE)

    FetchContent_Declare(
        imgui_sfml
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG ${SCS_IMGUI_SFML_VERSION}
    )
    FetchContent_MakeAvailable(imgui_sfml)

    add_executable(scs_desktop
        src/app/desktop_main.cpp
        src/rendering/sfml_tactical_map_renderer.cpp
        src/ui/imgui_tactical_panels.cpp
    )

    target_compile_features(scs_desktop PRIVATE cxx_std_17)

    target_link_libraries(scs_desktop
        PRIVATE
            scs_core
            scs_ui
            ImGui-SFML::ImGui-SFML
            sfml-graphics
            sfml-window
            sfml-system
    )

    if(MSVC)
        target_compile_options(scs_desktop PRIVATE /W4 /permissive-)
    else()
        target_compile_options(scs_desktop PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endif()

enable_testing()

add_executable(scs_core_tests
    tests/simulation/deterministic_replay_tests.cpp
)

target_link_libraries(scs_core_tests PRIVATE scs_core)

add_test(NAME scs_core_tests COMMAND scs_core_tests)

add_executable(scs_contact_tests
    tests/simulation/contact_tests.cpp
)

target_link_libraries(scs_contact_tests PRIVATE scs_core)

add_test(NAME scs_contact_tests COMMAND scs_contact_tests)

add_executable(scs_missile_tests
    tests/simulation/missile_tests.cpp
)

target_link_libraries(scs_missile_tests PRIVATE scs_core)

add_test(NAME scs_missile_tests COMMAND scs_missile_tests)

add_executable(scs_time_scale_policy_tests
    tests/gameplay/time_scale_policy_tests.cpp
)

target_link_libraries(scs_time_scale_policy_tests PRIVATE scs_core)

add_test(NAME scs_time_scale_policy_tests COMMAND scs_time_scale_policy_tests)

add_executable(scs_presentation_tests
    tests/presentation/tactical_snapshot_tests.cpp
)

target_link_libraries(scs_presentation_tests PRIVATE scs_core)

add_test(NAME scs_presentation_tests COMMAND scs_presentation_tests)

add_executable(scs_ui_command_map_tests
    tests/integration/ui_command_map_tests.cpp
)

target_link_libraries(scs_ui_command_map_tests PRIVATE scs_core scs_ui)

add_test(NAME scs_ui_command_map_tests COMMAND scs_ui_command_map_tests)

add_executable(scs_desktop_interaction_tests
    tests/integration/desktop_interaction_tests.cpp
)

target_link_libraries(scs_desktop_interaction_tests PRIVATE scs_core scs_ui)

add_test(NAME scs_desktop_interaction_tests COMMAND scs_desktop_interaction_tests)

add_executable(scs_desktop_time_control_tests
    tests/integration/desktop_time_control_tests.cpp
)

target_link_libraries(scs_desktop_time_control_tests PRIVATE scs_core scs_ui)

add_test(NAME scs_desktop_time_control_tests COMMAND scs_desktop_time_control_tests)

add_executable(scs_desktop_panel_model_tests
    tests/integration/desktop_panel_model_tests.cpp
)

target_link_libraries(scs_desktop_panel_model_tests PRIVATE scs_core scs_ui)

add_test(NAME scs_desktop_panel_model_tests COMMAND scs_desktop_panel_model_tests)

add_executable(scs_desktop_render_model_tests
    tests/integration/desktop_render_model_tests.cpp
)

target_link_libraries(scs_desktop_render_model_tests PRIVATE scs_core scs_ui)

add_test(NAME scs_desktop_render_model_tests COMMAND scs_desktop_render_model_tests)

add_executable(scs_desktop_order_tests
    tests/integration/desktop_order_tests.cpp
)

target_link_libraries(scs_desktop_order_tests PRIVATE scs_core scs_ui)

add_test(NAME scs_desktop_order_tests COMMAND scs_desktop_order_tests)

add_executable(scs_desktop_command_flow_tests
    tests/integration/desktop_command_flow_tests.cpp
)

target_link_libraries(scs_desktop_command_flow_tests PRIVATE scs_core scs_ui)

add_test(NAME scs_desktop_command_flow_tests COMMAND scs_desktop_command_flow_tests)

add_executable(scs_vertical_slice_replay_tests
    tests/integration/vertical_slice_replay_tests.cpp
)

target_link_libraries(scs_vertical_slice_replay_tests PRIVATE scs_core)

add_test(NAME scs_vertical_slice_replay_tests COMMAND scs_vertical_slice_replay_tests)
