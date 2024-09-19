

#include <basic_ship.h>
#include <cmath>  


void basic_ship::draw(sf::RenderWindow & window, float currentZoom){

    update_shape(currentZoom);
    window.draw(shape);

    positionText.setScale(currentZoom, currentZoom);
    sf::FloatRect textBounds = positionText.getLocalBounds();
    positionText.setOrigin(textBounds.width / 2, textBounds.height / 2);
    positionText.setString(name);
    positionText.setPosition(entity_state.position[0], entity_state.position[1] - 25*currentZoom);
    window.draw(positionText);  
}

void basic_ship::init_shape(){
    shape = sf::VertexArray(sf::Triangles, 3);
    update_shape();

    positionText.setFont(e_base::font);
    positionText.setCharacterSize(15);
    sf::FloatRect textBounds = positionText.getLocalBounds();
    positionText.setOrigin(textBounds.width / 2, textBounds.height / 2);
    positionText.setFillColor(sf::Color::Green);
}

void basic_ship::update_shape(float currentZoom){

    float cos = std::cos(entity_state.direction[0]);
    float sin = std::sin(entity_state.direction[0]);

    float scaled_heigh = fig_heigh*currentZoom;
    float sup_x = scaled_heigh*cos + entity_state.position[0];
    float sup_y = scaled_heigh*sin + entity_state.position[1];
    shape[0].position = sf::Vector2f(sup_x, sup_y);  // Vértice superior
    shape[0].color = fig_color;

    float scaled_width= fig_width*currentZoom;
    float w_x = scaled_width*sin;
    float w_y = scaled_width*cos;

    float right_x = w_x + entity_state.position[0];
    float right_y = -w_y + entity_state.position[1];
    shape[1].position = sf::Vector2f(right_x, right_y);  // Vértice derecha
    shape[1].color = fig_color;

    float left_x = -w_x + entity_state.position[0];
    float left_y = w_y + entity_state.position[1];
    shape[2].position = sf::Vector2f(left_x, left_y);  // Vértice abajo derecha
    shape[2].color = fig_color;  
}