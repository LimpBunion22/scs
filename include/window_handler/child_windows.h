#ifndef CHILD_WINDOWS_H
#define CHILD_WINDOWS_H

#include <common.h>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <basic_ship.h>


class ship_window{
    public:
        std::string name;
        int ship_class;
        bool programed_erase = false;
        basic_ship* ship = nullptr;

        int window_width = 1000;
        int window_heigh = 800;

        std::vector<std::string> log_vector;

    private:
        sf::Texture texture;
        
    public:
        ship_window() = delete;
        ship_window(std::string in_name, int in_ship_class, basic_ship* in_ship);
        ship_window(const ship_window &rh):name(rh.name),ship_class(rh.ship_class),texture(rh.texture), ship(rh.ship){
            log_vector.reserve(128);
            log_vector.emplace_back("CAPTAIN: Ship ONLINE");};
        // ship_window(ship_window && rh):name(rh.name),ship_class(rh.ship_class), texture(std::move(rh.texture)){};

        void draw();

};


#endif