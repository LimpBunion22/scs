#ifndef CARGOBAYS_H
#define CARGOBAYS_H

#include <common.h>

enum class cargoPurpose {ARMAMENT, PLATFORM_LAUNCHER, DOCK, OTHER};

class cargoBay_base
{

public:
    std::string name;
    std::string type;
    cargoPurpose purpose;

    int spaces;
public:
    cargoBay_base() = default;
};

#endif