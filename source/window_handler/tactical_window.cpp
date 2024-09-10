#include <tactical_window.h>
#include <cmath>  // Para std::sqrt


tactical_window_handler::tactical_window_handler(const sf::Font in_font, log_window_handler* in_logger):
        window_tactical(sf::VideoMode(C_WINDOW_WIDTH, C_WINDOW_HEIGH), "TACTICAL MAP"),
        map_view(sf::FloatRect(0.f, 0.f, C_WINDOW_WIDTH, C_WINDOW_HEIGH)),
        hud_view(sf::FloatRect(0.f, 0.f, C_WINDOW_WIDTH, C_WINDOW_HEIGH)),
        referenceLine(sf::Vector2f(referenceLineLength, 5.f)),
        logger(in_logger)
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

    gravity_map.create(C_WINDOW_WIDTH,C_WINDOW_HEIGH);
}

void tactical_window_handler::emplace_planet(planet * new_planet_ptr){
    if(planets_ptr.size()+1>planets_ptr.capacity())
        planets_ptr.reserve(planets_ptr.capacity()+32);
    planets_ptr.emplace_back(new_planet_ptr);
}

bool tactical_window_handler::manage_events(float deltaTime){
    sf::Event event;
    bool on_change = false;
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
            on_change = true;
        }

        // Detectar scroll del ratón para hacer zoom
        if (event.type == sf::Event::MouseWheelScrolled) {
            on_change = true;
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
        on_change = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        map_view.move(0.f, C_VIEW_SPEED * currentZoom * deltaTime); // Abajo
        on_change = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        map_view.move(-C_VIEW_SPEED * currentZoom * deltaTime, 0.f); // Izquierda
        on_change = true;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        map_view.move(C_VIEW_SPEED * currentZoom * deltaTime, 0.f); // Derecha
        on_change = true;
    }

    if(on_change == true && draw_gravity == true){
        window_tactical.setView(map_view);

        // Obtener el área visible en el mundo
        sf::Vector2f viewSize = map_view.getSize();
        sf::Vector2f viewCenter = map_view.getCenter();
        float left = viewCenter.x - viewSize.x / 2.0f;
        float top = viewCenter.y - viewSize.y / 2.0f;
        sf::FloatRect visibleArea(left, top, viewSize.x, viewSize.y);  // Área visible en el mundo

        // Recalcular el mapa de gravedad
        sf::Vector2u windowSize = window_tactical.getSize();
        gravity_map.create(windowSize.x, windowSize.y);
        generateHeatmap(gravity_map, visibleArea);  // Pasa el área visible en el mundo

        if (!heatmapTexture.loadFromImage(gravity_map)) {
            logger->logMessage("Error: No se pudo cargar la textura del mapa de calor", RED);
        }
        heatmapSprite.setTexture(heatmapTexture, true);
        float scaleX = viewSize.x / static_cast<float>(windowSize.x);
        float scaleY = viewSize.y / static_cast<float>(windowSize.y);
        heatmapSprite.setScale(scaleX, scaleY);
        heatmapSprite.setPosition(left, top);  // Posicionar el sprite correctamente
        // heatmapSprite.setPosition(0, 0);  // Intentar dibujar en la esquina superior izquierda
    }

    return false;
}

void tactical_window_handler::draw_map(){
    window_tactical.clear();
    window_tactical.setView(map_view);
    if(draw_gravity == true){
        window_tactical.draw(heatmapSprite);
    }
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

float tactical_window_handler::_gravityFunction(float x, float y){
    float g_x = 0;
    float g_y = 0;
    for (auto ptr : planets_ptr) {
        float r_x = 1000*(x - ptr->entity_state.position[0]);
        float r_y = 1000*(y - ptr->entity_state.position[1]);
        float new_dir = std::atan2(-r_y,-r_x);
        float r = r_x*r_x + r_y*r_y;
        if(r==0){
            continue;
        }
        float new_g = 6.6743e-11*ptr->mass/r;
        float g_x = new_g*std::cos(new_dir) + g_x;
        float g_y = new_g*std::sin(new_dir) + g_y;
    }
    // g = g > 1e14 ? 1e14 : g;
    return std::sqrt(g_x*g_x + g_y*g_y);
}

const float P1 = 0.000000001;
const float P2 = 0.0004;
const float P3 = 0.1;
const float P4 = 300.0;

void tactical_window_handler::generateHeatmap(sf::Image& heatmap, const sf::FloatRect& visibleArea) {
    unsigned int width = heatmap.getSize().x;
    unsigned int height = heatmap.getSize().y;
    float step_w = visibleArea.width/width;
    float step_h = visibleArea.height/height;
    for (unsigned int i = 0; i < width; ++i) {
        for (unsigned int j = 0; j < height; ++j) {
            // Convertir el píxel en coordenadas del mundo
            float x = visibleArea.left + i * step_w;
            float y = visibleArea.top + j * step_h;
            float value = _gravityFunction(x, y);
            // float value = std::log(_gravityFunction(x, y)/1e13 + 1);
            // g = g > 1e14 ? 1e14 : g;
            // value = 1/(1+std::exp(-value/1e5+0.5));

            sf::Uint8 colorValue = static_cast<sf::Uint8>(value*255/P1); // De [-1,1] a [0,255]
            heatmap.setPixel(i, j, sf::Color(0, 0, colorValue,50)); // Colores de azul a rojo

            // if(value < P1){
            //     sf::Uint8 colorValue = static_cast<sf::Uint8>(value*255/P1); // De [-1,1] a [0,255]
            //     heatmap.setPixel(i, j, sf::Color(0, 0, colorValue,50)); // Colores de azul a rojo
            //     continue;
            // }
            // if(value < P2){
            //     // value = std::log(value - P1 + 1)/std::log(P2 + 1);
            //     sf::Uint8 colorValue = static_cast<sf::Uint8>((value-P1)*255/(P2-P1)); // De [-1,1] a [0,255]
            //     heatmap.setPixel(i, j, sf::Color(0, colorValue, 255 - colorValue,20)); // Colores de azul a rojo
            //     continue;
            // }
            // if(value < P3){
            //     // value = std::log(value - P1 + 1)/std::log(P2 + 1);
            //     sf::Uint8 colorValue = static_cast<sf::Uint8>((value-P2)*255/(P3-P2)); // De [-1,1] a [0,255]
            //     heatmap.setPixel(i, j, sf::Color(colorValue, 255 - colorValue, 0,20)); // Colores de azul a rojo
            //     continue;
            // }
            // // value = std::log(value - P2 + 1)/std::log(P3 + 1);
            //     sf::Uint8 colorValue = static_cast<sf::Uint8>((value-P3)*255/(P4-P3)); // De [-1,1] a [0,255]
            // heatmap.setPixel(i, j, sf::Color(255 - colorValue, 0, 0,20)); // Colores de azul a rojo
        }
    }
}