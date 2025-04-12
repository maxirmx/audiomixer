#include "AudioMixerApo.h"
#include <propkey.h>

STDMETHODIMP AudioMixerApo::Initialize(UINT32 cbDataSize, BYTE* pbyData) {
    return S_OK;
}

STDMETHODIMP_(void) AudioMixerApo::Process(UINT32 numFrames, const APO_CONNECTION_PROPERTY** inputs,
                                           UINT32, APO_CONNECTION_PROPERTY** outputs) {
    FLOAT32* in = reinterpret_cast<FLOAT32*>(inputs[0]->pBuffer);
    FLOAT32* out = reinterpret_cast<FLOAT32*>(outputs[0]->pBuffer);
    for (UINT32 i = 0; i < numFrames; ++i) {
        out[i] = in[i] * 0.5f;
    }
}
