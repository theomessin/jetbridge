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
  void HandleReceivedClientDataEvent(void* event);
  Packet* Request(char data[], int timeout = 1000);

  void ExecuteCalculatorCode(std::string code);
  double GetNamedVariable(std::string variable_name);
};

}  // namespace jetbridge
