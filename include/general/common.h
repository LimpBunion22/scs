#ifndef COMMON_H
#define COMMON_H

#include <vector>
#include <string>
#include <sstream>
#include <functional>

#define in_bounds(value, lower, upper) ((value) >= (lower) && (value) <= (upper))
#define RED 1
#define YELLOW 2
#define GREEN 3

#define ON_LEFT_CLICK_SHIP 0b00010001

const float G = 6.6743e-11;
using f_vector = std::vector<float>;
using lamda_func = std::function<f_vector(float)>;

class basic_state{

    public:
        f_vector position = {0.0, 0.0, 0.0};
        f_vector velocity = {0.0, 0.0, 0.0};

        f_vector direction = {0.0, 0.0, 0.0};
        f_vector ang_velocity = {0.0, 0.0, 0.0};

    public:
        basic_state(){};
        basic_state(f_vector& v1)
            : position(v1){};
        basic_state(f_vector& v1, f_vector& v2)
            : position(v1), velocity(v2){};
        basic_state(f_vector& v1, f_vector& v2, f_vector& v3)
            : position(v1), velocity(v2), direction(v3) {};
        basic_state(f_vector& v1, f_vector& v2, f_vector& v3, f_vector& v4)
            : position(v1), velocity(v2), direction(v3), ang_velocity(v4) {};
};





#endif