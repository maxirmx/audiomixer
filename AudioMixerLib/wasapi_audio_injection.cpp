#include "pch.h"

static std::thread captureThread;
static std::atomic<bool> running = false;
static std::vector<BYTE> customAudioBuffer;
static size_t customAudioOffset = 0;
static WAVEFORMATEX customFormat = {};

extern "C" __declspec(dllexport) void StartAudioInjection();
extern "C" __declspec(dllexport) void StopAudioInjection();
extern "C" __declspec(dllexport) bool LoadAudioFile(const wchar_t* filepath);

static bool LoadWavFile(const wchar_t* filepath, std::vector<BYTE>& buffer, WAVEFORMATEX& format) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) return false;

    char chunkId[4];
    DWORD chunkSize;
    char formatType[4];

    file.read(chunkId, 4);
    file.read(reinterpret_cast<char*>(&chunkSize), 4);
    file.read(formatType, 4);

    if (strncmp(chunkId, "RIFF", 4) != 0 || strncmp(formatType, "WAVE", 4) != 0) return false;

    while (file.read(chunkId, 4)) {
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        if (strncmp(chunkId, "fmt ", 4) == 0) {
            file.read(reinterpret_cast<char*>(&format), sizeof(WAVEFORMATEX));
            file.ignore(chunkSize - sizeof(WAVEFORMATEX));
        }
        else if (strncmp(chunkId, "data", 4) == 0) {
            buffer.resize(chunkSize);
            file.read(reinterpret_cast<char*>(buffer.data()), chunkSize);
            break;
        }
        else {
            file.ignore(chunkSize);
        }
    }

    return true;
}

static bool MixAudio(BYTE* dest, UINT32 frames, WAVEFORMATEX* format) {
    if (customAudioBuffer.empty() || format->wFormatTag != WAVE_FORMAT_PCM) return false;

    size_t bytesPerFrame = format->nBlockAlign;
    size_t totalBytes = frames * bytesPerFrame;

    for (size_t i = 0; i < totalBytes && customAudioOffset + i < customAudioBuffer.size(); ++i) {
        int mixed = static_cast<int>(dest[i]) + static_cast<int>(customAudioBuffer[customAudioOffset + i]) - 128;
        dest[i] = static_cast<BYTE>(max(0, min(255, mixed)));
    }

    customAudioOffset += totalBytes;
    if (customAudioOffset >= customAudioBuffer.size()) customAudioOffset = 0; // loop
    return true;
}

static void CaptureAudio() {
    HRESULT hr;
    hr = CoInitialize(nullptr);
    if (FAILED(hr)) return;

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) return;

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) return;

    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pAudioClient);
    if (FAILED(hr)) return;

    WAVEFORMATEX* pwfx = nullptr;
    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) return;

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        10000000, 0, pwfx, nullptr);
    if (FAILED(hr)) return;

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    if (FAILED(hr)) return;

    hr = pAudioClient->Start();
    if (FAILED(hr)) return;

    while (running) {
        UINT32 packetLength = 0;
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) break;

        while (packetLength > 0) {
            BYTE* pData;
            UINT32 numFramesAvailable;
            DWORD flags;

            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
            if (FAILED(hr)) break;

            MixAudio(pData, numFramesAvailable, pwfx);

            pCaptureClient->ReleaseBuffer(numFramesAvailable);
            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) break;
        }

        Sleep(5);
    }

    pAudioClient->Stop();
    if (pCaptureClient) pCaptureClient->Release();
    if (pAudioClient) pAudioClient->Release();
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();

    CoUninitialize();
}

extern "C" __declspec(dllexport) void StartAudioInjection() {
    if (running) return;
    running = true;
    captureThread = std::thread(CaptureAudio);
}

extern "C" __declspec(dllexport) void StopAudioInjection() {
    if (!running) return;
    running = false;
    if (captureThread.joinable())
        captureThread.join();
}

extern "C" __declspec(dllexport) bool LoadAudioFile(const wchar_t* filepath) {
    customAudioBuffer.clear();
    customAudioOffset = 0;
    return LoadWavFile(filepath, customAudioBuffer, customFormat);
}
