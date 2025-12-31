
#include <physic_engine.h>
#include <cmath> 


void physic_engine::emplace_planet(planet &new_planet){
    if(planetsV.size()+1>planetsV.capacity())
        planetsV.reserve(planetsV.capacity()+32);
    planetsV.emplace_back(new_planet);
}

void physic_engine::emplace_ship(basic_ship &new_entity){
    if(shipsV.size()+1>shipsV.capacity())
        shipsV.reserve(shipsV.capacity()+128);
    shipsV.push_back(new_entity);
}

void physic_engine::run_step(){

    if(pause == true) return;

    run_planets();
    run_ships();
    inGameTime += step;
}

void physic_engine::run_planets(){

    for (auto& planetA : planetsV) {
        double Fx = 0;
        double Fy = 0;
        double x = planetA.entity_state.position[0];
        double y = planetA.entity_state.position[1];
        for(const auto& planetB : planetsV){
            if(&planetA != &planetB){
                double r_x = 1000*(planetB.entity_state.position[0] - x);
                double r_y = 1000*(planetB.entity_state.position[1] - y);
                double r2 = r_x*r_x + r_y*r_y;
                double dir = std::atan2(r_y,r_x);
                double F = G*planetB.mass/r2;
                Fx += F*std::cos(dir);
                Fy += F*std::sin(dir);
            }            
        }
        planetA.entity_state.velocity[0] += Fx*step/1000;
        planetA.entity_state.velocity[1] += Fy*step/1000;
        planetA.entity_state.position[0] += planetA.entity_state.velocity[0]*step;
        planetA.entity_state.position[1] += planetA.entity_state.velocity[1]*step;
    }
}

void physic_engine::run_ships(){

    for (auto& ship : shipsV) {
        double Fx = 0;
        double Fy = 0;
        double x = ship.entity_state.position[0];
        double y = ship.entity_state.position[1];
        for(auto& planet: planetsV){
            double r_x = 1000*(planet.entity_state.position[0] - x);
            double r_y = 1000*(planet.entity_state.position[1] - y);
            double r2 = r_x*r_x + r_y*r_y;
            double dir = std::atan2(r_y,r_x);
            double F = G*planet.mass/r2;
            Fx += F*std::cos(dir);
            Fy += F*std::sin(dir);
        }
        ship.entity_state.velocity[0] += Fx*step/1000;
        ship.entity_state.velocity[1] += Fy*step/1000;
        ship.entity_state.position[0] += ship.entity_state.velocity[0]*step;
        ship.entity_state.position[1] += ship.entity_state.velocity[1]*step;
    }    
}

sf::VertexArray physic_engine::evaluate_current_trajectory(basic_ship * ship, int max_iterations){

    sf::VertexArray line(sf::LinesStrip);
    // basic_ship * ship = nullptr;
    // for (auto& ship:shipsV){
    //     if(ship.name == ship_name){
    //         ship = ship_ptr;
    //         break;
    //     }
    // }

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