#ifndef BASIC_SHIP_H
#define BASIC_SHIP_H

#include <common.h>
#include <basic_entities.h>

// Ship classes identificators
#define CLASS_MASK 0b11110000
#define STATION 0b00010000
#define CARRIER 0b00100000
#define BATTLESHIP 0b00110000
#define CRUISER 0b01000000
#define DESTROYER 0b01010000
#define FRIGATE 0b01100000
#define FIGHTER 0b01110000

#define STEADY 0
#define THRUST 1
#define ROTATION 2


class basic_ship : public e_base {
    public:
        float fig_heigh = 20.0;     
        float fig_width = 8.0;  
        sf::Color fig_color = sf::Color(0, 50, 255);  
        std::string name = "UNAMED SHIP"; 

        int ship_class = DESTROYER;

        float fuel = 100;
        int comms_status = 1;
        int sensors_status = 1;
        int reactor_status = 1;
        int engines_status = 1;
        int weapons_status = 1;

    private:
        sf::VertexArray shape;
        sf::Text positionText;

    public:
        basic_ship() = delete;
        basic_ship(float mass, float size_r, basic_state entity_state):e_base(mass, size_r, entity_state){init_shape();};
        basic_ship(const basic_ship &rh):e_base(rh){init_shape();};
        basic_ship(basic_ship && rh):e_base(std::move(rh)), shape(std::move(rh.shape)){};

        void draw(sf::RenderWindow & window, float currentZoom) override;

    private:
        void init_shape();
        void update_shape(float currentZoom = 1.0);
};

// class flight_plan{
//     public:
//         std::string designation;
//         std::string status = "EMPTY";
//         float fuel_consumed = 0;
//         float fuel_estimation = 0;

//     private:

//     public:
//         flight_plan();
//         flight_plan(const flight_plan &rh):designation(rh.designation),status(rh.status),fuel_consumed(rh.fuel_consumed),fuel_estimation(rh.fuel_estimation){};
//         flight_plan(flight_plan &&rh):designation(rh.designation),status(rh.status),fuel_consumed(rh.fuel_consumed),fuel_estimation(rh.fuel_estimation){};

//         void add_thrust_segment(float star_time, float end_time, float engine_thrust);
//         void add_rotation_segment(float star_time, float end_time, std::vector<float> new_orientation);
//         /*
//         Quiero dejar guardad la trayectoria proyectada, pero con que precision? Si guardo por km, y tenemos billones... pero que pasa cuando nos acercamos a objetos pequenos?
//         El plan de vuelo esta siempre activo? hay otro modo de vuelo? guardamos planes antiguos?

//         Nuevo plan, segementos en base a la acelaracion, puntos de netrada y salida y velocidades, errores aceptables, todfo basado en la celeracion maxima,
//         eso soluciona los objectos en las cortas distancias y la memoria en las largas. COlor con velocidad?
//         */

//     private:

//         class flight_segment{
//             public:
//                 std::vector<float> expected_entry_point;
//                 std::vector<float> expected_entry_velocity;

//                 std::vector<float> expected_output_point;
//                 std::vector<float> expected_velocity_point;

//                 std::vector<float> position_margin;
//                 std::vector<float> velocity_margin;

//                 float start_time;
//                 float end_time;

//                 int segment_type = STEADY;
//                 float engine_thrust;
//                 std::vector<float> rotation_thrust;

//             private:

//             public:
//                 flight_segment(float in_start_time, float in_end_time);
//                 flight_segment(const flight_segment &rh);
//                 flight_segment(flight_segment &&rh);

//                 bool check_boundaries(std::vector<float>*error_position,std::vector<float>*error_velocity);
//                 bool is_segment_active(float time);

//             private:

//         };
// };

#endif  // BASIC_SHIP_H