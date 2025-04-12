#pragma once
#include <audioenginebaseapo.h>
#include "AudioMixerGuids.h"

class AudioMixerApo : public CBaseAudioProcessingObject {
public:
    AudioMixerApo() : CBaseAudioProcessingObject(&CLSID_AudioMixerApo, 1, 1, true) {}
    STDMETHOD(Initialize)(UINT32 cbDataSize, BYTE* pbyData) override;
    STDMETHOD_(void, Process)(UINT32 u32NumInputFrames, const APO_CONNECTION_PROPERTY** ppInputConnections,
                              UINT32 u32NumOutputFrames, APO_CONNECTION_PROPERTY** ppOutputConnections) override;
};
