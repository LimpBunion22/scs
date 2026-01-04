#ifndef SENSORS_H
#define SENSORS_H

#include <common.h>

// enum class enginePurpose {GENERATOR, THRUSTER, MULTIPURPOSE};

class sensors_base
{

public:
    std::string name;
    std::string type;

    float sensitivity;
    float amplitude;
public:
    sensors_base() = default;
};

#endif