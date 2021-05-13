#include <windows.h>

#include <iostream>
#include <thread>

#include "../Client/Client.hh"
#include "SimConnect.h"

HANDLE simconnect = 0;
jetbridge::Client* client = 0;
bool QUIT_SIGNAL_RECEIVED = false;

void CALLBACK MyDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
  switch (pData->dwID) {
    case SIMCONNECT_RECV_ID_QUIT: {
      QUIT_SIGNAL_RECEIVED = true;
      break;
    }

    case SIMCONNECT_RECV_ID_CLIENT_DATA: {
      auto e = static_cast<SIMCONNECT_RECV_CLIENT_DATA*>(pData);
      if (e->dwRequestID == jetbridge::kDownlinkRequest) client->HandleReceivedClientDataEvent(e);
      break;
    }
  }
}

void loop() {
  for (; !QUIT_SIGNAL_RECEIVED; Sleep(1)) {
    SimConnect_CallDispatch(simconnect, MyDispatchProc, 0);
  }
}

int main() {
  SimConnect_Open(&simconnect, "Jetbridge Consumer Example", 0, 0, 0, 0);

  // We'll use our lovely flyover sdk client.
  client = new jetbridge::Client(simconnect);

  // We'll use this thread to loop SimConnect_CallDispatch.
  std::thread loopThread(loop);

  while (!QUIT_SIGNAL_RECEIVED) {
    std::string verb, args;
    std::cin >> verb;
    std::getline(std::cin, args);
    // Remove the extra space.
    args.erase(0, 1);

    if (verb == "exe") {
      client->ExecuteCalculatorCode(args);
      std::cout << args << std::endl;
    } else if (verb == "get") {
      double value = client->GetNamedVariable(args);
      std::cout << value << std::endl;
    }
  }

  // We'll block until the loop thread exits too.
  loopThread.join();

  // If we've reached this point, we should close simconnect and exit.
  SimConnect_Close(simconnect);
  return 0;
}
