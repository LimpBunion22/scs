#ifndef PLANET_H
#define PLANET_H

#include <basic_entities.h>

class planet : public e_base
{
    public:
        std::string name = "UNAMED PLANET";

    private:
        
        sf::CircleShape shape;
        sf::RectangleShape square; 
        // sf::Font font;
        sf::Text positionText;

        float rec_size = 25;

    public:
        // planet() = delete;
        planet(float mass, float size_r, basic_state entity_state): e_base(mass, size_r, entity_state), shape(size_r){init_shape();};
        planet(const planet &rh):e_base(rh), shape(rh.size_r), square(rh.square), positionText(rh.positionText), rec_size(rh.rec_size), name(rh.name){init_shape();};
        planet(planet &&rh):e_base(std::move(rh)), shape(std::move(rh.shape)), positionText(std::move(rh.positionText)), rec_size(rh.rec_size), name(rh.name){};

        void star();

        void draw(sf::RenderWindow & window, float currentZoom) override;

    private:

        void init_shape();

        // ~planet();
};

#endif