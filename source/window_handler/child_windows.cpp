
#include <child_windows.h>
#include <basic_ship.h>
#include <imgui.h>
#include <imgui-SFML.h>

// ship_window

ship_window::ship_window(std::string in_name, int in_ship_class):name(in_name), ship_class(in_ship_class){
    
    texture.loadFromFile("../utils/ship1.png");
    
    switch (ship_class & CLASS_MASK)
    {
    case CARRIER:
            texture.loadFromFile("../utils/ship1.png");
        break;
    
    default:
            texture.loadFromFile("../utils/ship1.png");
        break;
    }
}

void ship_window::draw(){
    ImGui::SetNextWindowSize(ImVec2(800, 600), ImGuiCond_Always);
    ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoResize);

    // Mostrar la imagen en la parte superior
    if (ImGui::BeginChild("Imagen", ImVec2(0, 150), true)) {
        // Renderizar la imagen. Suponiendo que la textura de SFML está vinculada con OpenGL
        ImGui::Image(reinterpret_cast<void*>(texture.getNativeHandle()), ImVec2(400, 300));
    }
    ImGui::EndChild();

    // Crear las pestañas (tabs) en la parte inferior
    if (ImGui::BeginTabBar("Pestanas")) {
        if (ImGui::BeginTabItem("Status")) {
            ImGui::Text("Contenido de la pestaña 1");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Comms")) {
            ImGui::Text("Contenido de la pestaña 2");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Trajectory")) {
            ImGui::Text("Contenido de la pestaña 2");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Armament")) {
            ImGui::Text("Contenido de la pestaña 2");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // Ventana de logs a la derecha
    // ImGui::SameLine();  // Alinear a la derecha
    ImVec2 logSize = ImVec2(300, 0);
    ImGui::SetCursorPos(ImVec2(800 - logSize.x - 20, 80));
    ImGui::BeginChild("LogWindow", ImVec2(300, 0), true);
    ImGui::Text("Ship log:");
    ImGui::Separator();
    ImGui::Text("Log 1: Esto es un ejemplo.");
    ImGui::Text("Log 2: Más mensajes de logs.");
    ImGui::EndChild();

    // Definir el tamaño del botón
    ImVec2 buttonSize = ImVec2(80, 20);

    // Colocar el cursor en la posición donde queremos el botón (esquina superior derecha)
    ImGui::SetCursorPos(ImVec2(800 - buttonSize.x - 20, 30));  // 10 es el padding desde la esquina

    // Dibujar el botón
    programed_erase = ImGui::Button("Close", buttonSize);

    ImGui::End();
}