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


#endif /* _MYO_COMMON_H */