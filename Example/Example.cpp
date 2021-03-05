#include <windows.h>

#include <iostream>
#include <thread>

#include "../Client/Client.h"
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
      if (e->dwRequestID == jetbridge::kDownlinkRequest) client->handle_received_client_data_event(e);
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
  SimConnect_Open(&simconnect, "Flyover Consumer", 0, 0, 0, 0);

  // We'll use our lovely flyover sdk client.
  client = new jetbridge::Client(simconnect);

  // We'll use this thread to loop SimConnect_CallDispatch.
  std::thread loopThread(loop);

  while (!QUIT_SIGNAL_RECEIVED) {
    char code[64] = {};
    code[0] = 'x';
    std::cin.getline(code + 1, sizeof(code) - 1);
    auto response = client->request(code);
    delete response;  // response is a Packet* - be sure to delete when no longer needed.
  }

  // We'll block until the loop thread exits too.
  loopThread.join();

  // If we've reached this point, we should close simconnect and exit.
  SimConnect_Close(simconnect);
  return 0;
}
