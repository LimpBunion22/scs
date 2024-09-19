#ifndef PHYSIC_ENGINE_H
#define PHYSIC_ENGINE_H

#include <common.h>
#include <basic_entities.h>
#include <planets.h>

class physic_engine{
    public:
        float step = 1.0;
    
    private:
        std::vector<planet*> planets_ptr;
        std::vector<e_base*> small_entities_ptr;

    public:
        physic_engine(){planets_ptr.reserve(32);small_entities_ptr.reserve(128);};

        void emplace_planet(planet * new_planet_ptr);
        void emplace_small_entity(e_base * new_entity_ptr);
        void run_step();

    private:
        void run_planets();
        void run_small_entities();


};

#endif