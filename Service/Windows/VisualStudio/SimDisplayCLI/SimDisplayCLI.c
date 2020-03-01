/*
SimDisplay - A simracing dashboard created using Arduino to show shared memory
             telemetry from Assetto Corsa Competizione.

Copyright (C) 2020  Filippo Erik Negroni

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#define SIMDISPLAYCLI_VERSION 0x0801
#define SIMDISPLAYCLI_VERSION_STRING "8.1"

#include <windows.h>
#include <string.h>

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "..\include\ACCSharedMemory.h"
#include "..\include\SimDisplayProtocol.h"

enum mapAcpmf_action {
	MAPACPMF_CREATE,
	MAPACPMF_OPEN_EXISTING
};

static struct CarModelData {
	int optRpm;
	int shiftRpm;
	float bbOffset;
	wchar_t *carModel;
} carModelDict[] = {
	{ 0, 0, -70.0f,		L"amr_v12_vantage_gt3" },
	{ 0, 0, -70.0f,		L"amr_v8_vantage_gt3" },
	{ 0, 0, -140.0f,	L"audi_r8_lms" },
	{ 0, 0, -140.0f,	L"audi_r8_lms_evo" },
	{ 0, 0, -70.0f,		L"bentley_continental_gt3_2016" },
	{ 6300, 7100, -70.0f,	L"bentley_continental_gt3_2018" },
	{ 0, 0, -150.0f,	L"bmw_m6_gt3" },
	{ 0, 0, -70.0f,		L"jaguar_g3" },
	{ 0, 0, -170.0f,	L"ferrari_488_gt3" },
	{ 0, 0, -140.0f,	L"honda_nsx_gt3" },
	{ 0, 0, -140.0f,	L"honda_nsx_gt3_evo" },
	{ 0, 0, -140.0f,	L"lamborghini_gallardo_rex" },
	{ 0, 0, -150.0f,	L"lamborghini_huracan_gt3" },
	{ 0, 0, -140.0f,	L"lamborghini_huracan_gt3_evo" },
	{ 0, 0, -140.0f,	L"lamborghini_huracan_st" },
	{ 0, 0, -140.0f,	L"lexus_rc_f_gt3" },
	{ 0, 0, -170.0f,	L"mclaren_650s_gt3" },
	{ 0, 0, -170.0f,	L"mclaren_720s_gt3" },
	{ 0, 0, -150.0f,	L"mercedes_amg_gt3" },
	{ 0, 0, -150.0f,	L"nissan_gt_r_gt3_2017" },
	{ 0, 0, -150.0f,	L"nissan_gt_r_gt3_2018" },
	{ 0, 0, -60.0f,		L"porsche_991_gt3_r" },
	{ 0, 0, -150.0f,	L"porsche_991ii_gt3_cup" },
	{ 0, 0, -210.0f,	L"porsche_991ii_gt3_r" },
};

struct CarModelData * lookupCarModelData(wchar_t *carModel)
{
	for (int i = 0; i < (sizeof carModelDict / sizeof(struct CarModelData)); ++i) {
		if (!wcscmp(carModelDict[i].carModel, carModel)) {
			return &carModelDict[i];
		}
	}
	return 0;
}

int populateCarModelData(struct CarModelData *retdata, int maxRpm)
{
	struct CarModelData *data = lookupCarModelData(retdata->carModel);
	if (data) {
		*retdata = *data;
	} else {
		retdata->carModel = L"default_car_data";
		retdata->bbOffset = 0.0f;
		retdata->optRpm = 0;
		retdata->shiftRpm = 0;
	}
	// FIXME: once the LUT is fully populated this won't be necessary anymore
	if (0 == retdata->optRpm) {
		retdata->optRpm = maxRpm * 85 / 100;
	}
	if (0 == retdata->shiftRpm) {
		retdata->shiftRpm = maxRpm * 95 / 100;
	}
	return data ? 1 : 0;
}

const wchar_t acpmf_physics[] = L"Local\\acpmf_physics";
const wchar_t acpmf_graphics[] = L"Local\\acpmf_graphics";
const wchar_t acpmf_static[] = L"Local\\acpmf_static";

int mapAcpmf(enum mapAcpmf_action action, struct ACCPhysics **phy, struct ACCGraphics **gra, struct ACCStatic **sta)
{
	int err = 0;

	HANDLE phyMap, graMap, staMap;

	if (MAPACPMF_OPEN_EXISTING == action) {
		while (!(phyMap = OpenFileMapping(FILE_MAP_READ, FALSE, acpmf_physics))) {
			fprintf(stderr, "Waiting: open file mapping for ACCPhysics.\n");
			Sleep(1000);
		}
		graMap = OpenFileMapping(FILE_MAP_READ, FALSE, acpmf_graphics); 
		staMap = OpenFileMapping(FILE_MAP_READ, FALSE, acpmf_static);
	} else {
		phyMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCPhysics), acpmf_physics);
		graMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCGraphics), acpmf_graphics);
		staMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct ACCStatic), acpmf_static);
	}
	if (!phyMap) {
		fprintf(stderr, "Error: %d: file mapping ACCPhysics.\n", action);
		err = 1;
	}
	if (!graMap) {
		fprintf(stderr, "Error: %d: file mapping ACCGraphics.\n", action);
		err = 1;
	}
	if (!staMap) {
		fprintf(stderr, "Error: %d: file mapping ACCStatic.\n", action);
		err = 1;
	}
	if (err) {
		return err;
	}

	*phy = (struct ACCPhysics *) MapViewOfFile(phyMap, (action == MAPACPMF_OPEN_EXISTING) ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);
	if (!phy) {
		fprintf(stderr, "Error: mapping view ACCPhysics.\n");
		err = 1;
	}
	*gra = (struct ACCGraphics *) MapViewOfFile(graMap, (action == MAPACPMF_OPEN_EXISTING) ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);
	if (!gra) {
		fprintf(stderr, "Error: mapping view ACCGraphics.\n");
		err = 1;
	}
	*sta = (struct ACCStatic *) MapViewOfFile(staMap, (action == MAPACPMF_OPEN_EXISTING) ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);
	if (!sta) {
		err = 1;
		fprintf(stderr, "Error: mapping view ACCStatic.\n");
	}

	return err;
}

uint16_t bbFromBrakeBias(float brakeBias, float offset)
{
	return brakeBias ? (uint16_t)(brakeBias * 1000.0f + offset + 0.5f) : 0;
}

int doSend(int argc, const wchar_t *argv[])
{
	const wchar_t *comPortName = argv[0];

	if (!comPortName) {
		fprintf(stderr, "usage: send <serial_port>\n\n");
		fprintf(stderr, "<serial_port> is the name of the serial port the device is attached to.\n");
		return 1;
	}

	struct ACCPhysics *phy;
	struct ACCGraphics *gra;
	struct ACCStatic *sta;

	if (mapAcpmf(MAPACPMF_OPEN_EXISTING, &phy, &gra, &sta)) {
		return 1;
	}

	HANDLE comPort = CreateFile(comPortName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (comPort == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Error: open serial port %S\n", comPortName);
		return 1;
	}

	DCB comPortDCB;

	if (!GetCommState(comPort, &comPortDCB)) { //retreives  the current settings
		fprintf(stderr, "Error: get COM state.");
		return 1;
	}

	comPortDCB.BaudRate = CBR_9600;
	comPortDCB.ByteSize = 8;
	comPortDCB.StopBits = ONESTOPBIT;
	comPortDCB.Parity = NOPARITY;

	if (!SetCommState(comPort, &comPortDCB)) {
		fprintf(stderr, "Error: set COM state.");
		return 1;
	}

	HANDLE sendTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (NULL == sendTimer) {
		fprintf(stderr, "Error: create timer.\n");
		return 1;
	}
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -400000LL; // 40ms == 25Hz
	if (!SetWaitableTimer(sendTimer, &dueTime, 40, NULL, NULL, FALSE)) {
		printf("Error: SetWaitableTimer: %d\n", GetLastError());
		return 1;
	}

	struct SimDisplayPacket packet;
	struct CarModelData cardata = { 0 };
	int prevStatus = ACC_OFF; // TODO: we could use packet-> status as the previous status...
	while (WaitForSingleObject(sendTimer, INFINITE) == WAIT_OBJECT_0) {
		if (ACC_LIVE != gra->status && prevStatus == gra->status) continue;
		if (gra->status != prevStatus && ACC_LIVE == gra->status ) {
			cardata.carModel = sta->carModel;
			populateCarModelData(&cardata, sta->maxRpm);
		}
		packet.status = prevStatus = gra->status;
		packet.rpm = phy->rpms;
		packet.optrpm = cardata.optRpm;
		packet.shftrpm = cardata.shiftRpm;
		packet.pitlim = phy->pitLimiterOn;
		packet.gear = phy->gear; // 0 = Reverse, 1 = Neutra, 2 = 1st, 3 = 2nd, ..., 7 = 6th.
		packet.tc = gra->TC;
		packet.tcc = gra->TCCut;
		packet.tcact = (uint8_t)phy->tc;
		packet.abs = gra->ABS;
		packet.absact = (uint8_t)phy->abs;
		packet.bb = bbFromBrakeBias(phy->brakeBias, cardata.bbOffset);
		packet.remlaps = (uint8_t)gra->fuelEstimatedLaps; // Only full laps are useful to the driver.
		packet.map = gra->EngineMap + 1;
		packet.airt = (uint8_t)phy->airTemp; // match ACC's UI: floor instead of round.
		packet.roadt = (uint8_t)phy->roadTemp; // match ACC's UI: floor instead of round.

		DWORD bytesWritten;
		WriteFile(comPort, &packet, sizeof(packet), &bytesWritten, NULL);
		// FIXME: validate bytes written and return status.
		// TODO: if error, stop writing completely or pause?
		// TODO: will the serial port be open?
		// TODO: what if I disconnect and reconnect the arduino?
	}
	fprintf(stderr, "Error: WaitForSingleObject: %d\n", GetLastError());
	return 1;
}

int doHelpSave(void)
{
	fprintf(stderr,
		"usage: SimDisplayCLI save [<prefix>]\n\n"
		"Saves shared memory from ACC gaming sessions to a local file named\n\n"
		"  prefixYYYMMDD-HHMMSS.bin\n\n"
		"where prefix is the (optional) first command line argument to this command.\n"
	);
	return 0;
}

int doSave(int argc, const wchar_t *argv[])
{
	const wchar_t *prefix = L"";
	if (argc) {
		prefix = argv[0];
	}

	struct ACCPhysics *phy;
	struct ACCGraphics *gra;
	struct ACCStatic *sta;

	if (mapAcpmf(MAPACPMF_OPEN_EXISTING, &phy, &gra, &sta)) {
		return 1;
	}

	HANDLE saveTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (NULL == saveTimer) {
		fprintf(stderr, "Error: create timer.\n");
		return 1;
	}
	LARGE_INTEGER dueTime;
	dueTime.QuadPart = -200000LL; // 20ms == 50Hz
	if (!SetWaitableTimer(saveTimer, &dueTime, 20, NULL, NULL, FALSE)) {
		printf("Error: SetWaitableTimer: %d\n", GetLastError());
		return 1;
	}

	wchar_t binfilename[MAX_PATH];
	SYSTEMTIME dt;
	GetSystemTime(&dt);
	if (0 > swprintf(binfilename, MAX_PATH, L"%s%d%02d%02d-%02d%02d%02d.bin",
		prefix, dt.wYear, dt.wMonth, dt.wDay, dt.wHour, dt.wMinute, dt.wSecond)) {
		fprintf(stderr, "Error: prefix is too long.\n");
		return 1;
	}
	HANDLE output = CreateFile(binfilename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == output) {
		fprintf(stderr, "Error: create %S: %d\n", binfilename, GetLastError());
		return 1;
	}
	DWORD bytesWritten;
	uint16_t versioninfo[2] = { SIMDISPLAYCLI_VERSION, ACCSHAREDMEMORY_VERSION };
	WriteFile(output, versioninfo, sizeof versioninfo, &bytesWritten, NULL);
	if (bytesWritten < sizeof versioninfo) {
		fprintf(stderr, "Error: write to %S: %d\n", binfilename, GetLastError());
		return 1;
	}
	while (1) {
		DWORD totalBytesWritten;
		WriteFile(output, phy, sizeof(*phy), &bytesWritten, NULL);
		totalBytesWritten = bytesWritten;
		WriteFile(output, gra, sizeof(*gra), &bytesWritten, NULL);
		totalBytesWritten += bytesWritten; 
		WriteFile(output, sta, sizeof(*sta), &bytesWritten, NULL);
		totalBytesWritten += bytesWritten;
		if (totalBytesWritten < (sizeof(*sta) + sizeof(*gra) + sizeof(*phy))) {
			fprintf(stderr, "Error: write to %S: %d\n", binfilename, GetLastError());
			return 1;
		}
		if (WaitForSingleObject(saveTimer, INFINITE) != WAIT_OBJECT_0) {
			fprintf(stderr, "Error: WaitForSingleObject: %d\n", GetLastError());
			return 1;
		}
	}
	return 0;
}

static void printRedline(char leds[9], uint16_t rpm, uint16_t shiftrpm, uint16_t optrpm)
{
	if (rpm > shiftrpm) {
		for (int led = 0; led < 8; ++led) {
			leds[led] = 'b';
		}
	} else {
		int steprpm = (shiftrpm - optrpm) / 8;
		for (int led = 0, ledrpm = optrpm; led < 8; ++led, ledrpm += steprpm) {
			if (rpm > ledrpm) {
				leds[led] = '1';
			} else {
				leds[led] = '0';
			}
		}
	}
	leds[8] = '\0';
}

int doCsv(int argc, const wchar_t *argv[])
{
	HANDLE input, output;

	if (!argc) {
		input = GetStdHandle(STD_INPUT_HANDLE);
		if (input == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "Error: GetStdHandle STD_INPUT_HANDLE: %d\n", GetLastError());
			return 1;
		}
		output = GetStdHandle(STD_OUTPUT_HANDLE);
		if (output == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "Error: GetStdHandle STD_OUTPUT_HANDLE: %d\n", GetLastError());
			return 1;
		}
	} else {
		if (argc != 2) {
			fprintf(stderr, "usage: csv [<input_file_name> <output_file_name>]\n\n");
			fprintf(stderr, "<input_file_name> is a session data file created using the save command.\n");
			fprintf(stderr, "<output_file_name> is the name of the CSV output by this command.\n");
			fprintf(stderr, "If you don't specify the two file names, this command will read from stdin\n");
			fprintf(stderr, "and write to stdout.\n");
			return 1;
		}
		input = CreateFile(argv[0], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == input) {
			fprintf(stderr, "Error: open %S: %d\n", argv[0], GetLastError());
			return 1;
		}
		output = CreateFile(argv[1], GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE == output) {
			fprintf(stderr, "Error: create %S: %d\n", argv[1], GetLastError());
			return 1;
		}
	}

	int maxCsvRecord = 8192;
	char *csvRecord = malloc(maxCsvRecord);
	if (!csvRecord) ExitProcess(1);
	DWORD writtenBytes;
	if (!WriteFile(output, csvRecord,
			snprintf(csvRecord, maxCsvRecord,
					"status,rpm,maxrpm,optrpm,shiftrpm,leds,pitlimiteron,gear,"
					"tc,tccut,tcaction,itcaction,abs,absaction,iabsaction,"
					"bb,ibb,fuellaps,map,airt,roadt,car_model\n"),
			&writtenBytes, NULL)) {
		fprintf(stderr, "Error: write CSV header: %d\n", GetLastError());
		return 1;
	}
	int binBufferSize = sizeof(struct ACCPhysics) + sizeof(struct ACCGraphics) + sizeof(struct ACCStatic);
	char *binBuffer = malloc(binBufferSize);
	if (!binBuffer) ExitProcess(1);
	struct ACCPhysics *phy = (struct ACCPhysics *)binBuffer;
	struct ACCGraphics *gra = (struct ACCGraphics *)(binBuffer + sizeof(struct ACCPhysics));
	struct ACCStatic *sta = (struct ACCStatic *)(binBuffer + sizeof(struct ACCPhysics) + sizeof(struct ACCGraphics));
	DWORD readBytes;
	char leds[9];
	while (ReadFile(input, binBuffer, binBufferSize, &readBytes, NULL) && readBytes == binBufferSize) {
		struct CarModelData data;
		data.carModel = sta->carModel;
		populateCarModelData(&data, sta->maxRpm);
		printRedline(leds, phy->rpms, data.shiftRpm, data.optRpm);
		if (!WriteFile(output, csvRecord,
				snprintf(csvRecord, maxCsvRecord,
					"%d,%d,%d,%d,%d,%s,%d,%d,"
					"%d,%d,%f,%u,%d,%f,%u,"
					"%f,%u,%f,%d,%f,%f,%S\n",
					gra->status, phy->rpms, sta->maxRpm, data.optRpm, data.shiftRpm, leds, phy->pitLimiterOn, phy->gear,
					gra->TC, gra->TCCut, phy->tc, (uint8_t)phy->tc, gra->ABS, phy->abs, (uint8_t)phy->abs,
					phy->brakeBias, bbFromBrakeBias(phy->brakeBias, data.bbOffset), gra->fuelEstimatedLaps, gra->EngineMap+1, phy->airTemp, phy->roadTemp, sta->carModel),
				&writtenBytes, NULL)) {
			fprintf(stderr, "Error: write CSV record: %d\n", GetLastError());
			return 1;
		}
	}
	return 0;
}

int doReplay(int argc, const wchar_t *argv[])
{
	HANDLE stdinh;
	HANDLE *input;

	if (!argc) {
		stdinh = GetStdHandle(STD_INPUT_HANDLE);
		input = &stdinh;
		if (*input == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "Error: GetStdHandle STD_INPUT_HANDLE: %d\n", GetLastError());
			return 1;
		}
		argc = 1;
		argv[0] = L"stdin";
	} else {
		input = malloc(argc * sizeof(HANDLE));
		if (!input) ExitProcess(1);
		for (int i = 0; i < argc; ++i) {
			input[i] = CreateFile(argv[i], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (INVALID_HANDLE_VALUE == input[i]) {
				fprintf(stderr, "Error: open %S: %d\n", argv[i], GetLastError());
				return 1;
			}
		}
	}
	
	struct ACCPhysics *phy;
	struct ACCGraphics *gra;
	struct ACCStatic *sta;

	if (mapAcpmf(MAPACPMF_CREATE, &phy, &gra, &sta)) {
		return 1;
	}

	HANDLE replayTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (NULL == replayTimer) {
		fprintf(stderr, "Error: create timer.\n");
		return 1;
	}

	for (int i = 0; i < argc; ++i) {
		LARGE_INTEGER dueTime;
		dueTime.QuadPart = -200000LL; // 20ms == 50Hz
		if (!SetWaitableTimer(replayTimer, &dueTime, 20, NULL, NULL, FALSE)) {
			printf("Error: SetWaitableTimer: %d\n", GetLastError());
			return 1;
		}
		DWORD phyBytesRead;
		DWORD graBytesRead;
		DWORD staBytesRead;
		do {
			if (WaitForSingleObject(replayTimer, INFINITE) != WAIT_OBJECT_0) {
				fprintf(stderr, "Error: WaitForSingleObject: %d\n", GetLastError());
				return 1;
			}
			if (!ReadFile(input[i], phy, sizeof(*phy), &phyBytesRead, NULL)
				|| !ReadFile(input[i], gra, sizeof(*gra), &graBytesRead, NULL)
				|| !ReadFile(input[i], sta, sizeof(*sta), &staBytesRead, NULL)) {
				fprintf(stderr, "Error: ReadFile %S: %d\n", argv[i], GetLastError());
				return 1;
			}
		} while (phyBytesRead || graBytesRead || staBytesRead);
	}

	return 0;
}

int doVersion(void)
{
	puts(
		"SimDisplayCLI version " SIMDISPLAYCLI_VERSION_STRING "\n"
		"SimDisplay protocol version " SIMDISPLAYPROTOCOL_VERSION_STRING "\n"
		"ACCSharedMemory version " ACCSHAREDMEMORY_VERSION_STRING "\n"
		);
	return 0;
}

int doLicense(void)
{
	puts(
"SimDisplay - A simracing dashboard created using Arduino to show shared memory\n"
"	      telemetry from Assetto Corsa Competizione.\n"
"\n"
"Copyright(C) 2020  Filippo Erik Negroni\n"
"\n"
"This program is free software : you can redistribute it and /or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation, either version 3 of the License, or\n"
"(at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the\n"
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License\n"
"along with this program.If not, see <https://www.gnu.org/licenses/>.\n"
	);
	return 0;
}

int doHelp(int argc, const wchar_t *argv[])
{
	if (argc) {
		if (!wcscmp(argv[0], L"send")) {
			return 1; // doHelpSend();
		}
		if (!wcscmp(argv[0], L"save")) {
			return doHelpSave();
		}
		if (!wcscmp(argv[0], L"csv")) {
			return 1; // doHelpCsv();
		}
		if (!wcscmp(argv[0], L"replay")) {
			return 1; // doHelpReplay();
		}
	}
	puts(
		"usage: SimDisplayCLI help <command>\n"
		"\n"
		"Commands are send, save, csv, replay.\n"
	);
	return 1;
}

int doUsage(void)
{
	puts(
		"usage: SimDisplayCLI <command> [<args>]\n"
		"\n"
		"Commands are:\n"
		"  help    get usage help for a command\n"
		"  send    transmit data to device over serial connection\n"
		"  save    saves a gaming session to file\n"
		"  csv     convert data from a saved session into a CSV format file\n"
		"  replay  reads a saved session and populates shared memory\n"
		"  version prints version information\n"
		"  license prints the license terms\n"
	);
	return 0;
}

int wmain(int argc, const wchar_t *argv[])
{
	enum { USAGE, HELP, SEND, SAVE, CSV, REPLAY, VERSION, LICENSE } action = USAGE;

	if (argc > 1) {
		if (!wcscmp(argv[1], L"send")) {
			action = SEND;
		} else if (!wcscmp(argv[1], L"save")) {
			action = SAVE;
		} else if (!wcscmp(argv[1], L"csv")) {
			action = CSV;
		} else if (!wcscmp(argv[1], L"replay")) {
			action = REPLAY;
		} else if (!wcscmp(argv[1], L"version")) {
			action = VERSION;
		} else if (!wcscmp(argv[1], L"license")) {
			action = LICENSE;
		} else if (!wcscmp(argv[1], L"help")) {
			action = HELP;
		} else {
			action = USAGE;
		}
	}

	argc -= 2;
	argv += 2;

	switch (action) {
	case USAGE:
		return doUsage();
	case HELP:
		return doHelp(argc, argv);
	case SEND:
		return doSend(argc, argv);
	case SAVE:
		return doSave(argc, argv);
	case CSV:
		return doCsv(argc, argv);
	case REPLAY:
		return doReplay(argc, argv);
	case VERSION:
		return doVersion();
	case LICENSE:
		return doLicense();
	}

	return 0;
}
