

#include <basic_ship.h>
#include <cmath>

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