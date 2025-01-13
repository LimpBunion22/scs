
#include <physic_engine.h>
#include <cmath> 


void physic_engine::emplace_planet(planet * new_planet_ptr){
    if(planets_ptr.size()+1>planets_ptr.capacity())
        planets_ptr.reserve(planets_ptr.capacity()+32);
    planets_ptr.emplace_back(new_planet_ptr);
}

void physic_engine::emplace_ship(basic_ship * new_entity_ptr){
    if(ships_ptr.size()+1>ships_ptr.capacity())
        ships_ptr.reserve(ships_ptr.capacity()+128);
    ships_ptr.emplace_back(new_entity_ptr);
}

void physic_engine::run_step(){
    run_planets();
    run_ships();
}

void physic_engine::run_planets(){

    for (auto ptr : planets_ptr) {
        double Fx = 0;
        double Fy = 0;
        double x = ptr->entity_state.position[0];
        double y = ptr->entity_state.position[1];
        for(auto ptr2 : planets_ptr){
            if(ptr != ptr2){
                double r_x = 1000*(ptr2->entity_state.position[0] - x);
                double r_y = 1000*(ptr2->entity_state.position[1] - y);
                double r2 = r_x*r_x + r_y*r_y;
                double dir = std::atan2(r_y,r_x);
                double F = G*ptr2->mass/r2;
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

void physic_engine::run_ships(){

    for (auto small_ptr : ships_ptr) {
        double Fx = 0;
        double Fy = 0;
        double x = small_ptr->entity_state.position[0];
        double y = small_ptr->entity_state.position[1];
        for(auto planet_ptr : planets_ptr){
            double r_x = 1000*(planet_ptr->entity_state.position[0] - x);
            double r_y = 1000*(planet_ptr->entity_state.position[1] - y);
            double r2 = r_x*r_x + r_y*r_y;
            double dir = std::atan2(r_y,r_x);
            double F = G*planet_ptr->mass/r2;
            Fx += F*std::cos(dir);
            Fy += F*std::sin(dir);
        }
        small_ptr->entity_state.velocity[0] += Fx*step/1000;
        small_ptr->entity_state.velocity[1] += Fy*step/1000;
        small_ptr->entity_state.position[0] += small_ptr->entity_state.velocity[0]*step;
        small_ptr->entity_state.position[1] += small_ptr->entity_state.velocity[1]*step;
    }    
}

sf::VertexArray physic_engine::evaluate_current_trajectory(std::string ship_name, int max_iterations){

    sf::VertexArray line(sf::LinesStrip);
    basic_ship * ship = nullptr;
    for (auto ship_ptr:ships_ptr){
        if(ship_ptr->name == ship_name){
            ship = ship_ptr;
            break;
        }
    }

    if(ship == nullptr){
        log_queue.push(std::pair<std::string,int>("Unknown ship",YELLOW));
        return line;
    }
    return line;

    // if(ship->flight_plan_status == INVALID || ship->flight_plan_status == DONE){
    //     for(int i = 0; i<max_iterations; i++){

    //     }
    // }

    // for(auto & segment: )
    
}