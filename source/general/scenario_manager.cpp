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
            std::string name, variantName;
            UnitDesignation designation;
            basic_ship ship;

            // name
            if (shipJson.contains("name")) {
                name = shipJson["name"].get<std::string>();
            }            
            // Variation name
            if (shipJson.contains("Variation_name")) {
                variantName = shipJson["Variation_name"].get<std::string>();
                bool fnd = false;
                for(auto& shipTemplate : SetManager::shipTemplates){
                    if(shipTemplate.variantName.find(variantName)!= std::string::npos){
                        ship = shipTemplate;
                        fnd = true;
                        continue;
                    }
                }
                if(!fnd) std::cerr << "[ERROR]   [SM]    No se pudo encontrar la variante "<< variantName <<".\n";
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
            // designation
            if (shipJson.contains("designation")) {
                std::string aux = shipJson["designation"].get<std::string>();
                from_string(aux, designation);
            }

            basic_state state(pos,vel,dir);
            ship.entity_state = state;            
            ship.name = name;
            ship.designation = designation;
            ship.init_shape();

            // Añadir a la lista
            engine.emplace_ship(ship);
        }
    }
}


void load_set(std::string file_name){

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

    // 3. Leer y almacenar "Platforms"
    if (j.contains("Platforms")) {
        for (auto& [key, value] : j["Platforms"].items()) {
            // key  -> std::string
            // value -> json
            PlatformClass clase;
            from_string(key, clase);
            if(clase==PlatformClass::UNKNOWN) {
                std::cerr << "[ERROR]   [SM]    SHIP Family not defined.\n";
                continue;
            }

            for (auto& family : value) {
                    std::string familyName, manufacturer, generation, information;
                    
                if (family.contains("Family_name"))     familyName = family["Family_name"].get<std::string>();
                if (family.contains("Manufacturer"))    manufacturer = family["Manufacturer"].get<std::string>();
                if (family.contains("Generation"))      generation = family["Generation"].get<std::string>();
                if (family.contains("Information"))     information = family["Information"].get<std::string>();

                if (family.contains("Variants")){
                    for (auto& variant : family["Variants"]) {
                        basic_ship shipTemplate;
                        shipTemplate.type = UnitType::PLATFORM;
                        shipTemplate.entityClass = clase;
                        shipTemplate.familyName = familyName;
                        shipTemplate.manufacturer = manufacturer;
                        shipTemplate.generation = generation;
                        shipTemplate.information = information;

                        if (variant.contains("Variant_name")) shipTemplate.variantName = variant["Variant_name"].get<std::string>();

                        if (variant.contains("Structure")) {
                            f_vector szv;
                            shipTemplate.chassis.name = shipTemplate.variantName+"_CHASSIS";
                            for (auto& sz : variant["Structure"]["Size"]) szv.push_back(sz.get<float>());
                            shipTemplate.main_dimensions = szv;
                            shipTemplate.mass = variant["Structure"]["Wheigth"].get<float>();
                        }
                        if (variant.contains("Generators")) {
                            for (auto& gen : variant["Generators"]) {
                                engines_base generator;
                                generator.name = gen["Name"].get<std::string>();
                                generator.type = gen["Type"].get<std::string>();
                                generator.purpose = "GENERATOR";
                                generator.purposeI = enginePurpose::GENERATOR;
                                generator.maxGeneration = gen["Generator_capacity"].get<float>();
                                shipTemplate.enginesV.push_back(generator);
                            }
                        }
                        if (variant.contains("Sensors")) {
                            for (auto& sen : variant["Sensors"]) {
                                sensors_base sensor;
                                sensor.name = sen["Name"].get<std::string>();
                                sensor.type = sen["Type"].get<std::string>();
                                sensor.sensitivity = sen["Sensitivity"].get<float>();
                                sensor.amplitude = sen["Amplitude"].get<float>();
                                shipTemplate.sensorsV.push_back(sensor);
                            }
                        }
                        if (variant.contains("Armament_bays")) {
                            for (auto& bay : variant["Armament_bays"]) {
                                cargoBay_base cargoBay;
                                cargoBay.name = bay["Name"].get<std::string>();
                                cargoBay.type = bay["Type"].get<std::string>();
                                cargoBay.spaces = bay["Spaces"].get<int>();
                                shipTemplate.baysV.push_back(cargoBay);
                            }
                        }

                        SetManager::shipTemplates.push_back(shipTemplate);
                    }
                }
                
            }
        }
    }

    // 3. Leer y almacenar "Ships"
    if (j.contains("Ships")) {
        for (auto& [key, value] : j["Ships"].items()) {
            // key  -> std::string
            // value -> json
            ShipClass clase;
            from_string(key, clase);
            if(clase==ShipClass::UNKNOWN) {
                std::cerr << "[ERROR]   [SM]    SHIP Family not defined.\n";
                continue;
            }

            for (auto& family : value) {
                    std::string familyName, manufacturer, generation, information;
                    
                if (family.contains("Family_name"))     familyName = family["Family_name"].get<std::string>();
                if (family.contains("Manufacturer"))    manufacturer = family["Manufacturer"].get<std::string>();
                if (family.contains("Generation"))      generation = family["Generation"].get<std::string>();
                if (family.contains("Information"))     information = family["Information"].get<std::string>();

                if (family.contains("Variants")){
                    for (auto& variant : family["Variants"]) {
                        basic_ship shipTemplate;

                        shipTemplate.type = UnitType::SHIP;
                        shipTemplate.entityClass = clase;
                        shipTemplate.familyName = familyName;
                        shipTemplate.manufacturer = manufacturer;
                        shipTemplate.generation = generation;
                        shipTemplate.information = information;

                        if (variant.contains("Variant_name")) shipTemplate.variantName = variant["Variant_name"].get<std::string>();

                        if (variant.contains("Chassis")) {
                            f_vector szv,inv;
                            shipTemplate.chassis.name = variant["Chassis"]["Name"].get<std::string>();
                            for (auto& sz : variant["Chassis"]["Size"]) szv.push_back(sz.get<float>());
                            shipTemplate.main_dimensions = szv;
                            for (auto& sz : variant["Chassis"]["MainInertia"]) inv.push_back(sz.get<float>());
                            shipTemplate.main_dimensions = inv;
                            shipTemplate.mass = variant["Chassis"]["Wheigth"].get<float>();
                        }
                        if (variant.contains("Engines")) {
                            for (auto& gen : variant["Engines"]) {
                                engines_base generator;
                                generator.name = gen["Name"].get<std::string>();
                                generator.type = gen["Type"].get<std::string>();
                                generator.purpose = gen["Purpose"].get<std::string>();
                                from_string(generator.purpose, generator.purposeI);

                                generator.fuelCapacity =        gen["Fuel_capacity"].get<float>();
                                generator.thrustEfficiency =    gen["Thrust_efficiency"].get<float>();
                                generator.maxThrust =           gen["Thrust"].get<float>();
                                generator.maxRotationalThrust = gen["Rotationa_thrust"].get<float>();
                                generator.generatorEfficiency = gen["Generator_efficiency"].get<float>();
                                generator.maxGeneration =       gen["Generator_capacity"].get<float>();
                                shipTemplate.enginesV.push_back(generator);
                            }
                        }
                        if (variant.contains("Sensors")) {
                            for (auto& sen : variant["Sensors"]) {
                                sensors_base sensor;
                                sensor.name = sen["Name"].get<std::string>();
                                sensor.type = sen["Type"].get<std::string>();
                                sensor.sensitivity = sen["Sensitivity"].get<float>();
                                sensor.amplitude = sen["Amplitude"].get<float>();
                                shipTemplate.sensorsV.push_back(sensor);
                            }
                        }
                        if (variant.contains("Armament_bays")) {
                            for (auto& bay : variant["Armament_bays"]) {
                                cargoBay_base cargoBay;
                                cargoBay.name = bay["Name"].get<std::string>();
                                cargoBay.type = bay["Type"].get<std::string>();
                                cargoBay.spaces = bay["Spaces"].get<int>();
                                shipTemplate.baysV.push_back(cargoBay);
                            }
                        }

                        SetManager::shipTemplates.push_back(shipTemplate);
                    }
                }
                
            }
        }
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