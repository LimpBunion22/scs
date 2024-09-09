#ifndef COMMON_H
#define COMMON_H

#include <vector>

#define in_bounds(value, lower, upper) ((value) >= (lower) && (value) <= (upper))


class basic_state{

    public:
        std::vector<float> position;
        std::vector<float> velocity;
        std::vector<float> direction;

    public:
        basic_state(): position(3), velocity(3), direction(3){};
        basic_state(std::vector<float>& v1, std::vector<float>& v2, std::vector<float>& v3)
            : position(v1), velocity(v2), direction(v3) {};
};

#endif