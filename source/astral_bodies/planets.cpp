

#include <planets.h>


void planet::draw(sf::RenderWindow & window, float currentZoom){
    shape.setPosition(entity_state.position[0], entity_state.position[1]);
    window.draw(shape);
    float new_size = rec_size * currentZoom;
    if(new_size > 1.5*size_r){
        square.setSize(sf::Vector2f(new_size, new_size));
        square.setOrigin(square.getSize().x / 2, square.getSize().y / 2);
        square.setOutlineThickness(2*currentZoom);  // Grosor del borde
        square.setPosition(entity_state.position[0], entity_state.position[1]);  // Posición del cuadrado
        window.draw(square);  

        // positionText.setCharacterSize(20*currentZoom);
        positionText.setScale(currentZoom, currentZoom);
        sf::FloatRect textBounds = positionText.getLocalBounds();
        positionText.setOrigin(textBounds.width / 2, textBounds.height / 2);
        positionText.setString(name);
        positionText.setPosition(entity_state.position[0], entity_state.position[1] - 50*currentZoom);
        window.draw(positionText);  
    }
}

void planet::init_shape(){
    square = sf::RectangleShape(sf::Vector2f(rec_size, rec_size));
    square.setOrigin(square.getSize().x / 2, square.getSize().y / 2);
    square.setFillColor(sf::Color::Transparent);  // Sin relleno
    square.setOutlineThickness(2);  // Grosor del borde
    square.setOutlineColor(sf::Color::Green);  // Color del borde

    positionText.setFont(font);
    positionText.setCharacterSize(15);
    sf::FloatRect textBounds = positionText.getLocalBounds();
    positionText.setOrigin(textBounds.width / 2, textBounds.height / 2);
    positionText.setFillColor(sf::Color::Green);

    shape.setOrigin(shape.getRadius(), shape.getRadius());
    shape.setFillColor(sf::Color::White);
    shape.setPosition(entity_state.position[0], entity_state.position[1]);
}

void planet::star(){
    rec_size = 50;
    square = sf::RectangleShape(sf::Vector2f(rec_size, rec_size));
    square.setOrigin(square.getSize().x / 2, square.getSize().y / 2);
    square.setFillColor(sf::Color::Transparent);  // Sin relleno
    square.setOutlineThickness(2);  // Grosor del borde
    square.setOutlineColor(sf::Color(200,200,0));  // Color del borde

    positionText.setFont(font);
    positionText.setCharacterSize(20);
    sf::FloatRect textBounds = positionText.getLocalBounds();
    positionText.setOrigin(textBounds.width / 2, textBounds.height / 2);
    positionText.setFillColor(sf::Color(200,200,0));
}