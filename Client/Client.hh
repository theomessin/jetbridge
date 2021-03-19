#pragma once
#include <Windows.h>

#include <future>
#include <map>

#include "../Protocol/Protocol.hh"

namespace jetbridge {

enum CalculatorCodeResultType {
  kFloat,    // double
  kInteger,  // int
  kString,   // char*
};

class Client {
 private:
  void* simconnect = 0;
  std::map<int, std::promise<Packet*>*> requests;

 public:
  Client(void* simconnect);
  void HandleReceivedClientDataEvent(void* event);
  Packet* Request(char data[], int timeout = 1000);

  void ExecuteCalculatorCode(std::string code);
  void ExecuteCalculatorCode(std::string code, double* result);
};

}  // namespace jetbridge
