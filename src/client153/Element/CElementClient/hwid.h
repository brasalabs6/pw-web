#include <iostream>
#include <windows.h>

DWORD GetHWID() {
    char volumeName[MAX_PATH + 1] = { 0 };
    char fileSystemName[MAX_PATH + 1] = { 0 };
    DWORD serialNumber = 0;
    DWORD maxComponentLength = 0;
    DWORD fileSystemFlags = 0;

    // Obtém informações sobre o volume C:
    if (GetVolumeInformationA(
        "C:\\",
        volumeName,
        sizeof(volumeName),
        &serialNumber,
        &maxComponentLength,
        &fileSystemFlags,
        fileSystemName,
        sizeof(fileSystemName))) {
        
        // Usa a função alternativa para converter o número de série para string
        return serialNumber;
    }
    return 0;
}