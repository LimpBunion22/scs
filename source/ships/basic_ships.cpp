

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

void basic_ship::update_shape(float currentZoom)
{

    float cos = std::cos(entity_state.direction[0]);
    float sin = std::sin(entity_state.direction[0]);

    float scaled_heigh = fig_heigh * currentZoom;
    float sup_x = scaled_heigh * cos + entity_state.position[0];
    float sup_y = scaled_heigh * sin + entity_state.position[1];
    shape[0].position = sf::Vector2f(sup_x, sup_y); // Vértice superior
    shape[0].color = fig_color;

    float scaled_width = fig_width * currentZoom;
    float w_x = scaled_width * sin;
    float w_y = scaled_width * cos;

    float right_x = w_x + entity_state.position[0];
    float right_y = -w_y + entity_state.position[1];
    shape[1].position = sf::Vector2f(right_x, right_y); // Vértice derecha
    shape[1].color = fig_color;

    float left_x = -w_x + entity_state.position[0];
    float left_y = w_y + entity_state.position[1];
    shape[2].position = sf::Vector2f(left_x, left_y); // Vértice abajo derecha
    shape[2].color = fig_color;
}

flight_plan::flight_plan(basic_ship *owner, std::string name)
{
    _owner = owner;
    designation = name;
}

void flight_plan::emplace_thrust_segment(float end_time, float engine_thrust)
{

    size_t last = segments.size();
    if (last < 1)
        return;
    float start_time = segments[last - 1].end_time;
    emplace_thrust_segment(start_time, end_time, engine_thrust);
}

void flight_plan::emplace_thrust_segment(float start_time, float end_time, float engine_thrust)
{

    size_t last = segments.size();
    segments.emplace_back(this);
    segments[last].start_time = start_time;
    segments[last].end_time = end_time;
    segments[last].engine_thrust = engine_thrust;
}

void flight_plan::emplace_rotation_segment(float end_time, f_vector new_orientation)
{

    size_t last = segments.size();
    if (segments.size() < 1)
        return;
    float start_time = segments[last - 1].end_time;
    emplace_rotation_segment(start_time, end_time, new_orientation);
}

void flight_plan::emplace_rotation_segment(float start_time, float end_time, f_vector new_orientation)
{
    // No me gusta esta aproximacion, es complicada y no tiene autocorreccion. PIDs separados?
    size_t last = segments.size();
    float maneuver_time = end_time - start_time;
    for (size_t i = 0; i < 3; i++)
    {
        float delta_dir = new_orientation[i] - _owner->entity_state.direction[i];
        bool sign = delta_dir >= 0 ? true : false;
        delta_dir = std::abs(delta_dir);
        delta_dir = delta_dir > 3.14159264 ? delta_dir - 3.14159264 : delta_dir;

        float aux = std::sqrt(maneuver_time * maneuver_time - 4 / _owner->max_rotation_force[i] * delta_dir);
        float speed1 = (maneuver_time + aux) * _owner->max_rotation_force[i] / 2;
        float speed2 = (maneuver_time - aux) * _owner->max_rotation_force[i] / 2;
        float selected_speed = _owner->rotation_consumption_mode == MAX_POWER ? speed1 : speed2;
    }

    segments.emplace_back(this);
    segments[last].start_time = start_time;
    segments[last].end_time = end_time;

    segments[last].engine_thrust = engine_thrust;
}