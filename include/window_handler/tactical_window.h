#ifndef TACTICAL_WINDOW_H
#define TACTICAL_WINDOW_H

#include <SFML/Graphics.hpp>
#include <sstream>
#include <vector>

#include <planets.h>


class tactical_window_handler{

    public:
        const int C_WINDOW_WIDTH = 1920;
        const int C_WINDOW_HEIGH = 1080;
        const float C_ORIGINAL_ASPECT_RATIO = C_WINDOW_WIDTH/C_WINDOW_HEIGH;
        const float C_ZOOM_INCREMENT = 0.9f;  // Para acercar o alejar (menos de 1 para zoom in, mayor que 1 para zoom out)
        const float C_VIEW_SPEED = 300.0f; // píxeles por segundo

        // Grafical elements 
        sf::RenderWindow window_tactical;
        sf::View map_view;
        sf::View hud_view;
        sf::Font font;
        sf::Text positionText;
        sf::Text scaleText;
        sf::RectangleShape referenceLine;
        std::vector<planet*> planets_ptr;
        // std::vector<ship> ships;

        // Configuration variables
        float aspectRatio = C_ORIGINAL_ASPECT_RATIO;
        float currentZoom = 1.0f;
        float window_width = C_WINDOW_WIDTH;
        float window_heigh = C_WINDOW_HEIGH;

        // HUD elements
        float referenceLineLength = 100.0f;

    public:
        tactical_window_handler() = delete;
        tactical_window_handler(const sf::Font in_font);

        void emplace_planet(planet * new_planet_ptr);

        bool manage_events(float deltaTime);

        void draw_map();
        void draw_hud();

        ~tactical_window_handler();

};

#endif