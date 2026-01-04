#ifndef ENGINES_H
#define ENGINES_H

#include <common.h>

enum class enginePurpose {GENERATOR, THRUSTER, MULTIPURPOSE};
inline const char* to_string(enginePurpose t) {
    switch (t) {
        case enginePurpose::GENERATOR:          return "GENERATOR";
        case enginePurpose::THRUSTER:           return "THRUSTER";
        case enginePurpose::MULTIPURPOSE:       return "MULTIPURPOSE";
        default:                                return "MULTIPURPOSE";
    }
}
inline const void from_string(std::string t, enginePurpose &r) {    
    if(t.find("GENERATOR")!= std::string::npos)         {r = enginePurpose::GENERATOR; return;}
    if(t.find("THRUSTER")!= std::string::npos)          {r = enginePurpose::THRUSTER; return;}
    if(t.find("MULTIPURPOSE")!= std::string::npos)      {r = enginePurpose::MULTIPURPOSE; return;}
    r = enginePurpose::MULTIPURPOSE;
}

class engines_base
{

public:
    std::string name;
    std::string type;
    std::string purpose;
    enginePurpose purposeI;

    float fuelCapacity;
    float thrustEfficiency;
    float maxThrust;
    float maxRotationalThrust;
    float generatorEfficiency;
    float maxGeneration;

public:
    engines_base() = default;
};

#endif