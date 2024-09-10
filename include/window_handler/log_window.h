#ifndef LOG_WINDOW_H
#define LOG_WINDOW_H

#include <SFML/Graphics.hpp>
#include <sstream>
#include <vector>
#include <imgui.h>
#include <imgui-SFML.h>
#include <common.h>

class log_window_handler{
    public:
        const int C_WINDOW_WIDTH = 1050;
        const int C_WINDOW_HEIGH = 1680;

        sf::RenderWindow window_log;
    private:
        std::vector<std::pair<std::string, int>> logs;

    public:
        log_window_handler();

        bool manage_events(sf::Time elapTime);
        void logMessage(const std::string& message, int severity);

        ~log_window_handler();

    private:
        ImVec4 getSeverityColor(int severity);

};

#endif