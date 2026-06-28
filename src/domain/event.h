#pragma once

#include <string>

#include "domain/ids.h"
#include "domain/time.h"

namespace scs::domain {

enum class EventSeverity {
    Info,
    Advisory,
    Threat,
    Critical
};

enum class EventType {
    ScenarioLoaded,
    CommandAccepted,
    CommandRejected,
    VelocityChanged
};

struct Event {
    Tick tick{0};
    EventSeverity severity{EventSeverity::Info};
    EventType type{EventType::ScenarioLoaded};
    EntityId subject;
    std::string message;
};

} // namespace scs::domain
