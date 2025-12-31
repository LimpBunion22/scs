#ifndef TACTICAL_WINDOW_H
#define TACTICAL_WINDOW_H

#include <SFML/Graphics.hpp>
#include <sstream>
#include <vector>

#include <planets.h>
#include <log_window.h>
#include <basic_ship.h>


class tactical_window_handler{

    public:
        const int C_WINDOW_WIDTH = 1920;
        const int C_WINDOW_HEIGH = 1080;
        const float C_ORIGINAL_ASPECT_RATIO = C_WINDOW_WIDTH/C_WINDOW_HEIGH;
        const float C_ZOOM_INCREMENT = 0.9f;  // Para acercar o alejar (menos de 1 para zoom in, mayor que 1 para zoom out)
        const float C_VIEW_SPEED = 300.0f; // píxeles por segundo

        sf::Clock* master_clock;

        // Grafical elements 
        sf::RenderWindow window_tactical;
        sf::View map_view;
        sf::View hud_view;
        sf::Font font;
        sf::Text positionText;
        sf::Text scaleText;
        sf::Text timeText;
        sf::RectangleShape referenceLine;
        std::vector<planet>* planetsV_ptr = nullptr;
        std::vector<basic_ship>* shipsV_ptr = nullptr;
        sf::Image gravity_map;
        sf::Texture heatmapTexture;
        sf::Sprite heatmapSprite;

        // Configuration variables
        float aspectRatio = C_ORIGINAL_ASPECT_RATIO;
        double currentZoom = 1.0f;
        float window_width = C_WINDOW_WIDTH;
        float window_heigh = C_WINDOW_HEIGH;
        bool draw_gravity = false;

        // HUD elements
        float referenceLineLength = 100.0f;
    private:
        log_window_handler* logger;
        physic_engine* engine;

        bool spacePressed = false;
        float lastSpace = 0.0;

    public:
        tactical_window_handler() = delete;
        tactical_window_handler(const sf::Font in_font, log_window_handler* in_logger, sf::Clock* masterClock);

        bool manage_events(float deltaTime);

        void draw_map();
        void draw_hud();

        ~tactical_window_handler();
    private:
        float _gravityFunction(float x, float y);
        void generateHeatmap(sf::Image& heatmap, const sf::FloatRect& visibleArea, double currentZoom);

};

#endif