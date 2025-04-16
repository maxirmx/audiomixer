
#include <windows.h>
#include <iostream>
#include <fstream>

#define SHMEM_NAME L"Global\\MicAPO_SharedBuffer"
#define BUFFER_SIZE (48000 * 2 * 2)

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Usage: APOController <file.wav>" << std::endl;
        return 1;
    }

    HANDLE hMapFile = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
                                         0, BUFFER_SIZE, SHMEM_NAME);
    if (!hMapFile) return 1;

    BYTE* pBuf = (BYTE*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
    if (!pBuf) return 2;

    std::ifstream file(argv[1], std::ios::binary);
    file.seekg(44);
    file.read((char*)pBuf, BUFFER_SIZE);
    std::cout << "WAV loaded and written to shared buffer." << std::endl;

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    return 0;
}
