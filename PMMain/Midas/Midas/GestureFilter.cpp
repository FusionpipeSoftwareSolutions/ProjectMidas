#include "GestureFilter.h"
#include <time.h>
#include "MyoCommon.h"

// TODO: Refactor cases to modularize into seperate handler functions!

GestureFilter::GestureFilter(ControlState* controlState, clock_t timeDel) : timeDelta(timeDel), lastPoseType(Pose::rest),
    lastTime(0), controlStateHandle(controlState), gestSeqRecorder(midasMode::LOCK_MODE, 3000)
{
    registerMouseSequences();
    registerKeyboardSequences();
    registerStateSequences();
}

GestureFilter::~GestureFilter()
{
}

void GestureFilter::process()
{
    clock_t timeFromLastPose = 0;
    filterDataMap input = Filter::getInput();
    Pose::Type gesture = boost::any_cast<Pose::Type>(input[GESTURE_INPUT]);
    
    Filter::setFilterError(filterError::NO_FILTER_ERROR);
    Filter::setFilterStatus(filterStatus::OK);

    // Note the gesture sequence recorder should handle ALL gesture events - not just states, as it is doing right now. TODO.
    if (gesture != Pose::Type::rest)
    {
        sequenceResponse response;
        SequenceStatus ss = gestSeqRecorder.progressSequence(gesture, *controlStateHandle, response);
        if (response.responseType == ResponseType::STATE_CHANGE)
        {
            handleStateChange(response);
        }
    }

    // TODO - port this to use the gestSeqRecorder
    if (gesture != lastPoseType)
    {
        // The user's gesture has changed.
        if (gesture == Pose::rest && controlStateHandle->getMode() != midasMode::LOCK_MODE)
        {
            filterDataMap outputToSharedCommandData;
            commandData command;
            command.type = MOUSE_COMMAND;

            if (lastPoseType == MYO_GESTURE_LEFT_MOUSE)
            {
                command.mouse = LEFT_RELEASE;
                outputToSharedCommandData[COMMAND_INPUT] = command;
                Filter::setOutput(outputToSharedCommandData);
                return;
            }
            else if (lastPoseType == MYO_GESTURE_RIGHT_MOUSE)
            {
                command.mouse = RIGHT_RELEASE;
                outputToSharedCommandData[COMMAND_INPUT] = command;
                Filter::setOutput(outputToSharedCommandData);
                return;
            }
        }

        lastPoseType = gesture;
        timeFromLastPose = clock() - lastTime;
        lastTime = clock();
    }

    if (timeFromLastPose >= timeDelta)
    {
        // The user has held the same gesture for a long enough
        // period of time.

        if (controlStateHandle->getMode() != midasMode::LOCK_MODE)
        {
            // No state change. Pass data along pipeline
            filterDataMap outputToSharedCommandData;
            commandData sendData = translateGesture(gesture);

            if (sendData.type == UNKNOWN_COMMAND)
            {
                Filter::setFilterStatus(filterStatus::END_CHAIN);
            }
            else
            {
                outputToSharedCommandData[COMMAND_INPUT] = sendData;
                Filter::setOutput(outputToSharedCommandData);
            }
        }
        else
        {
            Filter::setFilterStatus(filterStatus::END_CHAIN);
        }
    }
}

commandData GestureFilter::translateGesture(Pose::Type pose)
{
    commandData command;
    command.type = MOUSE_COMMAND;

    if (pose == MYO_GESTURE_LEFT_MOUSE)
    {
        command.mouse = LEFT_CLICK;
    }
    else if (pose == MYO_GESTURE_RIGHT_MOUSE)
    {
        command.mouse = RIGHT_CLICK;
    }
    else
    {
        command.type = UNKNOWN_COMMAND;
    }

    return command;
}

void GestureFilter::registerMouseSequences(void)
{

}

void GestureFilter::registerKeyboardSequences(void)
{
    // Nothing to register right now. Always could though :)
}

void GestureFilter::registerStateSequences(void)
{
    // Register sequence from lock to Mouse Mode
    sequence lockToMouseSeq;
    lockToMouseSeq.push_back(Pose::Type::thumbToPinky);
    lockToMouseSeq.push_back(Pose::Type::waveIn);
    lockToMouseSeq.push_back(Pose::Type::waveOut);
    sequenceResponse lockToMouseResponse;
    lockToMouseResponse.responseName = "Unlock";
    lockToMouseResponse.responseType = ResponseType::STATE_CHANGE;
    lockToMouseResponse.responseAction.mode = midasMode::MOUSE_MODE;

    SequenceStatus ss = gestSeqRecorder.registerSequence(midasMode::LOCK_MODE, lockToMouseSeq, lockToMouseResponse);

    // Register sequence from Mouse Mode to Gesture Mode
    sequence mouseToGestSeq;
    mouseToGestSeq.push_back(Pose::Type::thumbToPinky);
    sequenceResponse mouseToGestResponse;
    mouseToGestResponse.responseName = "Mouse To Gesture";
    mouseToGestResponse.responseType = ResponseType::STATE_CHANGE;
    mouseToGestResponse.responseAction.mode = midasMode::GESTURE_MODE;

    ss = gestSeqRecorder.registerSequence(midasMode::MOUSE_MODE, mouseToGestSeq, mouseToGestResponse);

    // Register sequence from Mouse Mode to Keyboard Mode
    sequence mouseToKybrdSeq;
    mouseToKybrdSeq.push_back(Pose::Type::waveOut);
    mouseToKybrdSeq.push_back(Pose::Type::waveIn);
    sequenceResponse mouseToKybrdResponse;
    mouseToKybrdResponse.responseName = "Mouse To Keyboard";
    mouseToKybrdResponse.responseType = ResponseType::STATE_CHANGE;
    mouseToKybrdResponse.responseAction.mode = midasMode::KEYBOARD_MODE;

    ss = gestSeqRecorder.registerSequence(midasMode::MOUSE_MODE, mouseToKybrdSeq, mouseToKybrdResponse);

    // Register sequence from Gesture Mode to Mouse Mode
    sequence gestureToMouseSeq;
    gestureToMouseSeq.push_back(Pose::Type::thumbToPinky);
    sequenceResponse gestureToMouseResponse;
    gestureToMouseResponse.responseName = "Gesture To Mouse";
    gestureToMouseResponse.responseType = ResponseType::STATE_CHANGE;
    gestureToMouseResponse.responseAction.mode = midasMode::MOUSE_MODE;

    ss = gestSeqRecorder.registerSequence(midasMode::GESTURE_MODE, gestureToMouseSeq, gestureToMouseResponse);

    // Register sequence from Keyboard Mode to Mouse Mode
    sequence kybrdToMouseSeq;
    kybrdToMouseSeq.push_back(Pose::Type::waveOut);
    kybrdToMouseSeq.push_back(Pose::Type::waveIn);
    sequenceResponse kybrdToMouseResponse;
    kybrdToMouseResponse.responseName = "Keyboard To Mouse";
    kybrdToMouseResponse.responseType = ResponseType::STATE_CHANGE;
    kybrdToMouseResponse.responseAction.mode = midasMode::MOUSE_MODE;

    ss = gestSeqRecorder.registerSequence(midasMode::KEYBOARD_MODE, kybrdToMouseSeq, kybrdToMouseResponse);

    // Register sequence from ALL Modes to Lock Mode.
    sequence toLockSeq;
    toLockSeq.push_back(Pose::Type::waveIn);
    toLockSeq.push_back(Pose::Type::thumbToPinky);
    sequenceResponse toLockResponse;
    toLockResponse.responseType = ResponseType::STATE_CHANGE;
    toLockResponse.responseAction.mode = midasMode::LOCK_MODE;
    // From Mouse:
    toLockResponse.responseName = "Mouse To Lock";
    ss = gestSeqRecorder.registerSequence(midasMode::MOUSE_MODE, toLockSeq, toLockResponse);
    // From Gesture:
    toLockResponse.responseName = "Gesture To Lock";
    ss = gestSeqRecorder.registerSequence(midasMode::GESTURE_MODE, toLockSeq, toLockResponse);
    // From Keyboard:
    toLockResponse.responseName = "Keyboard To Lock";
    ss = gestSeqRecorder.registerSequence(midasMode::KEYBOARD_MODE, toLockSeq, toLockResponse);
}

void GestureFilter::handleStateChange(sequenceResponse response)
{
    if (response.responseType != ResponseType::STATE_CHANGE)
    {
        // Should NOT have made it here
        return;
    }

    std::cout << "Transitioning to state: " << response.responseAction.mode << std::endl;
    controlStateHandle->setMode(response.responseAction.mode);
    
    return;
}