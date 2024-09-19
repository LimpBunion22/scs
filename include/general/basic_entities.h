#ifndef BASIC_ENTITIES_H
#define BASIC_ENTITIES_H

#include <SFML/Graphics.hpp>
#include <common.h>

class e_base{
    
    public:
        basic_state entity_state;

        float mass;
        float size_r;
        float selectable_size = 25;
    protected:
        static sf::Font font;
    
    public:
        e_base(): mass(0), size_r(0){};
        e_base(float mass, float size_r, basic_state entity_state): mass(mass), size_r(size_r), entity_state(entity_state){};
        e_base(const e_base &rh): mass(rh.mass), size_r(rh.size_r), entity_state(rh.entity_state){};
        e_base(e_base &&rh): mass(rh.mass), size_r(rh.size_r), entity_state(std::move(rh.entity_state)){};

        virtual void draw(sf::RenderWindow & window, float currentZoom) = 0;
        static void init_font(sf::Font in_font){e_base::font = in_font;};
        // ~e_base();

};

#endif