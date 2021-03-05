#include "Module.hh"

// Microsoft Flight Simulator includes.
#include <MSFS/Legacy/gauges.h>
#include <MSFS/MSFS.h>
#include <MSFS/MSFS_WindowsTypes.h>
#include <SimConnect.h>

// Standard Template Library includes.
#include <iostream>
#include <string>

// Library defines common things e.g. Packet.
#include "../Protocol/Protocol.hh"

HANDLE simconnect = 0;

void CALLBACK HandleSimconnectMessage(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
  switch (pData->dwID) {
    case SIMCONNECT_RECV_ID_CLIENT_DATA: {
      auto received_data = static_cast<SIMCONNECT_RECV_CLIENT_DATA*>(pData);
      jetbridge::Packet* packet = (jetbridge::Packet*)&received_data->dwData;

      // TODO: better opcode decoding.
      if (packet->data[0] != 'x') return;

      std::string code = (char*)(packet->data + 1);
      std::cout << "[Jetbridge] [" << packet->id << ":x] " << code << std::endl;
      execute_calculator_code(code.c_str(), 0, 0, 0);

      // Reply with the same exact same packet (for now).
      SimConnect_SetClientData(simconnect, jetbridge::kPublicDownlinkArea, jetbridge::kPacketDefinition, 0, 0,
                               sizeof(jetbridge::Packet), packet);
      break;
    }
  }
}

extern "C" MSFS_CALLBACK void module_init() {
  // This will (hopefully) be visibile in the Microsoft Flight Simulator console.
  std::cout << "[Jetbridge] Module version " << kModuleVersion << " initialising." << std::endl;

  // TODO: implement simconnect error handling.
  SimConnect_Open(&simconnect, kSimconnectClientName, 0, 0, 0, 0);

  // Define a custom ClientDataDefinition for jetbridge::Packet.
  SimConnect_AddToClientDataDefinition(simconnect, jetbridge::kPacketDefinition, 0, sizeof(jetbridge::Packet));

  // Map the public downlink and uplink channels with own ids for them (see CleitnDataAreas enum).
  SimConnect_MapClientDataNameToID(simconnect, jetbridge::kPublicDownlinkChannel, jetbridge::kPublicDownlinkArea);
  SimConnect_MapClientDataNameToID(simconnect, jetbridge::kPublicUplinkChannel, jetbridge::kPublicUplinkArea);

  // Create the public downlink and uplink channels (which are actually ClientData areas).
  SimConnect_CreateClientData(simconnect, jetbridge::kPublicDownlinkArea, sizeof(jetbridge::Packet), 0);
  SimConnect_CreateClientData(simconnect, jetbridge::kPublicUplinkArea, sizeof(jetbridge::Packet), 0);

  // Request to be notified (via Simconnect Dispatch) for any changes to the public uplink channel.
  SimConnect_RequestClientData(simconnect, jetbridge::kPublicUplinkArea, jetbridge::kUplinkRequest,
                               jetbridge::kPacketDefinition, SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET,
                               SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED);

  SimConnect_CallDispatch(simconnect, HandleSimconnectMessage, 0);
}

extern "C" MSFS_CALLBACK void module_deinit() {
  if (!simconnect) return;
  SimConnect_Close(simconnect);
}
