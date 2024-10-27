#ifndef BASIC_ENTITIES_H
#define BASIC_ENTITIES_H

#include <SFML/Graphics.hpp>
#include <common.h>

class e_base
{

public:
    basic_state entity_state;

    float mass;
    float size_r;
    float selectable_size = 25;

    f_vector main_dimensions;
    f_vector inertia_tensor;

protected:
    static sf::Font font;

public:
    e_base() : mass(0), size_r(0), main_dimensions(3), inertia_tensor(9) {};
    e_base(float mass, float size_r, basic_state entity_state) : mass(mass), size_r(size_r), entity_state(entity_state) {};
    e_base(float mass, float size_r, basic_state entity_state, f_vector main_dimensions, f_vector inertia_tensor) : mass(mass), size_r(size_r), entity_state(entity_state), main_dimensions(main_dimensions), inertia_tensor(inertia_tensor) {};

    e_base(const e_base &rh) : mass(rh.mass), size_r(rh.size_r), entity_state(rh.entity_state), main_dimensions(rh.main_dimensions), inertia_tensor(rh.inertia_tensor) {};
    e_base(e_base &&rh) : mass(rh.mass), size_r(rh.size_r), entity_state(std::move(rh.entity_state)), main_dimensions(std::move(rh.main_dimensions)), inertia_tensor(std::move(rh.inertia_tensor)) {};

    virtual void draw(sf::RenderWindow &window, float currentZoom) = 0;
    static void init_font(sf::Font in_font) { e_base::font = in_font; };
    // ~e_base();
};

#endif