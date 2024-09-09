
#include <log_window.h>

log_window_handler::log_window_handler():window_log(sf::VideoMode(C_WINDOW_WIDTH, C_WINDOW_HEIGH), "MISSION LOG"){

    window_log.setFramerateLimit(60);
    ImGui::SFML::Init(window_log);

}

bool show_demo_window = true; 
float slider_value = 0.0f;
sf::Color clear_color = sf::Color::Black;
bool log_window_handler::manage_events(sf::Time elapTime){

    sf::Event event;
    while (window_log.pollEvent(event)) {
        ImGui::SFML::ProcessEvent(event);
        if (event.type == sf::Event::Closed)
            return true;    
    }

    ImGui::SFML::Update(window_log,elapTime);

    // Empezar una nueva ventana de ImGui
    ImGui::Begin("Mi primera ventana con ImGui");

    // Crear un slider que controle un valor flotante
    ImGui::SliderFloat("Deslizador", &slider_value, 0.0f, 100.0f);

    // Crear un botón que cambie el color de fondo
    if (ImGui::Button("Cambiar color de fondo")) {
        clear_color = sf::Color::Green;  // Cambiar el color de fondo
    }

    // Crear una casilla para mostrar u ocultar la ventana de demostración de ImGui
    ImGui::Checkbox("Mostrar ventana de demostración", &show_demo_window);

    ImGui::End();

    // Mostrar la ventana de demostración de ImGui (si está activada)
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // Limpiar la ventana con el color de fondo seleccionado
    window_log.clear(clear_color);

    // Dibujar la interfaz de ImGui en la ventana SFML
    ImGui::SFML::Render(window_log);

    // Mostrar los gráficos de SFML
    window_log.display();

    return false;
}

log_window_handler::~log_window_handler(){    
    window_log.close();
    ImGui::SFML::Shutdown();
}