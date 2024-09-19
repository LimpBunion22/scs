
#include <physic_engine.h>
#include <cmath> 


void physic_engine::emplace_planet(planet * new_planet_ptr){
    if(planets_ptr.size()+1>planets_ptr.capacity())
        planets_ptr.reserve(planets_ptr.capacity()+32);
    planets_ptr.emplace_back(new_planet_ptr);
}

void physic_engine::emplace_small_entity(e_base * new_entity_ptr){
    if(small_entities_ptr.size()+1>small_entities_ptr.capacity())
        small_entities_ptr.reserve(small_entities_ptr.capacity()+128);
    small_entities_ptr.emplace_back(new_entity_ptr);
}

void physic_engine::run_step(){
    run_planets();
    run_small_entities();
}

void physic_engine::run_planets(){

    for (auto ptr : planets_ptr) {
        float Fx = 0;
        float Fy = 0;
        float x = ptr->entity_state.position[0];
        float y = ptr->entity_state.position[1];
        for(auto ptr2 : planets_ptr){
            if(ptr != ptr2){
                float r_x = 1000*(ptr2->entity_state.position[0] - x);
                float r_y = 1000*(ptr2->entity_state.position[1] - y);
                float r2 = r_x*r_x + r_y*r_y;
                float dir = std::atan2(r_y,r_x);
                float F = G*ptr2->mass/r2;
                Fx += F*std::cos(dir);
                Fy += F*std::sin(dir);
            }            
        }
        ptr->entity_state.velocity[0] += Fx*step/1000;
        ptr->entity_state.velocity[1] += Fy*step/1000;
        ptr->entity_state.position[0] += ptr->entity_state.velocity[0]*step;
        ptr->entity_state.position[1] += ptr->entity_state.velocity[1]*step;
    }
}

void physic_engine::run_small_entities(){

    for (auto small_ptr : small_entities_ptr) {
        float Fx = 0;
        float Fy = 0;
        float x = small_ptr->entity_state.position[0];
        float y = small_ptr->entity_state.position[1];
        for(auto planet_ptr : planets_ptr){
            float r_x = 1000*(planet_ptr->entity_state.position[0] - x);
            float r_y = 1000*(planet_ptr->entity_state.position[1] - y);
            float r2 = r_x*r_x + r_y*r_y;
            float dir = std::atan2(r_y,r_x);
            float F = G*planet_ptr->mass/r2;
            Fx += F*std::cos(dir);
            Fy += F*std::sin(dir);
        }
        small_ptr->entity_state.velocity[0] += Fx*step/1000;
        small_ptr->entity_state.velocity[1] += Fy*step/1000;
        small_ptr->entity_state.position[0] += small_ptr->entity_state.velocity[0]*step;
        small_ptr->entity_state.position[1] += small_ptr->entity_state.velocity[1]*step;
    }    
}