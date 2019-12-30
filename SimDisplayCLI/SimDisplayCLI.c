#include <stdio.h>

#include <windows.h>

HANDLE graphics, stat, physics;

int main(void) {
	fprintf(stderr, "Opening mmap'd files.");

	HANDLE hFile = CreateFileA("C:\\tmp\\x.txt", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == hFile) {
		fprintf(stderr, "Could not open file.\n");
		return 1;
	}

	HANDLE mapped = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!mapped) {
		fprintf(stderr, "Could not map file.\n");
		return 1;
	}

	void* x = MapViewOfFile(mapped, FILE_MAP_READ, 0, 0, 0);
	if (!x) {
		fprintf(stderr, "Could not map view of file.\n");
		return 1;
	}

	for (int t = 0; t < 1000000000; ++t) {
		char* y = x;
		for (int i = 0; i < 4; ++i) {
			//fprintf(stdout, "%c", *y);
			++y;
		}
		//fprintf(stdout, "\n");
	}

	return 0;
}