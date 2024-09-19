#ifndef LOG_WINDOW_H
#define LOG_WINDOW_H

#include <SFML/Graphics.hpp>
#include <sstream>
#include <vector>
#include <queue>
#include <utility>
#include <imgui.h>
#include <imgui-SFML.h>

#include <common.h>
#include <child_windows.h>

class log_window_handler{
    public:
        const int C_WINDOW_WIDTH = 1050;
        const int C_WINDOW_HEIGH = 1680;

        sf::RenderWindow window_log;
        std::queue<std::pair<int,void*>> custom_events;
    private:
        std::vector<std::pair<std::string, int>> logs;

        std::map<std::string, ship_window> child_windows;

    public:
        log_window_handler();

        bool manage_events(sf::Time elapTime);
        void logMessage(const std::string& message, int severity);

        ~log_window_handler();

    private:
        ImVec4 getSeverityColor(int severity);

};


#endif