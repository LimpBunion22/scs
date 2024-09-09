#include <tactical_window.h>

tactical_window_handler::tactical_window_handler(const sf::Font in_font):window_tactical(sf::VideoMode(C_WINDOW_WIDTH, C_WINDOW_HEIGH), "TACTICAL MAP"),
        map_view(sf::FloatRect(0.f, 0.f, C_WINDOW_WIDTH, C_WINDOW_HEIGH)),
        hud_view(sf::FloatRect(0.f, 0.f, C_WINDOW_WIDTH, C_WINDOW_HEIGH)),
        referenceLine(sf::Vector2f(referenceLineLength, 5.f))
{
    font = in_font;

    positionText.setFont(font);
    positionText.setCharacterSize(20);
    positionText.setFillColor(sf::Color::Green);

    scaleText.setFont(font);
    scaleText.setCharacterSize(20);
    scaleText.setFillColor(sf::Color::Green);

    referenceLine.setFillColor(sf::Color::Green);
    referenceLine.setSize(sf::Vector2f(100.0f, 5.f));
    
    planets_ptr.reserve(32);
}

void tactical_window_handler::emplace_planet(planet * new_planet_ptr){
    if(planets_ptr.size()+1>planets_ptr.capacity())
        planets_ptr.reserve(planets_ptr.capacity()+32);
    planets_ptr.emplace_back(new_planet_ptr);
}

bool tactical_window_handler::manage_events(float deltaTime){
    sf::Event event;
    while (window_tactical.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            return true;        

        // Detectar si se ha redimensionado la ventana
        if (event.type == sf::Event::Resized) {
            window_heigh = event.size.height;
            window_width = event.size.width;
            aspectRatio = static_cast<float>(event.size.width) / static_cast<float>(event.size.height);
            map_view.setSize(C_WINDOW_HEIGH * aspectRatio, C_WINDOW_HEIGH);
            hud_view.setSize(C_WINDOW_HEIGH * aspectRatio, C_WINDOW_HEIGH);
        }

        // Detectar scroll del ratón para hacer zoom
        if (event.type == sf::Event::MouseWheelScrolled) {
            if (event.mouseWheelScroll.delta > 0) {
                // Acercar (zoom in)
                float nextZoom = currentZoom*C_ZOOM_INCREMENT;
                if(in_bounds(nextZoom,1.0,12e6)){
                    currentZoom = nextZoom;
                    map_view.zoom(C_ZOOM_INCREMENT);  // Reduce la vista en un 10%
                }
            } else if (event.mouseWheelScroll.delta < 0) {
                float nextZoom = currentZoom / C_ZOOM_INCREMENT;
                if(in_bounds(nextZoom,1.0,12e6)){
                    currentZoom = nextZoom;
                    map_view.zoom(1.0f / C_ZOOM_INCREMENT);  // Reduce la vista en un 10%
                }
            }
        }
    }

    // Desplazamiento de la vista con teclas WASD
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        map_view.move(0.f, -C_VIEW_SPEED * currentZoom * deltaTime); // Arriba
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        map_view.move(0.f, C_VIEW_SPEED * currentZoom * deltaTime); // Abajo
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        map_view.move(-C_VIEW_SPEED * currentZoom * deltaTime, 0.f); // Izquierda
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        map_view.move(C_VIEW_SPEED * currentZoom * deltaTime, 0.f); // Derecha
    }

    return false;
}

void tactical_window_handler::draw_map(){
    window_tactical.clear();
    window_tactical.setView(map_view);
    for (auto ptr : planets_ptr) {
        ptr->draw(window_tactical, currentZoom);
    }
}

void tactical_window_handler::draw_hud(){
    sf::Vector2i mousePos = sf::Mouse::getPosition(window_tactical);
    sf::Vector2f mouseVis= window_tactical.mapPixelToCoords(mousePos, map_view);
    // window_tactical.setView(window_tactical.getDefaultView());
    window_tactical.setView(hud_view);

    // float scaledLineLength = referenceLineLength / (aspectRatio/C_ORIGINAL_ASPECT_RATIO);
    // referenceLine.setSize(sf::Vector2f(scaledLineLength, 5.f)); 
    sf::Vector2i pixelPos(20, 40);
    sf::Vector2f worldPos = window_tactical.mapPixelToCoords(pixelPos); 
    referenceLine.setPosition( worldPos);
    window_tactical.draw(referenceLine);  

    std::stringstream ss;
    ss << static_cast<int>(100 * currentZoom) << " Km";  // Ajustar el texto según el zoom
    scaleText.setString(ss.str());
    pixelPos = sf::Vector2i(20, 70);
    worldPos = window_tactical.mapPixelToCoords(pixelPos);
    scaleText.setPosition( worldPos);
    window_tactical.draw(scaleText);
    
    ss.str("");  // Vacía el buffer
    ss.clear();  // Restablece los flags de error
    ss << "Coordinates: " << mouseVis.x << ", " << mouseVis.y;
    positionText.setString(ss.str());
    sf::FloatRect textBounds = positionText.getLocalBounds();
    pixelPos = sf::Vector2i(static_cast<int>(window_width - textBounds.width - 10), static_cast<int>(10));
    worldPos = window_tactical.mapPixelToCoords(pixelPos);
    positionText.setPosition(worldPos);
    // positionText.setPosition(
    //     (window_width - textBounds.width - 10)/ (aspectRatio/C_ORIGINAL_ASPECT_RATIO),  // Colocar en la esquina superior derecha con un margen de 10 píxeles
    //     10  // Un margen de 10 píxeles desde la parte superior
    // );
    window_tactical.draw(positionText);
    window_tactical.display();
}

tactical_window_handler::~tactical_window_handler(){
    window_tactical.close();
}