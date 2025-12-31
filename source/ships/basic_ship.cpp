

#include <basic_ship.h>
#include <cmath>

basic_ship::basic_ship(const basic_ship &rh) : e_base(rh) {
    init_shape();

    //Definition
    ship_class = rh.ship_class;
    fuel_consumption = rh.fuel_consumption;
    max_thrust_force = rh.max_thrust_force;
    max_rotation_force = rh.max_rotation_force;

    //Status
    fuel = rh.fuel;
    comms_status = rh.comms_status;
    sensors_status = rh.sensors_status;
    reactor_status = rh.reactor_status;
    engines_status = rh.engines_status;
    weapons_status = rh.weapons_status;
    flight_plan_status = rh.flight_plan_status;

    //Configurations
    rotation_consumption_mode = rh.rotation_consumption_mode;
    selected_fight_plan = rh.selected_fight_plan;

    // Graphics
    fig_heigh = rh.fig_heigh;
    fig_width = rh.fig_width;
    fig_color = rh.fig_color;
    name = rh.name;

    // Graphics
    shape = rh.shape;
    positionText = rh.positionText;
}

basic_ship::basic_ship(basic_ship &&rh) : e_base(std::move(rh)), 
    ship_class(std::move(rh.ship_class)),
    fuel_consumption(std::move(rh.fuel_consumption)),
    max_thrust_force(std::move(rh.max_thrust_force)),
    max_rotation_force(std::move(rh.max_rotation_force)),

    //Status
    fuel(std::move(rh.fuel)),
    comms_status(std::move(rh.comms_status)),
    sensors_status(std::move(rh.sensors_status)),
    reactor_status(std::move(rh.reactor_status)),
    engines_status(std::move(rh.engines_status)),
    weapons_status(std::move(rh.weapons_status)),
    flight_plan_status(std::move(rh.flight_plan_status)),

    //Configurations
    rotation_consumption_mode(std::move(rh.rotation_consumption_mode)),
    selected_fight_plan(std::move(rh.selected_fight_plan)),

    // Graphics
    fig_heigh(std::move(rh.fig_heigh)),
    fig_width(std::move(rh.fig_width)),
    fig_color(std::move(rh.fig_color)),
    name(std::move(rh.name)),

    // Graphics
    shape(std::move(rh.shape)),
    positionText(std::move(rh.positionText)) {}

void basic_ship::draw(sf::RenderWindow &window, float currentZoom)
{

    update_shape(currentZoom);
    window.draw(shape);

    positionText.setScale(currentZoom, currentZoom);
    sf::FloatRect textBounds = positionText.getLocalBounds();
    positionText.setOrigin(textBounds.width / 2, textBounds.height / 2);
    positionText.setString(name);
    positionText.setPosition(entity_state.position[0], entity_state.position[1] - 25 * currentZoom);
    window.draw(positionText);
}

void basic_ship::init_shape()
{
    shape = sf::VertexArray(sf::Triangles, 3);
    update_shape();

    positionText.setFont(e_base::font);
    positionText.setCharacterSize(15);
    sf::FloatRect textBounds = positionText.getLocalBounds();
    positionText.setOrigin(textBounds.width / 2, textBounds.height / 2);
    positionText.setFillColor(sf::Color::Green);
}

void basic_ship::update_shape(double currentZoom)
{

    double cos = std::cos(entity_state.direction[0]);
    double sin = std::sin(entity_state.direction[0]);

    double scaled_heigh = fig_heigh * currentZoom;
    double sup_x = scaled_heigh * cos + entity_state.position[0];
    double sup_y = scaled_heigh * sin + entity_state.position[1];
    shape[0].position = sf::Vector2f(sup_x, sup_y); // Vértice superior
    shape[0].color = fig_color;

    double scaled_width = fig_width * currentZoom;
    double w_x = scaled_width * sin;
    double w_y = scaled_width * cos;

    double right_x = w_x + entity_state.position[0];
    double right_y = -w_y + entity_state.position[1];
    shape[1].position = sf::Vector2f(right_x, right_y); // Vértice derecha
    shape[1].color = fig_color;

    double left_x = -w_x + entity_state.position[0];
    double left_y = w_y + entity_state.position[1];
    shape[2].position = sf::Vector2f(left_x, left_y); // Vértice abajo derecha
    shape[2].color = fig_color;
}

void flight_plan::emplace_thrust_segment(double start_time, double end_time, double engine_thrust, basic_state expected_entry_state, basic_state expected_output_state)
{

    size_t last = segments.size();
    segments.emplace_back(this);
    segments[last].segment_type = THRUST;
    segments[last].start_time = start_time;
    segments[last].end_time = end_time;
    segments[last].expected_entry_state = expected_entry_state;
    segments[last].expected_output_state = expected_output_state;
    segments[last].engine_thrust = engine_thrust;
}

void flight_plan::emplace_rotation_segment(double start_time, double end_time, basic_state expected_entry_state, basic_state expected_output_state)
{
    size_t last = segments.size();
    segments.emplace_back(this);
    segments[last].segment_type = ROTATION;
    segments[last].start_time = start_time;
    segments[last].end_time = end_time;
    segments[last].expected_entry_state = expected_entry_state;
    segments[last].expected_output_state = expected_output_state;
    segments[last].position_margin = position_margin;
    segments[last].velocity_margin = velocity_margin;
}