#include <iostream>
#include <fstream>
#include <vector>
#include <string>
// Incluye la librería de nlohmann
#include <nlohmann/json.hpp>

#include <planets.h>
#include <tactical_window.h>
#include <physic_engine.h>
#include <basic_entities.h>
#include <basic_ship.h>

std::vector<planet> planets;
std::vector<basic_ship> ships;

void load_scenario(std::string file_name, tactical_window_handler & tactical_window, physic_engine & engine);