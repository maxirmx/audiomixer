#include "pch.h"

typedef bool (*InjectAudioFunc)(const wchar_t*);

static void PrintUsage() {
    std::wcout << L"Usage:\n";
    std::wcout << L"  AudioMixerApp.exe <path_to_audio_file>\n";
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    HMODULE hModule = LoadLibrary(L"AudioMixerLib.dll");
    if (!hModule) {
        std::wcerr << L"Failed to load AudioMixerLib.dll\n";
        return 1;
    }

    InjectAudioFunc InjectAudio = (InjectAudioFunc)GetProcAddress(hModule, "InjectAudio");
    if (!InjectAudio) {
        std::wcerr << L"Failed to get InjectAudio function address\n";
        FreeLibrary(hModule);
        return 1;
    }

    const wchar_t* filePath = argv[1];
    if (InjectAudio(filePath)) {
        std::wcout << L"Audio injection completed successfully.\n";
    }
    else {
        std::wcerr << L"Audio injection failed.\n";
    }

    FreeLibrary(hModule);
    return 0;
}
