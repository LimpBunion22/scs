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
    
    log_window_handler log_window = log_window_handler();
    tactical_window_handler tactical_window(font, &log_window);

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
    basic_state state(pos);
    planet sol(1.98e30, 6.98e5, state, font);
    sol.name = "SOL";
    sol.star();

    pos = {5.79e7,0.0,0.0};
    state = basic_state(pos);
    planet mercurio(3.30e23, 2.44e3, state, font);
    mercurio.name = "MERCURIO";

    pos = {1.08e8,0.0,0.0};
    state = basic_state(pos);
    planet venus(4.87e24, 6.05e3, state, font);
    venus.name = "VENUS";

    pos = {1.5e8,0.0,0.0};
    state = basic_state(pos);
    planet tierra(5.97e24, 6.05e3, state, font);
    tierra.name = "TIERRA";

    pos = {2.28e8,0.0,0.0};
    state = basic_state(pos);
    planet marte(6.42e23, 3.39e3, state, font);
    marte.name = "MARTE";

    pos = {7.78e8,0.0,0.0};
    state = basic_state(pos);
    planet jupiter(1.90e27, 7.15e4, state, font);
    jupiter.name = "JUPITER";

    pos = {1.43e9,0.0,0.0};
    state = basic_state(pos);
    planet saturno(5.68e26, 6.03e4, state, font);
    saturno.name = "SATURNO";

    pos = {2.87e9,0.0,0.0};
    state = basic_state(pos);
    planet urano(8.86e25, 2.54e4, state, font);
    urano.name = "URANO";

    pos = {4.50e9,0.0,0.0};
    state = basic_state(pos);
    planet neptuno(1.02e26, 2.47e4, state, font);
    neptuno.name = "NEPTUNO";

    tactical_window.emplace_planet(&sol);
    tactical_window.emplace_planet(&mercurio);
    tactical_window.emplace_planet(&venus);
    tactical_window.emplace_planet(&tierra);
    tactical_window.emplace_planet(&marte);
    tactical_window.emplace_planet(&jupiter);
    tactical_window.emplace_planet(&saturno);
    tactical_window.emplace_planet(&urano);
    tactical_window.emplace_planet(&neptuno);
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
    }

    return 0;
}
