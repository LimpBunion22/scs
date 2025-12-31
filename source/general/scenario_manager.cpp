#include <scenario_manager.h>

// Usamos un alias para simplificar
using json = nlohmann::json;

void load_scenario(std::string file_name, tactical_window_handler & tactical_window, physic_engine & engine){

    // 1. Abrir el archivo JSON
    std::ifstream file(file_name); // Ajusta el nombre/path según corresponda
    if (!file.is_open()) {
        std::cerr << "[ERROR]   [SM]    No se pudo abrir el archivo JSON.\n";
        return ;
    }

    // 2. Parsear el JSON
    json j;
    try {
        file >> j;
    } catch (std::exception& e) {
        std::cerr << "[ERROR]   [SM]    Error al parsear JSON: " << e.what() << "\n";
        return ;
    }

    // 3. Leer y almacenar "stars"
    if (j.contains("stars") && j["stars"].is_array()) {
        for (auto& starJson : j["stars"]) {
            std::vector<float> pos;
            std::vector<float> vel;
            std::string name;
            float mass, radius;
            // name
            if (starJson.contains("name")) {
                name = starJson["name"].get<std::string>();
            }
            // position
            if (starJson.contains("position") && starJson["position"].is_array()) {
                for (auto& val : starJson["position"]) {
                    pos.push_back(val.get<float>());
                }
            }
            // velocity
            if (starJson.contains("velocity") && starJson["velocity"].is_array()) {
                for (auto& val : starJson["velocity"]) {
                    vel.push_back(val.get<float>());
                }
            }
            // mass
            if (starJson.contains("mass")) {
                mass = starJson["mass"].get<float>();
            }
            // radius
            if (starJson.contains("radius")) {
                radius = starJson["radius"].get<float>();
            }

            basic_state state(pos,vel);
            planet star(mass, radius, state);
            star.name = name;
            star.star();

            // Añadir a la lista
            engine.emplace_planet(star);
        }
    }

    // 5. Leer y almacenar "planets"
    if (j.contains("planets") && j["planets"].is_array()) {
        for (auto& planetJson : j["planets"]) {
            std::vector<float> pos;
            std::vector<float> vel;
            std::string name;
            float mass, radius;
            
            // name
            if (planetJson.contains("name")) {
                name = planetJson["name"].get<std::string>();
            }
            // position
            if (planetJson.contains("position") && planetJson["position"].is_array()) {
                for (auto& val : planetJson["position"]) {
                    pos.push_back(val.get<float>());
                }
            }
            // velocity
            if (planetJson.contains("velocity") && planetJson["velocity"].is_array()) {
                for (auto& val : planetJson["velocity"]) {
                    vel.push_back(val.get<float>());
                }
            }
            // mass
            if (planetJson.contains("mass")) {
                mass = planetJson["mass"].get<float>();
            }
            // radius
            if (planetJson.contains("radius")) {
                radius = planetJson["radius"].get<float>();
            }
            basic_state state(pos,vel);
            planet planet(mass, radius, state);
            planet.name = name;

            // Añadir a la lista
            engine.emplace_planet(planet);
        }
    }

    // 6. Leer y almacenar "ships"
    if (j.contains("ships") && j["ships"].is_array()) {
        for (auto& shipJson : j["ships"]) {
            std::vector<float> pos;
            std::vector<float> vel;
            std::vector<float> dir;
            std::vector<float> dimensions;
            std::vector<float> inertia_tensor;
            std::string name;
            UnitDesignation designation;
            EntityClass entityClass;
            float mass;

            // name
            if (shipJson.contains("name")) {
                name = shipJson["name"].get<std::string>();
            }
            // position
            if (shipJson.contains("position") && shipJson["position"].is_array()) {
                for (auto& val : shipJson["position"]) {
                    pos.push_back(val.get<float>());
                }
            }
            // velocity
            if (shipJson.contains("velocity") && shipJson["velocity"].is_array()) {
                for (auto& val : shipJson["velocity"]) {
                    vel.push_back(val.get<float>());
                }
            }
            // direction
            if (shipJson.contains("direction") && shipJson["direction"].is_array()) {
                for (auto& val : shipJson["direction"]) {
                    dir.push_back(val.get<float>());
                }
            }
            // size
            if (shipJson.contains("size") && shipJson["size"].is_array()) {
                for (auto& val : shipJson["size"]) {
                    dimensions.push_back(val.get<float>());
                }
            }
            // inertia_tensor
            if (shipJson.contains("inertia_tensor") && shipJson["inertia_tensor"].is_array()) {
                for (auto& val : shipJson["inertia_tensor"]) {
                    inertia_tensor.push_back(val.get<float>());
                }
            }
            // mass
            if (shipJson.contains("mass")) {
                mass = shipJson["mass"].get<float>();
            }
            // designation
            if (shipJson.contains("designation")) {
                std::string aux = shipJson["designation"].get<std::string>();
                from_string(aux, designation);
            }
            // entityClass
            if (shipJson.contains("entityClass")) {
                std::string aux = shipJson["entityClass"].get<std::string>();
                ShipClass shipClass;
                from_string(aux, shipClass);
                entityClass = shipClass;
            }

            basic_state state(pos,vel,dir);
            basic_ship ship(mass, dimensions[0], state, pos, inertia_tensor);
            ship.name = name;
            ship.designation = designation;
            ship.type = UnitType::SHIP;
            ship.entityClass = entityClass;

            // Añadir a la lista
            engine.emplace_ship(ship);
        }
    }

//     // 7. Comprobación de que hemos leído todo correctamente (opcional)
//     std::cout << "Se han leído " << stars.size() << " stars.\n";
//     std::cout << "Se han leído " << planets.size() << " planets.\n";
//     std::cout << "Se han leído " << ships.size() << " ships.\n";

//     // Ejemplo de acceso a los datos leídos
//     if (!stars.empty()) {
//         std::cout << "Primera estrella: " << stars[0].name << "\n"
//                   << "Posición X: " << stars[0].position[0] << "\n"
//                   << "Masa: "       << stars[0].mass << "\n";
//     }

//     // Aquí continuarías con la lógica de tu programa
//     return 0;
// }

}