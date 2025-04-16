
#pragma once
#include <audiopolicy.h>
#include <AudioClient.h>
#include <AudioProcessingObject.h>

class MicrophoneAPO : public IAudioProcessingObject {
public:
    MicrophoneAPO();
    virtual ~MicrophoneAPO();
};
