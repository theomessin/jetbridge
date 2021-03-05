#include "Protocol.h"

#include <cstdlib>
#include <cstring>
#include <ctime>

jetbridge::Packet::Packet(char data[]) {
  // TODO: better random id generation.
  srand(time(0));
  this->id = rand();

  // Copy the passed data to the packet data
  std::memcpy(this->data, data, sizeof(this->data));
}
