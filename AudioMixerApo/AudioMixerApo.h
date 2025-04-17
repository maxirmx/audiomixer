#pragma once  
#include <audioenginebaseapo.h>
#include <audioengineextensionapo.h>
#include <BaseAudioProcessingObject.h>

#include "AudioMixerGuids.h"  

// Define the registration properties for AudioMixerApo
const APO_REG_PROPERTIES AudioMixerApoRegProperties = {
    CLSID_AudioMixerApo,          // CLSID
    APO_FLAG_NONE,                    // Flags
    L"Audio Mixer APO",               // Friendly name
    L"Copyright (C) 2025",            // Copyright info
    1,                                // Major version
    0,                                // Minor version
    1,                                // Min input connections
    1,                                // Max input connections
    1,                                // Min output connections
    1,                                // Max output connections
    1,                                // Max instances
    1,                                // Number of APO interfaces
    { __uuidof(IAudioProcessingObject) } // List of supported interfaces
};

class AudioMixerApo : public CBaseAudioProcessingObject {
private:
    bool m_isProcessing; // Track whether processing is active

public:
    AudioMixerApo() : CBaseAudioProcessingObject(&AudioMixerApoRegProperties), m_isProcessing(false) {}

    STDMETHODIMP Initialize(UINT32 cbDataSize, BYTE* pbyData) override;
    STDMETHODIMP_(void) APOProcess(
        _In_ UINT32 u32NumInputConnections,
        _In_reads_(u32NumInputConnections) APO_CONNECTION_PROPERTY** ppInputConnections,
        _In_ UINT32 u32NumOutputConnections,
        _Inout_updates_(u32NumOutputConnections) APO_CONNECTION_PROPERTY** ppOutputConnections) override;
};
