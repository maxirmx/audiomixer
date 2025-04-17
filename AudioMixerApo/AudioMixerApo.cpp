#include "AudioMixerApo.h"
#include <propkey.h>

#define INITGUID 
#include <guiddef.h>
DEFINE_GUID(CLSID_AudioMixerApo,
    0xa6781b19, 0xfae6, 0x4a59, 0x9f, 0x19, 0xd0, 0xd2, 0xa2, 0x7a, 0x9b, 0x88);

STDMETHODIMP AudioMixerApo::Initialize(UINT32 /* cbDataSize */, BYTE* /* pbyData */) {
    return S_OK;
}

STDMETHODIMP_(void) AudioMixerApo::APOProcess(
    _In_ UINT32 /*u32NumInputConnections*/,
    _In_reads_(u32NumInputConnections) APO_CONNECTION_PROPERTY** ppInputConnections,
    _In_ UINT32 /*u32NumOutputConnections*/,
    _Inout_updates_(u32NumOutputConnections) APO_CONNECTION_PROPERTY** ppOutputConnections)

{  
   FLOAT32* in = reinterpret_cast<FLOAT32*>(ppInputConnections[0]->pBuffer);  
   FLOAT32* out = reinterpret_cast<FLOAT32*>(ppOutputConnections[0]->pBuffer);  
   for (UINT32 i = 0; i < ppInputConnections[0]->u32ValidFrameCount; ++i) {  
       out[i] = in[i] * 0.5f;  
   }  
}

