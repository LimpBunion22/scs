#ifndef LOG_WINDOW_H
#define LOG_WINDOW_H

#include <SFML/Graphics.hpp>
#include <sstream>
#include <vector>
#include <imgui.h>
#include <imgui-SFML.h>

class log_window_handler{
    public:
        const int C_WINDOW_WIDTH = 1050;
        const int C_WINDOW_HEIGH = 1680;

        sf::RenderWindow window_log;

    public:
        log_window_handler();

        bool manage_events(sf::Time elapTime);

        ~log_window_handler();

};

#endif