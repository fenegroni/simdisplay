#include <windows.h>
#include <tchar.h>

#include "..\ACCSharedMemory\ACCSharedMemory.h"

#include <stdio.h>

int main(void)
{
    HANDLE phyMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCPhysics), TEXT("Local\\acpmf_physics"));
    if (!phyMap) {
        fprintf(stderr, "Error create file mapping for ACCPhysics.\n");
    }
    HANDLE graMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCGraphics), TEXT("Local\\acpmf_graphics"));
    if (!graMap) {
        fprintf(stderr, "Error create file mapping for ACCGraphics.\n");
    }
    HANDLE staMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCStatic), TEXT("Local\\acpmf_static"));
    if (!staMap) {
        fprintf(stderr, "Error create file mapping for ACCStatic.\n");
    }
    if (!staMap || !graMap || !phyMap) {
    fprintf(stderr, "Exiting with code 1\n");
    return 1;
    }

    struct ACCPhysics* phy = (struct ACCPhysics*) MapViewOfFile(phyMap, FILE_MAP_WRITE, 0, 0, 0);
    if (!phy) {
        fprintf(stderr, "Error mapping view ACCPhysics.\n");
    }
    struct ACCGraphics* gra = (struct ACCGraphics*) MapViewOfFile(graMap, FILE_MAP_WRITE, 0, 0, 0);
    if (!gra) {
        fprintf(stderr, "Error mapping view ACCGraphics.\n");
    }
    struct ACCStatic* sta = (struct ACCStatic*) MapViewOfFile(staMap, FILE_MAP_WRITE, 0, 0, 0);
    if (!sta) {
        fprintf(stderr, "Error mapping view ACCStatic.\n");
    }

    if (!phy || !gra || !sta) {
        fprintf(stderr, "Exiting with code 2\n");
        return 2;
    }

    for (int i = 0; i < 1000; ++i) {
        phy->abs = (float) (i % 2);
        phy->tc = (float) ((i+1) % 2);
        fprintf(stdout, "abs = %f, tc = %f\n", phy->abs, phy->tc);
        Sleep(1000);
    }
    return 0;
}
