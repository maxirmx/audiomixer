#include "pch.h"

typedef void (*StartAudioInjectionFunc)();
typedef void (*StopAudioInjectionFunc)();
typedef bool (*LoadAudioFileFunc)(const wchar_t*);

void PrintUsage() {
    std::wcout << L"Usage:\n";
    std::wcout << L"  AudioMixerApp.exe start\n";
    std::wcout << L"  AudioMixerApp.exe stop\n";
    std::wcout << L"  AudioMixerApp.exe load <path_to_audio_file>\n";
}

int wmain(int argc, wchar_t* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    HMODULE hModule = LoadLibrary(L"AudioMixerLib.dll");
    if (!hModule) {
        std::wcerr << L"Failed to load AudioMixerLib.dll\n";
        return 1;
    }

    StartAudioInjectionFunc StartAudioInjection = (StartAudioInjectionFunc)GetProcAddress(hModule, "StartAudioInjection");
    StopAudioInjectionFunc StopAudioInjection = (StopAudioInjectionFunc)GetProcAddress(hModule, "StopAudioInjection");
    LoadAudioFileFunc LoadAudioFile = (LoadAudioFileFunc)GetProcAddress(hModule, "LoadAudioFile");

    if (!StartAudioInjection || !StopAudioInjection || !LoadAudioFile) {
        std::wcerr << L"Failed to get function addresses\n";
        FreeLibrary(hModule);
        return 1;
    }

    std::wstring command = argv[1];
    if (command == L"start") {
        StartAudioInjection();
        std::wcout << L"Audio injection started.\n";
    }
    else if (command == L"stop") {
        StopAudioInjection();
        std::wcout << L"Audio injection stopped.\n";
    }
    else if (command == L"load" && argc == 3) {
        if (LoadAudioFile(argv[2])) {
            std::wcout << L"Audio file loaded successfully.\n";
        }
        else {
            std::wcerr << L"Failed to load audio file.\n";
        }
    }
    else {
        PrintUsage();
    }

    FreeLibrary(hModule);
    return 0;
}
