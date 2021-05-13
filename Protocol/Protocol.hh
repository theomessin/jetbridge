#pragma once

namespace jetbridge {

static const int kPacketDataSize = 128;
static const char* kPublicDownlinkChannel = "theomessin.jetbridge.downlink";
static const char* kPublicUplinkChannel = "theomessin.jetbridge.uplink";

class Packet {
 private:
  // HACK: static offset avoids collisions.
  inline static unsigned int offset = 0;

 public:
  int id = 0;
  char data[kPacketDataSize] = {};

  Packet(){};
  Packet(char data[]);
};

enum ClientDataDefinition {
  kPacketDefinition = 5321,
};

enum ClientDataArea {
  kPublicDownlinkArea = 5321,
  kPublicUplinkArea = 5322,
};

enum DataRequest {
  kUplinkRequest = 5321,
  kDownlinkRequest = 5322,
};

enum Opcode {
  kExecuteCalculatorCode = 0,
  kGetNamedVariable = 1,
};

}  // namespace jetbridge
