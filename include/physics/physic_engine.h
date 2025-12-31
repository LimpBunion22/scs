#ifndef PHYSIC_ENGINE_H
#define PHYSIC_ENGINE_H

#include <SFML/Graphics.hpp>
#include <queue>

#include <common.h>
#include <basic_ship.h>
#include <planets.h>

class physic_engine{
    friend class tactical_window_handler;
    friend class log_window_handler;
    friend class ship_window;
    public:
        double inGameTime = 0.0;
        float step = 1.0;
        bool pause = true;

        std::queue<std::pair<std::string,int>> log_queue;
        
        sf::Clock* master_clock;
    
    private:
        std::vector<planet> planetsV;
        std::vector<basic_ship> shipsV;

    public:
        physic_engine(sf::Clock* clock):master_clock(clock){planetsV.reserve(32);shipsV.reserve(128);};

        void emplace_planet(planet &new_planet);
        void emplace_ship(basic_ship &new_entity);
        void run_step();

        sf::VertexArray evaluate_current_trajectory(basic_ship * ship, int max_iterations);

    private:
        void run_planets();
        void run_ships();
};

#endif