typedef int AC_STATUS;

#define AC_OFF 0
#define AC_REPLAY 1
#define AC_LIVE 2
#define AC_PAUSE 3

typedef int AC_SESSION_TYPE;

#define AC_UNKNOWN -1
#define AC_PRACTICE 0
#define AC_QUALIFY 1
#define AC_RACE 2
#define AC_HOTLAP 3
#define AC_TIME_ATTACK 4
#define AC_DRIFT 5
#define AC_DRAG 6
#define AC_HOTSTINT 7
#define AC_HOTLAPSUPERPOLE 8

typedef int AC_FLAG_TYPE;

#define AC_NO_FLAG 0
#define AC_BLUE_FLAG 1
#define AC_YELLOW_FLAG 2
#define AC_BLACK_FLAG 3
#define AC_WHITE_FLAG 4
#define AC_CHECKERED_FLAG 5
#define AC_PENALTY_FLAG 6


#pragma pack(push)
#pragma pack(4)

struct ACCPhysics
{
	int packetId;
	float gas;
	float brake;
	float fuel;
	int gear;
	int rpms;
	float steerAngle;
	float speedKmh;
	float velocity[3];
	float accG[3];
	float wheelSlip[4];
	float wheelLoad[4];
	float wheelsPressure[4];
	float wheelAngularSpeed[4];
	float tyreWear[4];
	float tyreDirtyLevel[4];
	float tyreCoreTemperature[4];
	float camberRAD[4];
	float suspensionTravel[4];
	float drs;
	float tc;
	float heading;
	float pitch;
	float roll;
	float cgHeight;
	float carDamage[5];
	int numberOfTyresOut;
	int pitLimiterOn;
	float abs;
	float kersCharge;
	float kersInput;
	int autoShifterOn;
	float rideHeight[2];
	float turboBoost;
	float ballast;
	float airDensity;
	float airTemp;
	float roadTemp;
	float localAngularVel[3];
	float finalFF;
	float performanceMeter;
	int engineBrake;
	int ersRecoveryLevel;
	int ersPowerLevel;
	int ersHeatCharging;
	int ersIsCharging;
	float kersCurrentKJ;
	int drsAvailable;
	int drsEnabled;
	float brakeTemp[4];
	float clutch;
	float tyreTempI[4];
	float tyreTempM[4];
	float tyreTempO[4];
	int isAIControlled;
	float tyreContactPoint[4][3];
	float tyreContactNormal[4][3];
	float tyreContactHeading[4][3];
	float brakeBias;
	float localVelocity[3];
	int P2PActivations;
	int P2PStatus;
	int currentMaxRpm;
	float mz[4];
	float fx[4];
	float fy[4];
	float slipRatio[4];
	float slipAngle[4];
	int tcinAction;
	int absInAction;
	float suspensionDamage[4];
	float tyreTemp[4];
	float waterTemp;
	float brakePressure[4];
};


struct ACCGraphics
{
	int packetId;
	AC_STATUS status;
	AC_SESSION_TYPE session;
	wchar_t currentTime[15];
	wchar_t lastTime[15];
	wchar_t bestTime[15];
	wchar_t split[15];
	int completedLaps;
	int position;
	int iCurrentTime;
	int iLastTime;
	int iBestTime;
	float sessionTimeLeft;
	float distanceTraveled;
	int isInPit;
	int currentSectorIndex;
	int lastSectorTime;
	int numberOfLaps;
	wchar_t tyreCompound[33];
	float replayTimeMultiplier;
	float normalizedCarPosition;
	int activeCars;
	float carCoordinates[60][3];
	int carID[60];
	int playerCarID;
	float penaltyTime;
	AC_FLAG_TYPE flag;
	int penalty;
	int idealLineOn;
	int isInPitLane;
	float surfaceGrip;
	int mandatoryPitDone;
	float windSpeed;
	float windDirection;
	int isSetupMenuVisible;
	int mainDisplayIndex;
	int secondaryDisplayIndex;
	int TC;
	int TCCut;
	int EngineMap;
	int ABS;
	int fuelXLap;
	int rainLights;
	int flashingLights;
	int lightsStage;
	float exhaustTemperature;
	int wiperLV;
	int DriverStintTotalTimeLeft;
	int DriverStintTimeLeft;
	int rainTyres;
	int sessionIndex;
	float usedFuel;
	wchar_t deltaLapTime[15];
	int iEstimatedLapTime;
	int isDeltaPositive;
	int iSplit;
	int isValidLap;
	float fuelEstimatedLaps;
	wchar_t trackStatus[33];
	int missingMandatoryPits;
};


struct ACCStatic
{
	wchar_t smVersion[15];
	wchar_t acVersion[15];
	int numberOfSessions;
	int numCars;
	wchar_t carModel[33];
	wchar_t track[33];
	wchar_t playerName[33];
	wchar_t playerSurname[33];
	wchar_t playerNick[33];
	int sectorCount;
	float maxTorque;
	float maxPower;
	int	maxRpm;
	float maxFuel;
	float suspensionMaxTravel[4];
	float tyreRadius[4];
	float maxTurboBoost;
	float deprecated_1;
	float deprecated_2;
	int penaltiesEnabled;
	float aidFuelRate;
	float aidTireRate;
	float aidMechanicalDamage;
	int aidAllowTyreBlankets;
	float aidStability;
	int aidAutoClutch;
	int aidAutoBlip;
	int hasDRS;
	int hasERS;
	int hasKERS;
	float kersMaxJ;
	int engineBrakeSettingsCount;
	int ersPowerControllerCount;
	float trackSPlineLength;
	wchar_t trackConfiguration[33];
	float ersMaxJ;
	int isTimedRace;
	int hasExtraLap;
	wchar_t carSkin[33];
	int reversedGridPositions;
	int PitWindowStart;
	int PitWindowEnd;
	int isOnline;
};


#pragma pack(pop)
