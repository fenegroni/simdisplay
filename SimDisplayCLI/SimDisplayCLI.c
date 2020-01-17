#include <windows.h>
#include <tchar.h>

#include "..\ACCSharedMemory\ACCSharedMemory.h"

#include <stdio.h>

int main(void)
{
    int err = 0;

    HANDLE phyMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCPhysics), TEXT("Local\\acpmf_physics"));
    if (!phyMap) {
        fprintf(stderr, "Error: create file mapping for ACCPhysics.\n");
        err = 1;
    } else if (ERROR_ALREADY_EXISTS != GetLastError()) {
        fprintf(stderr, "Error: ACCPhysics does not exist.\n");
        err = 2;
    }
    HANDLE graMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCGraphics), TEXT("Local\\acpmf_graphics"));
    if (!graMap) {
        fprintf(stderr, "Error: create file mapping for ACCGraphics.\n");
        err = 1;
    } else if (ERROR_ALREADY_EXISTS != GetLastError()) {
        fprintf(stderr, "Error: ACCGraphics does not exist.\n");
        err = 2;
    }
    HANDLE staMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCStatic), TEXT("Local\\acpmf_static"));
    if (!staMap) {
        fprintf(stderr, "Error: create file mapping for ACCStatic.\n");
        err = 1;
    } else if (ERROR_ALREADY_EXISTS != GetLastError()) {
        fprintf(stderr, "Error: ACCStatic does not exist.\n");
        err = 2;
    }
    if (err) {
        fprintf(stderr, "Exiting with code %d\n", err);
        return err;
    }

    struct ACCPhysics* phy = (struct ACCPhysics*) MapViewOfFile(phyMap, FILE_MAP_READ, 0, 0, 0);
    if (!phy) {
        fprintf(stderr, "Error: mapping view ACCPhysics.\n");
        err = 3;
    }
    struct ACCGraphics* gra = (struct ACCGraphics*) MapViewOfFile(graMap, FILE_MAP_READ, 0, 0, 0);
    if (!gra) {
        fprintf(stderr, "Error: mapping view ACCGraphics.\n");
        err = 3;
    }
    struct ACCStatic* sta = (struct ACCStatic*) MapViewOfFile(staMap, FILE_MAP_READ, 0, 0, 0);
    if (!sta) {
        err = 3;
        fprintf(stderr, "Error: mapping view ACCStatic.\n");
    }
    if (err) {
        fprintf(stderr, "Exiting with code %d\n", err);
        return err;
    }

    HANDLE comPort = CreateFile(L"\\.\\COM3", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (comPort == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error: open serial port \\.\\COM3\n");
        fprintf(stderr, "Exiting with code 10\n");
        return 10;
    }

    return 0;
}
