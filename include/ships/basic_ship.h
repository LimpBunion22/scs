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

#define STEADY 1
#define THRUST 2
#define ROTATION 3

#define INVALID -1
#define DORMANT 0
// #define STEADY 1
#define EXECUTING 2
#define DONE 3

#define MAX_POWER 1
#define MIN_CONSUMPTION 0

class flight_plan;

class basic_ship : public e_base
{
public:
    friend class flight_plan;

    //Definition
    int ship_class = DESTROYER;
    double fuel_consumption = 10;
    double max_thrust_force = 100;
    f_vector max_rotation_force = {10, 100, 100};

    //Status
    double fuel = 100;
    int comms_status = 1;
    int sensors_status = 1;
    int reactor_status = 1;
    int engines_status = 1;
    int weapons_status = 1;
    int flight_plan_status = INVALID;

    //Configurations
    int rotation_consumption_mode = MAX_POWER;
    flight_plan * selected_fight_plan = nullptr;

    // Graphics
    double fig_heigh = 20.0;
    double fig_width = 8.0;
    sf::Color fig_color = sf::Color(0, 50, 255);
    std::string name = "UNAMED SHIP";

private:
    // Graphics
    sf::VertexArray shape;
    sf::Text positionText;

public:
    // basic_ship() = delete;
    // basic_ship(double mass, double size_r, basic_state entity_state):e_base(mass, size_r, entity_state){init_shape();};
    basic_ship(double mass, double size_r, basic_state entity_state, f_vector main_dimensions, f_vector inertia_tensor) : e_base(mass, size_r, entity_state, main_dimensions, inertia_tensor) { init_shape(); };
    basic_ship(const basic_ship &rh) : e_base(rh) { init_shape(); };
    basic_ship(basic_ship &&rh) : e_base(std::move(rh)), shape(std::move(rh.shape)) {};

    // Graphics
    void draw(sf::RenderWindow &window, float currentZoom) override;

private:
    // Graphics
    void init_shape();
    void update_shape(double currentZoom = 1.0);
};

class flight_plan
{
public:
    std::string designation;
    std::string status = "EMPTY";
    double fuel_consumed = 0;
    double fuel_estimation = 0;

    f_vector position_margin;
    f_vector velocity_margin;

private:
    class flight_segment;
    basic_ship *_owner = nullptr;
    std::vector<flight_segment> segments;

public:
    flight_plan(basic_ship *owner, std::string name) : _owner(owner), designation(name) {};
    flight_plan(const flight_plan &rh) : designation(rh.designation), status(rh.status), fuel_consumed(rh.fuel_consumed), fuel_estimation(rh.fuel_estimation), _owner(rh._owner), segments(rh.segments) {};
    flight_plan(flight_plan &&rh) : designation(rh.designation), status(rh.status), fuel_consumed(rh.fuel_consumed), fuel_estimation(rh.fuel_estimation), _owner(rh._owner), segments(std::move(rh.segments)) {};

    void emplace_thrust_segment(double start_time, double end_time, double engine_thrust, basic_state expected_entry_state, basic_state expected_output_state);
    void emplace_rotation_segment(double start_time, double end_time, basic_state expected_entry_state, basic_state expected_output_state);

    void configure_segment(int segment_index, lamda_func pos_lamb, lamda_func vel_lamb, f_vector e_pos, f_vector e_vel, f_vector o_pos, f_vector o_vel);
    void configure_segment(int segment_index, lamda_func pos_lamb, lamda_func vel_lamb, f_vector e_pos, f_vector e_vel, f_vector o_pos, f_vector o_vel, f_vector pos_marg, f_vector vel_marg);

    void start_fligh_plan();
    int evaluate_flight_plan();
    /*
    Quiero dejar guardad la trayectoria proyectada, pero con que precision? Si guardo por km, y tenemos billones... pero que pasa cuando nos acercamos a objetos pequenos?
    El plan de vuelo esta siempre activo? hay otro modo de vuelo? guardamos planes antiguos?

    Nuevo plan, segementos en base a la acelaracion, puntos de netrada y salida y velocidades, errores aceptables, todfo basado en la celeracion maxima,
    eso soluciona los objectos en las cortas distancias y la memoria en las largas. COlor con velocidad?
    */

private:
    class flight_segment
    {
    public:

        basic_state expected_entry_state;
        basic_state expected_output_state;

        f_vector position_margin;
        f_vector velocity_margin;

        double start_time;
        double end_time;

        double elap_time = 0;

        int segment_type = STEADY;
        double engine_thrust;
        f_vector rotation_thrust;
        

    private:
        flight_plan *_owner = nullptr;

        lamda_func _expected_position_lambda;
        lamda_func _expected_velocity_lambda;

    public:
        flight_segment(flight_plan *owner):_owner(owner){};
        // flight_segment(const flight_segment &rh);
        // flight_segment(flight_segment &&rh);

        void set_course_lambdas(lamda_func pos_lambda, lamda_func vel_lambda);

        void start_segment();
        int evaluate_segment();

        f_vector get_position_error();
        f_vector get_velocity_error();

    private:
        bool check_boundaries();
    };
};

#endif // BASIC_SHIP_H