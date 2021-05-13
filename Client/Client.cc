#include "Client.hh"

#include "SimConnect.h"

jetbridge::Client::Client(void* simconnect) {
  this->simconnect = simconnect;

  SimConnect_AddToClientDataDefinition(simconnect, kPacketDefinition, 0, sizeof(Packet));
  SimConnect_MapClientDataNameToID(simconnect, kPublicDownlinkChannel, kPublicDownlinkArea);
  SimConnect_MapClientDataNameToID(simconnect, kPublicUplinkChannel, kPublicUplinkArea);

  // We'll listen to downlink client data events.
  // It is the user's responsibility to forward these events to us.
  // This should be done by calling handle_received_client_data_event.
  SimConnect_RequestClientData(simconnect, kPublicDownlinkArea, kDownlinkRequest, kPacketDefinition,
                               SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED);
}

void jetbridge::Client::HandleReceivedClientDataEvent(void* event) {
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

jetbridge::Packet* jetbridge::Client::Request(char data[], int timeout) {
  // Prepare the outgoing packet.
  Packet* packet = new Packet(data);

  // Create a promise for the response packet.
  auto promise = new std::promise<Packet*>;
  requests.insert(std::make_pair(packet->id, promise));

  // Transmit the request packet.
  SimConnect_SetClientData(simconnect, kPublicUplinkArea, kPacketDefinition, 0, 0, sizeof(Packet), packet);

  auto future = promise->get_future();
  auto status = future.wait_for(std::chrono::milliseconds(timeout));

  if (status != std::future_status::ready) return nullptr;
  return future.get();
}

void jetbridge::Client::ExecuteCalculatorCode(std::string code) {
  char data[jetbridge::kPacketDataSize] = {};
  data[0] = jetbridge::kExecuteCalculatorCode;
  std::memcpy(data + 1, code.c_str(), sizeof(data) - 1);

  auto response = this->Request(data);
  delete response;
}

double jetbridge::Client::GetNamedVariable(std::string variable_name) {
  char data[jetbridge::kPacketDataSize] = {};
  data[0] = jetbridge::kGetNamedVariable;
  std::memcpy(data + 1, variable_name.c_str(), sizeof(data) - 1);

  auto response = this->Request(data);
  double variable_value;
  // Get the actual value from the response packet.
  std::memcpy(&variable_value, response->data, sizeof(double));

  delete response;
  return variable_value;
}
