#ifndef CHILD_WINDOWS_H
#define CHILD_WINDOWS_H

#include <common.h>
#include <sstream>
#include <SFML/Graphics.hpp>
#include <basic_ship.h>


class ship_window{
    public:
        std::string name;
        bool programed_erase = false;
        int shipInd;

        int window_width = 1000;
        int window_heigh = 800;

        std::vector<std::string> log_vector;

    private:
        std::vector<basic_ship> *shipV_ptr;
        sf::Texture texture;
        
    public:
        ship_window() = delete;
        ship_window(std::vector<basic_ship>  *in_shipV_ptr, std::string in_name, int shipCnt);
        ship_window(const ship_window &rh):shipV_ptr(rh.shipV_ptr),name(rh.name),texture(rh.texture), shipInd(rh.shipInd){
            log_vector.reserve(128);
            log_vector.emplace_back("CAPTAIN: Ship ONLINE");};
        // ship_window(ship_window && rh):name(rh.name),ship_class(rh.ship_class), texture(std::move(rh.texture)){};

        void draw();

};


#endif