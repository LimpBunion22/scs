#include <iostream>
#include <ctime>
#include <sstream>
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
    ImGui::Begin("MISSION LOG");

    for (const auto& log : logs) {
        ImVec4 color = getSeverityColor(log.second); // Obtener color según severidad
        ImGui::TextColored(color, "%s", log.first.c_str());
    }

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

void log_window_handler::logMessage(const std::string& message, int severity){
        std::time_t now = std::time(nullptr);
        std::tm* localTime = std::localtime(&now);

        // Crear un stream para formatear la fecha y hora
        std::stringstream ss;
        ss << "[" 
        << (localTime->tm_mday < 10 ? "0" : "") << localTime->tm_mday << ":"  // Día
        << (localTime->tm_mon + 1 < 10 ? "0" : "") << (localTime->tm_mon + 1) << ":"  // Mes
        << (localTime->tm_year + 1900) << "-"  // Año
        << (localTime->tm_hour < 10 ? "0" : "") << localTime->tm_hour << ":"  // Hora
        << (localTime->tm_min < 10 ? "0" : "") << localTime->tm_min << ":"   // Minutos
        << (localTime->tm_sec < 10 ? "0" : "") << localTime->tm_sec  // Segundos
        << "] ";

        logs.emplace_back(ss.str() + message, severity);
}

ImVec4 log_window_handler::getSeverityColor(int severity) {
    // Definir colores según la severidad del mensaje
    switch (severity) {
        case RED: return ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Rojo para errores graves
        case YELLOW: return ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Amarillo para advertencias
        case GREEN: return ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Verde para información
        default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Blanco para mensajes normales
    }
}

log_window_handler::~log_window_handler(){    
    window_log.close();
    ImGui::SFML::Shutdown();
}