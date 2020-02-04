/* Requires uint8_t and uint16_t, found in stdint.h */

#pragma pack (push)
#pragma pack (1)

struct SimDisplayPacket {
	uint8_t status;
	uint16_t rpm;
	uint16_t maxRpm;
	uint8_t pitLimiterOn;
	uint8_t gear;
	uint8_t tc;
	uint8_t tcc;
	uint8_t tcInAction;
	uint8_t abs;
	uint8_t absInAction;
	uint8_t bb;
	uint8_t fuelEstimatedLaps;
	uint8_t engineMap;
	uint8_t airTemp;
	uint8_t roadTemp;
};

#pragma pack (pop)

#define SDP_STATUS_OFF	0
#define SDP_STATUS_LIVE	1
