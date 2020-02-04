/* Requires uint8_t and uint16_t, found in stdint.h */

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

#define SDP_STATUS_OFF 0
#define SDP_STATUS_REPLAY 1
#define SDP_STATUS_LIVE 2
#define SDP_STATUS_PAUSE 3
