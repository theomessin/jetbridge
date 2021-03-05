#include "Client.h"

#include "SimConnect.h"

jetbridge::Client::Client(void* simconnect) {
  this->simconnect = simconnect;

  SimConnect_AddToClientDataDefinition(simconnect, kPacketDefinition, 0, sizeof(Packet));
  SimConnect_MapClientDataNameToID(simconnect, kPublicDownlinkChannel, kPublicDownlinkArea);
  SimConnect_MapClientDataNameToID(simconnect, kPublicUplinkChannel, kPublicDownlinkArea);

  // We'll listen to downlink client data events.
  // It is the user's responsibility to forward these events to us.
  // This should be done by calling handle_received_client_data_event.
  SimConnect_RequestClientData(simconnect, kPublicDownlinkArea, kDownlinkRequest, kPacketDefinition,
                               SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED);
}

void jetbridge::Client::handle_received_client_data_event(void* event) {
  // We're going to actually copy the packet. This allows us to return a pointer instead of copying it around.
  // Of course, it is the user's responsibility to delete this when done.
  auto e = static_cast<SIMCONNECT_RECV_CLIENT_DATA*>(event);
  Packet* packet = new Packet(*(Packet*)&e->dwData);

  // If the packet id is not in requests, delete the packet.
  if (requests.find(packet->id) == requests.end()) {
    delete packet;
    return;
  }

  // We'll fulfill the request promise with the incoming packet.
  requests[packet->id]->set_value(packet);
}

jetbridge::Packet* jetbridge::Client::request(char data[], int timeout) {
  // Prepare the outgoing packet.
  Packet* packet = new Packet(data);

  // Create a promise for the response packet.
  auto promise = new std::promise<Packet*>;
  requests.insert(std::make_pair(packet->id, promise));

  // Transmit the request packet.
  SimConnect_SetClientData(simconnect, kPublicUplinkArea, kPacketDefinition, 0, 0, sizeof(Packet), packet);

  auto future = promise->get_future();
  auto status = future.wait_for(std::chrono::milliseconds(timeout));

  if (status != std::future_status::ready) return 0;
  return future.get();
}
