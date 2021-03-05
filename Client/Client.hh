#pragma once

#include <Windows.h>

#include <future>
#include <map>

#include "../Protocol/Protocol.hh"

namespace jetbridge {

class Client {
 private:
  void* simconnect = 0;
  std::map<int, std::promise<Packet*>*> requests;

 public:
  Client(void* simconnect);
  void handle_received_client_data_event(void* event);
  Packet* request(char data[], int timeout = 1000);
};

}  // namespace jetbridge
