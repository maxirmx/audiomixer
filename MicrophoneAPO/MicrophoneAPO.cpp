
#include "MicrophoneAPO.h"
#include <mmreg.h>
#include <windows.h>
#include <memory>
#include <fstream>
#include <iostream>

#define SHMEM_NAME L"Global\\MicAPO_SharedBuffer"
#define BUFFER_SIZE (48000 * 2 * 2)

BYTE* g_SharedBuffer = nullptr;

HRESULT InitializeSharedBuffer() {
    HANDLE hMapFile = OpenFileMappingW(FILE_MAP_READ, FALSE, SHMEM_NAME);
    if (!hMapFile) return E_FAIL;
    g_SharedBuffer = (BYTE*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, BUFFER_SIZE);
    return g_SharedBuffer ? S_OK : E_FAIL;
}

void MixAudio(float* pOutput, const float* pMicInput, UINT32 frameCount) {
    if (!g_SharedBuffer) InitializeSharedBuffer();
    for (UINT32 i = 0; i < frameCount * 2; ++i) {
        float injected = g_SharedBuffer ? ((float*)g_SharedBuffer)[i % (BUFFER_SIZE / sizeof(float))] : 0.0f;
        pOutput[i] = pMicInput[i] + injected;
    }
}

extern "C" __declspec(dllexport) HRESULT CreateAudioProcessingObject(...) {
    return S_OK;
}
