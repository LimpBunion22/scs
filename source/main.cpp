#include <SFML/Graphics.hpp>
#include <sstream>
#include <iostream>

#include <scenario_manager.h>
// #include <planets.h>
// #include <tactical_window.h>
#include <log_window.h>
// #include <physic_engine.h>
// #include <basic_entities.h>
// #include <basic_ship.h>

int main() {
    
    std::cout << "      ---WELCOME TO SPACE COMBAT SIMULATOR---" << std::endl;
    std::cout << std::endl << "Starting boot process..." << std::endl;
    // Fuente para mostrar el texto (escala)

    std::cout << "  [INFO]   Loading font..." << std::endl;
    sf::Font font;
    if (!font.loadFromFile("../utils/arial.ttf")) {
        std::cout << "[ERROR]   Fail to load font" << std::endl;
        return -1;  // Cargar una fuente para mostrar el texto
    }
    e_base::init_font(font);
    std::cout << "  [INFO]   Font loaded" << std::endl;
    
    sf::Clock master_clock;
    master_clock.restart();

    std::cout << "  [INFO]   Starting engine..." << std::endl;
    physic_engine engine(&master_clock);
    std::cout << "  [INFO]   Engine started" << std::endl;

    std::cout << "  [INFO]   Starting log window..." << std::endl;
    log_window_handler log_window(&engine);
    std::cout << "  [INFO]   Log window started" << std::endl;

    std::cout << "  [INFO]   Starting tactical window..." << std::endl;
    tactical_window_handler tactical_window(font, &log_window);
    std::cout << "  [INFO]   Tactical window started" << std::endl;
    engine.step = 3600;

    std::cout << "  [INFO]   Loading test scenario..." << std::endl;
    load_scenario("../scenarios/sol.json", tactical_window, engine);
    std::cout << "  [INFO]   Test scenario loaded" << std::endl;

    // // Velocidad del triángulo
    // float V = 20.0f; // Velocidad en píxeles por segundo

    // // Crear el triángulo
    // sf::ConvexShape triangle;
    // triangle.setPointCount(3); // El triángulo tiene 3 puntos
    // triangle.setPoint(0, sf::Vector2f(0.f, 50.f));  // Primer punto
    // triangle.setPoint(1, sf::Vector2f(50.f, 50.f)); // Segundo punto
    // triangle.setPoint(2, sf::Vector2f(25.f, 0.f));  // Tercer punto
    // triangle.setFillColor(sf::Color::White);         // Color blanco
    // triangle.setPosition(100.f, 100.f);              // Posición inicial

    // //Planetas
    // std::vector<float> pos = {0.0, 0.0, 0.0};
    // std::vector<float> vel = {0.0, 0.0, 0.0};
    // basic_state state(pos);
    // planet sol(1.98e30, 6.98e5, state);
    // sol.name = "SOL";
    // sol.star();

    // pos = {5.79e7,0.0,0.0};
    // vel = {0.0,47.87,0.0};
    // state = basic_state(pos,vel);
    // planet mercurio(3.30e23, 2.44e3, state);
    // mercurio.name = "MERCURIO";

    // pos = {1.08e8,0.0,0.0};
    // vel = {0.0,35.02,0.0};
    // state = basic_state(pos,vel);
    // planet venus(4.87e24, 6.05e3, state);
    // venus.name = "VENUS";

    // pos = {1.5e8,0.0,0.0};
    // vel = {0.0,29.78,0.0};
    // state = basic_state(pos,vel);
    // planet tierra(5.97e24, 6.05e3, state);
    // tierra.name = "TIERRA";

    // pos = {2.28e8,0.0,0.0};
    // vel = {0.0,24.07,0.0};
    // state = basic_state(pos,vel);
    // planet marte(6.42e23, 3.39e3, state);
    // marte.name = "MARTE";

    // pos = {7.78e8,0.0,0.0};
    // vel = {0.0,13.07,0.0};
    // state = basic_state(pos,vel);
    // planet jupiter(1.90e27, 7.15e4, state);
    // jupiter.name = "JUPITER";

    // pos = {1.43e9,0.0,0.0};
    // vel = {0.0,9.68,0.0};
    // state = basic_state(pos,vel);
    // planet saturno(5.68e26, 6.03e4, state);
    // saturno.name = "SATURNO";

    // pos = {2.87e9,0.0,0.0};
    // vel = {0.0,6.80,0.0};
    // state = basic_state(pos,vel);
    // planet urano(8.86e25, 2.54e4, state);
    // urano.name = "URANO";

    // pos = {4.50e9,0.0,0.0};
    // vel = {0.0,5.43,0.0};
    // state = basic_state(pos,vel);
    // planet neptuno(1.02e26, 2.47e4, state);
    // neptuno.name = "NEPTUNO";

    // tactical_window.emplace_planet(&sol);
    // tactical_window.emplace_planet(&mercurio);
    // tactical_window.emplace_planet(&venus);
    // tactical_window.emplace_planet(&tierra);
    // tactical_window.emplace_planet(&marte);
    // tactical_window.emplace_planet(&jupiter);
    // tactical_window.emplace_planet(&saturno);
    // tactical_window.emplace_planet(&urano);
    // tactical_window.emplace_planet(&neptuno);

    // engine.emplace_planet(&sol);
    // engine.emplace_planet(&mercurio);
    // engine.emplace_planet(&venus);
    // engine.emplace_planet(&tierra);
    // engine.emplace_planet(&marte);
    // engine.emplace_planet(&jupiter);
    // engine.emplace_planet(&saturno);
    // engine.emplace_planet(&urano);
    // engine.emplace_planet(&neptuno);

    // // Naves
    // float mass = 1e5;
    // pos = {1.9e8,0.0,0.0};
    // vel = {0.0,29.78,10.0};
    // std::vector<float> dir = {0.0, 0.0, 0.0};
    // std::vector<float> main_dim = {100, 50, 50};
    // std::vector<float> inertia_tensor = {mass*(main_dim[1]*main_dim[1] + main_dim[2]*main_dim[2])/12, 0, 0,  0, mass*(main_dim[0]*main_dim[0] + main_dim[2]*main_dim[2])/12,  0, 0, mass*(main_dim[1]*main_dim[1] + main_dim[0]*main_dim[0])/12};
    // state = basic_state(pos,vel,dir);
    // basic_ship ship1(mass, 0.5, state, main_dim, inertia_tensor);
    // ship1.name = "Santa Maria";
    
    // mass = 1e6;
    // pos = {0.9e8,0.0,0.0};
    // vel = {0.0,36.78,0.0};
    // dir = {0.0, 0.0, 0.0};
    // main_dim = {500, 80, 80};
    // inertia_tensor = {mass*(main_dim[1]*main_dim[1] + main_dim[2]*main_dim[2])/12, 0, 0,  0, mass*(main_dim[0]*main_dim[0] + main_dim[2]*main_dim[2])/12,  0, 0, mass*(main_dim[1]*main_dim[1] + main_dim[0]*main_dim[0])/12};
    // state = basic_state(pos,vel,dir);
    // basic_ship ship2(mass, 0.5, state, main_dim, inertia_tensor);
    // ship2.name = "Enterprise";

    // tactical_window.emplace_ship(&ship1);
    // tactical_window.emplace_ship(&ship2);

    // engine.emplace_ship(&ship1);
    // engine.emplace_ship(&ship2);

    // tactical_window.draw_gravity = true;

    // Reloj para calcular el tiempo transcurrido
    sf::Clock clock;

    log_window.logMessage("Probando rojo", RED);
    log_window.logMessage("Probando amarillo", YELLOW);
    log_window.logMessage("Probando verde", GREEN);

    while (true) {

        // Calcular el tiempo transcurrido
        sf::Time elapTime = clock.restart();
        float deltaTime = elapTime.asSeconds();

        if (tactical_window.manage_events(deltaTime) || log_window.manage_events(elapTime))
            return 0;

        // Mover el triángulo en línea recta (en este caso hacia la derecha)
        // triangle.move(V * deltaTime, 0.f); // Mueve el triángulo con velocidad V en el eje X

        tactical_window.draw_map();
        tactical_window.draw_hud();
        for(int i = 0; i < 2; i++)
            engine.run_step();
    }

    return 0;
}
