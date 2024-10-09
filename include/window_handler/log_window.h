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
#include <physic_engine.h>

class log_window_handler{
    public:
        const int C_WINDOW_WIDTH = 1680;
        const int C_WINDOW_HEIGH = 1050;

        sf::RenderWindow window_log;
        std::queue<std::pair<int,void*>> custom_events;
        std::map<std::string,sf::VertexArray> paths;
    private:
        std::vector<std::pair<std::string, int>> logs;

        std::map<std::string, ship_window> child_windows;

        physic_engine *engine = nullptr;

    public:
        log_window_handler(physic_engine * in_engine);

        bool manage_events(sf::Time elapTime);
        void logMessage(const std::string& message, int severity);

        ~log_window_handler();

    private:
        ImVec4 getSeverityColor(int severity);

};


#endif