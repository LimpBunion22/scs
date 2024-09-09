#include <SFML/Graphics.hpp>
#include <sstream>

#include <planets.h>
#include <tactical_window.h>
#include <log_window.h>

int main() {
    
    // Fuente para mostrar el texto (escala)
    sf::Font font;
    if (!font.loadFromFile("../utils/arial.ttf")) {
        return -1;  // Cargar una fuente para mostrar el texto
    }
    
    tactical_window_handler tactical_window(font);
    log_window_handler log_window = log_window_handler();

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

    //Planetas
    std::vector<float> pos = {0.0, 0.0, 0.0};
    std::vector<float> vel = {0.0, 0.0, 0.0};
    std::vector<float> dir = {0.0, 0.0, 0.0};
    basic_state sol_state(pos,vel,dir);
    planet sol(1.98e30, 6.98e5, sol_state, font);
    sol.name = "SOL";

    pos = {1.1e8,2.1e7,0.0};
    vel = {0.0, 0.0, 0.0};
    dir = {0.0, 0.0, 0.0};
    basic_state terra_state(pos,vel,dir);
    planet tierra(5.92e24, 6.98e3, terra_state, font);
    tierra.name = "TIERRA";

    tactical_window.emplace_planet(&sol);
    tactical_window.emplace_planet(&tierra);

    // Reloj para calcular el tiempo transcurrido
    sf::Clock clock;

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
    }

    return 0;
}
