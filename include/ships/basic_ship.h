#ifndef BASIC_SHIP_H
#define BASIC_SHIP_H

#include <common.h>
#include <basic_entities.h>

// Ship classes identificators
#define CLASS_MASK 0b11110000
#define STATION 0b00010000
#define CARRIER 0b00100000
#define BATTLESHIP 0b00110000
#define CRUISER 0b01000000
#define DESTROYER 0b01010000
#define FRIGATE 0b01100000
#define FIGHTER 0b01110000


class basic_ship : public e_base {
    public:
        float fig_heigh = 20.0;     
        float fig_width = 8.0;  
        sf::Color fig_color = sf::Color(0, 50, 255);  
        std::string name = "UNAMED SHIP"; 

        int ship_class = DESTROYER;

    private:
        sf::VertexArray shape;
        sf::Text positionText;

    public:
        basic_ship() = delete;
        basic_ship(float mass, float size_r, basic_state entity_state):e_base(mass, size_r, entity_state){init_shape();};
        basic_ship(const basic_ship &rh):e_base(rh){init_shape();};
        basic_ship(basic_ship && rh):e_base(std::move(rh)), shape(std::move(rh.shape)){};

        void draw(sf::RenderWindow & window, float currentZoom) override;

    private:
        void init_shape();
        void update_shape(float currentZoom = 1.0);
};

#endif  // BASIC_SHIP_H