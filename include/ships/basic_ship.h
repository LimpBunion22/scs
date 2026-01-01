#ifndef BASIC_SHIP_H
#define BASIC_SHIP_H

#include <variant>
#include <common.h>
#include <basic_entities.h>


// Ship status
enum class UnitDesignation {
    UNKNOWN,
    NEUTRAL,
    UNFRIENDLY,
    HOSTILE,
    FRIENDLY,
    CONTROLLED
};
inline const char* to_string(UnitDesignation t) {
    switch (t) {
        case UnitDesignation::NEUTRAL:        return "NEUTRAL";
        case UnitDesignation::UNFRIENDLY:     return "UNFRIENDLY";
        case UnitDesignation::HOSTILE:        return "HOSTILE";
        case UnitDesignation::FRIENDLY:       return "FRIENDLY";
        case UnitDesignation::CONTROLLED:     return "CONTROLLED";
        default:                                return "UNKNOWN";
    }
}
inline const void from_string(std::string t, UnitDesignation &r) {    
    if(t.find("NEUTRAL")!= std::string::npos)        {r = UnitDesignation::NEUTRAL; return;}
    if(t.find("UNFRIENDLY")!= std::string::npos)     {r = UnitDesignation::UNFRIENDLY; return;}
    if(t.find("HOSTILE")!= std::string::npos)        {r = UnitDesignation::HOSTILE; return;}
    if(t.find("FRIENDLY")!= std::string::npos)       {r = UnitDesignation::FRIENDLY; return;}
    if(t.find("CONTROLLED")!= std::string::npos)     {r = UnitDesignation::CONTROLLED; return;}
    r = UnitDesignation::UNKNOWN;
}

// Ship/platform
enum class UnitType {UNKNOWN,SHIP,PLATFORM};
inline const char* to_string(UnitType t) {
    switch (t) {
        case UnitType::SHIP:            return "SHIP";
        case UnitType::PLATFORM:        return "PLATFORM";
        default:                        return "UNKNOWN";
    }
}
inline const void from_string(std::string t, UnitType &r) {    
    if(t.find("SHIP")!= std::string::npos)        {r = UnitType::SHIP; return;}
    if(t.find("PLATFORM")!= std::string::npos)    {r = UnitType::PLATFORM; return;}
    r = UnitType::UNKNOWN;
}

// Ship classes identificators
enum class ShipClass {
    UNKNOWN,
    CIVIL,
    AUXILIAR,
    PROBE,
    FIGHTER,
    CORVETTE,
    FRIGATE,
    CRUISER,
    CAPITAL,
    CARRIER
};
inline const char* to_string(ShipClass t) {
    switch (t) {
        case ShipClass::CIVIL:              return "CIVIL";
        case ShipClass::AUXILIAR:           return "AUXILIAR";
        case ShipClass::PROBE:              return "PROBE";
        case ShipClass::FIGHTER:            return "FIGHTER";
        case ShipClass::CORVETTE:           return "CORVETTE";
        case ShipClass::FRIGATE:            return "FRIGATE";
        case ShipClass::CRUISER:            return "CRUISER";
        case ShipClass::CAPITAL:            return "CAPITAL";
        case ShipClass::CARRIER:            return "CARRIER";
        default:                            return "UNKNOWN";
    }
}
inline const void from_string(std::string t, ShipClass &r) {    
    if(t.find("CIVIL")!= std::string::npos)        {r = ShipClass::CIVIL; return;}
    if(t.find("AUXILIAR")!= std::string::npos)     {r = ShipClass::AUXILIAR; return;}
    if(t.find("PROBE")!= std::string::npos)        {r = ShipClass::PROBE; return;}
    if(t.find("FIGHTER")!= std::string::npos)      {r = ShipClass::FIGHTER; return;}
    if(t.find("CORVETTE")!= std::string::npos)     {r = ShipClass::CORVETTE; return;}
    if(t.find("FRIGATE")!= std::string::npos)      {r = ShipClass::FRIGATE; return;}
    if(t.find("CRUISER")!= std::string::npos)      {r = ShipClass::CRUISER; return;}
    if(t.find("CAPITAL")!= std::string::npos)      {r = ShipClass::CAPITAL; return;}
    if(t.find("CARRIER")!= std::string::npos)      {r = ShipClass::CARRIER; return;}
    r = ShipClass::UNKNOWN;
}

// Platform classes identificators
enum class PlatformClass {
    UNKNOWN,
    CIVIL,
    OUTPOST,
    PLATFORM,
    SHIPYARD
};
inline const char* to_string(PlatformClass t) {
    switch (t) {
        case PlatformClass::CIVIL:              return "CIVIL";
        case PlatformClass::OUTPOST:            return "OUTPOST";
        case PlatformClass::PLATFORM:           return "PLATFORM";
        case PlatformClass::SHIPYARD:           return "SHIPYARD";
        default:                                return "UNKNOWN";
    }
}
inline const void from_string(std::string t, PlatformClass &r) {    
    if(t.find("CIVIL")!= std::string::npos)        {r = PlatformClass::CIVIL; return;}
    if(t.find("OUTPOST")!= std::string::npos)      {r = PlatformClass::OUTPOST; return;}
    if(t.find("PLATFORM")!= std::string::npos)     {r = PlatformClass::PLATFORM; return;}
    if(t.find("SHIPYARD")!= std::string::npos)     {r = PlatformClass::SHIPYARD; return;}
    r = PlatformClass::UNKNOWN;
}

using EntityClass = std::variant<ShipClass, PlatformClass>;


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
    UnitType type = UnitType::UNKNOWN;
    EntityClass entityClass = ShipClass::UNKNOWN;
    std::string familyName, manufacturer, generation, information;
    std::string variantName;

    
    double fuel_consumption = 10;
    double max_thrust_force = 100;
    f_vector max_rotation_force = {10, 100, 100};

    //Status
    UnitDesignation designation = UnitDesignation::UNKNOWN;

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
    basic_ship(const basic_ship &rh);
    basic_ship(basic_ship &&rh);

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