#ifndef CHILD_WINDOWS_H
#define CHILD_WINDOWS_H

#include <common.h>
#include <sstream>
#include <SFML/Graphics.hpp>


class ship_window{
    public:
        std::string name;
        int ship_class;
        bool programed_erase = false;
    private:
        sf::Texture texture;
        
    public:
        ship_window() = delete;
        ship_window(std::string in_name, int in_ship_class);
        ship_window(const ship_window &rh):name(rh.name),ship_class(rh.ship_class),texture(rh.texture){};
        // ship_window(ship_window && rh):name(rh.name),ship_class(rh.ship_class), texture(std::move(rh.texture)){};

        void draw();

};

#endif