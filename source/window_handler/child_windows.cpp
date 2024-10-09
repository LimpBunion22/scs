
#include <child_windows.h>
#include <basic_ship.h>
#include <imgui.h>
#include <imgui-SFML.h>

// ship_window

ship_window::ship_window(std::string in_name, int in_ship_class, basic_ship* in_ship):name(in_name), ship_class(in_ship_class), ship(in_ship){
    
    texture.loadFromFile("../utils/ship1.png");
    log_vector.reserve(128);
    log_vector.emplace_back("CAPTAIN: Ship ONLINE");
    
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
    ImGui::SetNextWindowSize(ImVec2(window_width, window_heigh), ImGuiCond_Always);
    ImGui::Begin(name.c_str(), nullptr, ImGuiWindowFlags_NoResize);

    // Mostrar la imagen en la parte superior
    if (ImGui::BeginChild("Imagen", ImVec2(window_width*2/3, window_heigh/2), true)) {
        ImGui::Image(reinterpret_cast<void*>(texture.getNativeHandle()), ImVec2(window_width*2/3, window_heigh/2));
    }
    ImGui::EndChild();

    // Crear las pestañas (tabs) en la parte inferior
    if (ImGui::BeginTabBar("Pestanas")) {
        if (ImGui::BeginTabItem("Status")) {
            ImGui::BeginTable("Info", 2);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 150.0f); 
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 500.0f); 

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Class:");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("DESTROYER");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Family:");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("VENATOR");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Info:");
            ImGui::TableSetColumnIndex(1);
            ImGui::TextWrapped("El Destructor Estelar clase Venator, también conocido como crucero de ataque de la República o Crucero Jedi, era una nave capital en forma de daga utilizada por la República Galáctica durante las Guerras Clon. Siendo la columna vertebral de la Armada de la República, el Venator era una nave capital versátil capaz de desempeñar los roles de una nave de guerra, capaz de combate nave a nave, así como el papel de transporte con su impresionante complemento de cazas estelares contra las fuerzas de la Confederación de Sistemas Independientes, en algunas de las batallas más conocidas de las Guerras Clon, incluidas las de Sullust, Christophsis y Coruscant.");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("");

            ImGui::EndTable();

            ImGui::BeginTable("Status", 2);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 150.0f); 
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 100.0f); 

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Fuel:");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::to_string(ship->fuel).c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Comms status:");
            ImGui::TableSetColumnIndex(1);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 150, 50, 55));
            ImGui::Text("ONLINE");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Sensors status:");
            ImGui::TableSetColumnIndex(1);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 150, 50, 55));
            ImGui::Text("ONLINE");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Reactor status:");
            ImGui::TableSetColumnIndex(1);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 150, 50, 55));
            ImGui::Text("ONLINE");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Engine status:");
            ImGui::TableSetColumnIndex(1);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 150, 50, 55));
            ImGui::Text("ONLINE");

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Weapons status:");
            ImGui::TableSetColumnIndex(1);
            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(0, 150, 50, 55));
            ImGui::Text("ONLINE");

            ImGui::EndTable();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Comms")) {
            ImGui::Text("Contenido de la pestaña 2");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Trajectory")) {
            ImGui::Text("Contenido de la pestaña 2");

            ImGui::BeginTable("Status", 2);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 200.0f); 
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, 400.0f); 

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Position:");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::string("X"+std::to_string(ship->entity_state.position[0])+" "+"Y"+std::to_string(ship->entity_state.position[1])+" "+"Z"+std::to_string(ship->entity_state.position[2])).c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Velocity:");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::string("Vx"+std::to_string(ship->entity_state.velocity[0])+" "+"Vy"+std::to_string(ship->entity_state.velocity[1])+" "+"Vz"+std::to_string(ship->entity_state.velocity[2])).c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Orientation:");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::string("Ox"+std::to_string(ship->entity_state.direction[0])+" "+"Oy"+std::to_string(ship->entity_state.direction[1])+" "+"Oz"+std::to_string(ship->entity_state.direction[2])).c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Fuel:");
            ImGui::TableSetColumnIndex(1);
            ImGui::Text(std::to_string(ship->fuel).c_str());

            ImGui::EndTable();

            ImVec2 buttonSize = ImVec2(200, 20);
            programed_erase = ImGui::Button("Show trajectory", buttonSize);
            programed_erase = ImGui::Button("Calculate new trajectory", buttonSize);

            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Armament")) {
            ImGui::Text("Contenido de la pestaña 2");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    // Definir el tamaño del botón
    ImVec2 buttonSize = ImVec2(80, 20);
    ImGui::SetCursorPos(ImVec2(window_width - buttonSize.x - 20, 30));
    programed_erase = ImGui::Button("Close", buttonSize);

    // Ventana de logs a la derecha
    // ImGui::SameLine();  // Alinear a la derecha
    ImVec2 logSize = ImVec2(window_width/3 - 40, window_heigh/2 - buttonSize.y - 15);
    ImGui::SetCursorPos(ImVec2(window_width - logSize.x - 20, buttonSize.y + 40));
    ImGui::BeginChild("LogWindow", logSize, true);
    ImGui::Text("Ship log:");
    ImGui::Separator();
    for(auto msg:log_vector){
        ImGui::Text(msg.c_str());
    }
    ImGui::EndChild();

    ImGui::End();
}