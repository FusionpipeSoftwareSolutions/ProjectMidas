#ifndef _MYO_COMMON_H
#define _MYO_COMMON_H

#include "myo\myo.hpp"
#include <string>
#include <iostream>

#ifdef USE_SIMULATOR
#include "MyoSimIncludes.hpp"
#endif

#ifdef USE_SIMULATOR
using namespace myoSim;
#else
using namespace myo;
#endif


/**
* Simple string conversion for Pose::Types to string.
*
* @param gesture The gesture to get toString() of.
* @return std::string The string equivelant of the input gesture.
*/
std::string PoseTypeToString(Pose::Type gesture);

struct SeqElement {
    // Enumeration to distinguish between:
    // TAP: 'short hold' ie, this is a pose that occurs for less time than REQ_HOLD_TIME,
    //      and using this type allows holds to be registered that would logically conflict otherwise
    // HOLD: indicates a pose needs to be held for REQ_HOLD_TIME to progress the sequence
    // IMMEDIATE: this will progress a sequence immediately, but will mean that no holds of the same
    //              pose-prefix can be registered without conflict. This should be okay, as it's main 
    //              design intention is to allow for click/drag functionality.
    enum class PoseLength {
        TAP,
        HOLD,
        IMMEDIATE
    };

    SeqElement(Pose::Type type) {
        this->type = type;
        poseLen = PoseLength::TAP;
    }
    SeqElement(Pose::Type type, PoseLength PL) {
        this->type = type;
        this->poseLen = PL;
    }

    Pose::Type type;

    PoseLength poseLen = PoseLength::TAP; 

    bool operator==(SeqElement& e)
    {
        if (e.type == type && e.poseLen == poseLen)
        {
            return true;
        }
        return false;
    }
    
    bool operator!=(SeqElement& e)
    {
        return !(e == *this);
    }
};


#endif /* _MYO_COMMON_H */