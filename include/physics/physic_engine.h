#ifndef PHYSIC_ENGINE_H
#define PHYSIC_ENGINE_H

#include <SFML/Graphics.hpp>
#include <queue>

#include <common.h>
#include <basic_ship.h>
#include <planets.h>

class physic_engine{
    public:
        float step = 1.0;

        std::queue<std::pair<std::string,int>> log_queue;
        
        sf::Clock* master_clock;
    
    private:
        std::vector<planet*> planets_ptr;
        std::vector<basic_ship*> ships_ptr;

    public:
        physic_engine(sf::Clock* clock):master_clock(clock){planets_ptr.reserve(32);ships_ptr.reserve(128);};

        void emplace_planet(planet * new_planet_ptr);
        void emplace_ship(basic_ship * new_entity_ptr);
        void run_step();

        sf::VertexArray evaluate_current_trajectory(std::string ship_name, int max_iterations);

    private:
        void run_planets();
        void run_ships();
};

#endif