/*
simdisplay - A simracing dashboard created using Arduino to show shared memory telemetry from Assetto Corsa Competizione.

Copyright (C) 2021  Filippo Erik Negroni

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

#define ACCSHAREDMEMORY_VERSION 0x0107
#define ACCSHAREDMEMORY_VERSION_STRING "1.7"

#define ACC_STATUS_OFF 0
#define ACC_STATUS_REPLAY 1
#define ACC_STATUS_LIVE 2
#define ACC_STATUS_PAUSE 3

#define ACC_SESSION_UNKNOWN -1
#define ACC_SESSION_PRACTICE 0
#define ACC_SESSION_QUALIFY 1
#define ACC_SESSION_RACE 2
#define ACC_SESSION_HOTLAP 3
#define ACC_SESSION_TIME_ATTACK 4
#define ACC_SESSION_DRIFT 5
#define ACC_SESSION_DRAG 6
#define ACC_SESSION_HOTSTINT 7
#define ACC_SESSION_HOTSTINT_SUPERPOLE 8

#define ACC_PENALTY_NONE 0
#define ACC_PENALTY_CUT_DT 1
#define ACC_PENALTY_CUT_SG_10 2
#define ACC_PENALTY_CUT_SG_20 3
#define ACC_PENALTY_CUT_SG_30 4
#define ACC_PENALTY_CUT_DSQ 5
#define ACC_PENALTY_CUT_REMOVEBESTLAP 6
#define ACC_PENALTY_PITSPEEDING_DT 7
#define ACC_PENALTY_PITSPEEDING_SG_10 8
#define ACC_PENALTY_PITSPEEDING_SG_20 9
#define ACC_PENALTY_PITSPEEDING_SG_30 10
#define ACC_PENALTY_PITSPEEDING_DSQ 11
#define ACC_PENALTY_PITSPEEDING_REMOVEBESTLAP 12
#define ACC_PENALTY_IGNOREMANDATORYPIT_DSQ 13
#define ACC_PENALTY_POSTRACETIME 14
#define ACC_PENALTY_DSQ_TROLL 15
#define ACC_PENALTY_PITENTRY 16
#define ACC_PENALTY_PITEXIT 17
#define ACC_PENALTY_WRONGWAY 18
#define ACC_PENALTY_IGNORESTINT_DT 19
#define ACC_PENALTY_IGNORESTINT_DSQ 20
#define ACC_PENALTY_EXCEEDSTINT_DSQ 21

#define ACC_FLAG_NO 0
#define ACC_FLAG_BLUE 1
#define ACC_FLAG_YELLOW 2
#define ACC_FLAG_BLACK 3
#define ACC_FLAG_WHITE 4
#define ACC_FLAG_CHECKERED 5
#define ACC_FLAG_PENALTY 6
#define ACC_FLAG_GREEN 7
#define ACC_FLAG_ORANGE 8

#define ACC_TYRE_FL 0
#define ACC_TYRE_FR 1
#define ACC_TYRE_RL 2
#define ACC_TYRE_RR 3

#define ACC_COORD_X 0
#define ACC_COORD_Y 1
#define ACC_COORD_Z 2

#define ACC_DAMAGE_FRONT 0
#define ACC_DAMAGE_REAR 1
#define ACC_DAMAGE_LEFT 2
#define ACC_DAMAGE_RIGHT 3
#define ACC_DAMAGE_CENTRE 4

#pragma pack(push)
#pragma pack(4)

struct ACCPhysics
{
	int packetId;
	float gas; // Accelerator pedal input (0.0 to 1.0)
	float brake; // Brake pedal input (0.0 to 1.0)
	float fuel; // Amount of fuel remaining in kg
	int gear; // Current gear 0=R, 1=N, 2=1, ...
	int rpms;
	float steerAngle; // Steering wheel input (-1.0 to 1.0)
	float speedKph;
	float velocity[3]; // Velocity vector in global coordinates
	float accG[3]; // Acceleration vector in global coordinates
	float wheelSlip[4];
	float _wheelLoad[4]; // Not used in ACC
	float wheelsPressure[4]; // unit?
	float wheelAngularSpeed[4]; // in rad/s
	float _tyreWear[4]; // Not used in ACC
	float _tyreDirtyLevel[4]; // Not used in ACC
	float tyreCoreTemperature[4]; // unit?
	float _camberRAD[4]; // Not used in ACC
	float suspensionTravel[4]; // unit?
	float _drs; // Not used in ACC
	float tc; // Traction Control in action
	float heading; // unit?
	float pitch; // unit?
	float roll; // unit?
	float _cgHeight; // Not used in ACC
	float carDamage[5]; // 0: front, 1: rear, 2: left, 3: right, 4: centre
	int _numberOfTyresOut; // Not used in ACC
	int pitLimiterOn;
	float abs; // ABS in action
	float _kersCharge; // Not used in ACC
	float _kersInput; // Not used in ACC
	int autoShifterOn;
	float _rideHeight[2]; // Not used in ACC
	float turboBoost; // unit?
	float _ballast; // Not used in ACC
	float _airDensity; // Not used in ACC
	float airTemp; // unit?
	float roadTemp; // unit?
	float localAngularVel[3]; // Angular velocity vector in local coordinates
	float finalFF; // Force Feedback signal (value range?)
	float _performanceMeter; // Not used in ACC
	int _engineBrake; // Not used in ACC
	int _ersRecoveryLevel; // Not used in ACC
	int _ersPowerLevel; // Not used in ACC
	int _ersHeatCharging; // Not used in ACC
	int _ersIsCharging; // Not used in ACC
	float _kersCurrentKJ; // Not used in ACC
	int _drsAvailable; // Not used in ACC
	int _drsEnabled; // Not used in ACC
	float brakeTemp[4]; // (unit?)
	float clutch; // Clutch pedal input (0.0 to 1.0)
	float _tyreTempI[4]; // Not used in ACC
	float _tyreTempM[4]; // Not used in ACC
	float _tyreTempO[4]; // Not used in ACC
	int isAIControlled;
	float tyreContactPoint[4][3]; // Global coordinates
	float tyreContactNormal[4][3]; // ?
	float tyreContactHeading[4][3]; // ?
	float brakeBias; // Adjust by car model specific offset (see ACC_CarModelDict)
	float localVelocity[3]; // Velocity vector in local coordinates
	int _P2PActivations; // Not used in ACC
	int _P2PStatus; // Not used in ACC
	int _currentMaxRpm; // Not used in ACC
	float _mz[4]; // Not used in ACC
	float _fx[4]; // Not used in ACC
	float _fy[4]; // Not used in ACC
	float slipRatio[4]; // in radians
	float slipAngle[4];
	int _tcInAction; // Not used in ACC
	int _absInAction; // Not used in ACC
	float _suspensionDamage[4]; // Not used in ACC
	float _tyreTemp[4]; // Not used in ACC
	float waterTemp; // unit?
	float brakePressure[4]; // Adjust by car model specific offset (not yet in ACC_CarModelDict)
	int frontBrakeCompound;
	int rearBrakeCompound;
	float padLife[4];
	float discLife[4];
	int ignitionOn;
	int starterEngineOn;
	int isEngineRunning;
	float kerbVibration; // Force Feedback
	float slipVibration; // Force Feedback
	float gVibration; // Force Feedback
	float absVibration; // Force Feedback
};

struct ACCGraphics
{
	int packetId;
	int status; // See ACC_STATUS_*
	int session; // See ACC_SESSION_*
	wchar_t currentTime[15];
	wchar_t lastTime[15];
	wchar_t bestTime[15];
	wchar_t split[15];
	int completedLaps;
	int position;
	int iCurrentTime; // in ms
	int iLastTime; // in ms
	int iBestTime; // in ms
	float sessionTimeLeft; // unit?
	float distanceTraveled; // unit?
	int isInPit;
	int currentSectorIndex;
	int lastSectorTime; // in ms
	int numberOfLaps; // same as completedLaps ?
	wchar_t tyreCompound[33];
	float _replayTimeMultiplier; // Not used in ACC
	float normalizedCarPosition; // 0.0: start, 1.0: finish
	int activeCars;
	float carCoordinates[60][3];
	int carID[60]; // See ACC_CarModelDict
	int playerCarID; // See ACC_CarModelDict
	float penaltyTime; // unit?
	int flag; // See ACC_FLAG_*
	int penalty; // See ACC_PENALTY_* 
	int idealLineOn;
	int isInPitLane;
	float surfaceGrip; // Ideal line friction coefficient
	int mandatoryPitDone;
	float windSpeed; // in m/s
	float windDirection; // in radians
	int isSetupMenuVisible;
	int mainDisplayIndex;
	int secondaryDisplayIndex;
	int TC; // TC level
	int TCCut; // TC Cut level
	int EngineMap;
	int ABS; // ABS level
	int fuelXLap; // litres per lap
	int rainLights; // Rain light on
	int flashingLights; // flash on
	int lightsStage;
	float exhaustTemperature;
	int wiperLV; // Wipers stage level
	int DriverStintTotalTimeLeft; // in ms
	int DriverStintTimeLeft; // in ms
	int rainTyres; // Rain tyres equipped
	int sessionIndex; // ?
	float usedFuel; // Used fuel since refuel (unit?)
	wchar_t deltaLapTime[15];
	int iDeltaLapTime; // in ms
	wchar_t estimatedLapTime[15];
	int iEstimatedLapTime; // in ms
	int isDeltaPositive;
	int iSplit; // in ms
	int isValidLap;
	float fuelEstimatedLaps;
	wchar_t trackStatus[33];
	int missingMandatoryPits; // Remaining mandatory pitstops
	float Clock; // in Seconds
	int directionLightsLeft; // left indicators on
	int directionLightsRight; // right indicators on
	int GlobalYellow; // FCY?
	int GlobalYellow1; // Yellow flag sector 1
	int GlobalYellow2; // Yellow flag sector 2
	int GlobalYellow3; // Yellow flag sector 3
	int GlobalWhite; // White flag
	int GlobalGreen; // Green flag
	int GlobalChequered; // Chequered flag
	int GlobalRed; // Red flag
	int mfdTyreSet; // as shown on MFD
	float mfdFuelToAdd; // as shown on MFD
	float mfdTyrePressure[4]; // as shown on MFD, use ACC_TYRE_*
};


struct ACCStatic
{
	wchar_t smVersion[15];
	wchar_t acVersion[15];
	int numberOfSessions;
	int numCars;
	wchar_t carModel[33]; // Use with ACC_CarModelDict
	wchar_t track[33];
	wchar_t playerName[33];
	wchar_t playerSurname[33];
	wchar_t playerNick[33];
	int sectorCount; // Sectors on this track
	float _maxTorque; // Not used in ACC
	float _maxPower; // Not used in ACC
	int maxRpm;
	float maxFuel; // Fuel tank capacity
	float _suspensionMaxTravel[4]; // Not used in ACC
	float _tyreRadius[4]; // Not used in ACC
	float _maxTurboBoost; // Not used in ACC
	float _deprecated_1; // Not used in ACC
	float _deprecated_2; // Not used in ACC
	int penaltiesEnabled;
	float aidFuelRate; // %
	float aidTireRate; // %
	float aidMechanicalDamage; // %
	float AllowTyreBlankets; // Is this int or float?
	float aidStability; // Is this int or float? %?
	int aidAutoClutch;
	int aidAutoBlip;
	int _hasDRS; // Not used in ACC
	int _hasERS; // Not used in ACC
	int _hasKERS; // Not used in ACC
	float _kersMaxJ; // Not used in ACC
	int _engineBrakeSettingsCount; // Not used in ACC
	int _ersPowerControllerCount; // Not used in ACC
	float _trackSPlineLength; // Not used in ACC
	wchar_t _trackConfiguration[33]; // Not used in ACC
	float _ersMaxJ; // Not used in ACC
	int _isTimedRace; // Not used in ACC
	int _hasExtraLap; // Not used in ACC
	wchar_t _carSkin[33]; // Not used in ACC
	int _reversedGridPositions; // Not used in ACC
	int PitWindowStart; // In seconds?
	int PitWindowEnd; // In seconds?
	int isOnline;
	wchar_t dryTyresName[33];
	wchar_t wetTyresName[33];
};

#pragma pack(pop)