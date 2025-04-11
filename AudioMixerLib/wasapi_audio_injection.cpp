#include "pch.h"

class AudioInjector {
public:
    AudioInjector()
        : running(false), customAudioOffset(0) {
    }

    bool InjectAudio(const wchar_t* filepath) {
        if (running) {
            std::cerr << "Audio injection is already running.\n";
            return false;
        }

        customAudioBuffer.clear();
        customAudioOffset = 0;

        if (!LoadWavFile(filepath, customAudioBuffer, customFormat)) {
            std::cerr << "Failed to load audio file.\n";
            return false;
        }

        running = true;

        std::promise<bool> promise;
        std::future<bool> future = promise.get_future();

        // Pass the promise to the thread
        captureThread = std::thread(&AudioInjector::CaptureAudio, this, std::move(promise));

        // Wait for the thread to signal success or failure
        bool result = future.get(); // Blocks until the promise is fulfilled
        running = false;

        return result;
    }

    void CancelAudioInjection() {
        if (!running) {
            std::cerr << "Audio injection is not running.\n";
            return;
        }

        // Set the running flag to false to stop the loop in CaptureAudio
        running = false;

        // Wait for the capture thread to finish
        if (captureThread.joinable()) {
            captureThread.join();
        }

        std::cout << "Audio injection has been canceled.\n";
    }

    bool IsAudioInjectionRunning() const {
        return running;
    }

private:
    std::thread captureThread;
    std::atomic<bool> running;
    std::vector<BYTE> customAudioBuffer;
    size_t customAudioOffset;
    WAVEFORMATEX customFormat;

    static constexpr DWORD LOOPBACK_BUFFER_DURATION = 10000000; // 1 second in 100-nanosecond units
    static constexpr DWORD SLEEP_DURATION_MS = 5;

    bool LoadWavFile(const wchar_t* filepath, std::vector<BYTE>& buffer, WAVEFORMATEX& format) {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) {
            // Replace the problematic line with the following code
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            std::cerr << "Failed to open WAV file: " << converter.to_bytes(filepath) << "\n";
            return false;
        }

        char chunkId[4] = { 0 };
        DWORD chunkSize = 0;
        char formatType[4] = { 0 };

        file.read(chunkId, 4);
        file.read(reinterpret_cast<char*>(&chunkSize), 4);
        file.read(formatType, 4);

        if (strncmp(chunkId, "RIFF", 4) != 0 || strncmp(formatType, "WAVE", 4) != 0) {
            std::cerr << "Invalid WAV file format.\n";
            return false;
        }

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

    bool MixAudio(BYTE* dest, UINT32 frames, WAVEFORMATEX* format) {
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

    static void SafeRelease(IUnknown* pInterface) {
        if (pInterface) {
            pInterface->Release();
        }
    }

    void CaptureAudio(std::promise<bool> promise) {
        HRESULT hr = CoInitialize(nullptr);
        if (FAILED(hr)) {
            std::cerr << "CoInitialize failed: " << std::hex << hr << "\n";
            promise.set_value(false); // Signal failure
            return;
        }

        IMMDeviceEnumerator* _pEnumerator = nullptr;
        IMMDevice* _pDevice = nullptr;
        IAudioClient* _pAudioClient = nullptr;
        IAudioCaptureClient* _pCaptureClient = nullptr;

        hr = CoCreateInstance(
            __uuidof(MMDeviceEnumerator),
            nullptr,
            CLSCTX_ALL,
            __uuidof(IMMDeviceEnumerator),
            reinterpret_cast<void**>(&_pEnumerator));

        if (FAILED(hr)) {
            std::cerr << "Failed to create IMMDeviceEnumerator: " << std::hex << hr << "\n";
            promise.set_value(false);
            return;
        }
        std::unique_ptr<IMMDeviceEnumerator, decltype(&AudioInjector::SafeRelease)> pEnumerator(_pEnumerator, &AudioInjector::SafeRelease);

        hr = pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &_pDevice);
        if (FAILED(hr)) {
            std::cerr << "Failed to get default audio endpoint: " << std::hex << hr << "\n";
            promise.set_value(false);
            return;
        }
        std::unique_ptr<IMMDevice, decltype(&SafeRelease)> pDevice(_pDevice, &AudioInjector::SafeRelease);

        hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, reinterpret_cast<void**>(&_pAudioClient));
        if (FAILED(hr)) {
            std::cerr << "Failed to activate IAudioClient: " << std::hex << hr << "\n";
            promise.set_value(false);
            return;
        }
        std::unique_ptr<IAudioClient, decltype(&SafeRelease)> pAudioClient(_pAudioClient, &AudioInjector::SafeRelease);

        WAVEFORMATEX* pwfx = nullptr;
        hr = pAudioClient->GetMixFormat(&pwfx);
        if (FAILED(hr)) {
            std::cerr << "Failed to get mix format: " << std::hex << hr << "\n";
            promise.set_value(false);
            return;
        }

        hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
            AUDCLNT_STREAMFLAGS_LOOPBACK,
            LOOPBACK_BUFFER_DURATION, 0, pwfx, nullptr);

        if (FAILED(hr)) {
            std::cerr << "Failed to initialize audio client: " << std::hex << hr << "\n";
            promise.set_value(false);
            return;
        }

        hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), reinterpret_cast<void**>(&_pCaptureClient));
        if (FAILED(hr)) {
            std::cerr << "Failed to get audio capture client: " << std::hex << hr << "\n";
            promise.set_value(false);
            return;
        }
        std::unique_ptr<IAudioCaptureClient, decltype(&SafeRelease)> pCaptureClient(_pCaptureClient, &AudioInjector::SafeRelease);

        hr = pAudioClient->Start();
        if (FAILED(hr)) {
            std::cerr << "Failed to start audio client: " << std::hex << hr << "\n";
            promise.set_value(false);
            return;
        }

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

            Sleep(SLEEP_DURATION_MS);
        }

        pAudioClient->Stop();
        promise.set_value(true); // Signal success
        CoUninitialize();
    }
};

// Singleton instance of AudioInjector
static AudioInjector& GetAudioInjectorInstance() {
    static AudioInjector instance;
    return instance;
}

extern "C" __declspec(dllexport) bool InjectAudio(const wchar_t* filepath) {
    return GetAudioInjectorInstance().InjectAudio(filepath);
}

extern "C" __declspec(dllexport) void CancelAudioInjection() {
    GetAudioInjectorInstance().CancelAudioInjection();
}

extern "C" __declspec(dllexport) bool IsAudioInjectionRunning() {
    return GetAudioInjectorInstance().IsAudioInjectionRunning();
}