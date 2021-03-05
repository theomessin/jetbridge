Jetbridge is a simple WebAssembly (WASM) module that allows out-of-process SimConnect modules to act as if they were in-process ones. Specifically, the current development version allows for external modules to dispatch `execute_calculator_code` calls which can be used to read/write `L:` simvars, or even dispatch `H:` events.

#### How does this even work?

Jetbridge establishes a communication protocol using SimConnect's Client Data Areas. It then defines some basic operations that external clients can use, such as calling `execute_calculator_code`. Jetbridge ships with a C++ SDK which defines a Client class which allows the consumer to communcate with Jetbridge without having to implement the underlying protocol. SDK's for other language _may_ eventually be provided (e.g. C#).

#### M’kay… But will anyone actually use this?

Jetbridge is currently being used by myself, as well as other [FlyByWire Simulations](https://flybywiresim.com/) developers, to code hardware drivers for the [A32NX](https://github.com/flybywiresim/a32nx). It allows for external programs (such as a hardware driver) to access the inner workings of MSFS in ways not otherwise possible through pure SimConnect.


#### What are all these folders?

Currently, the repo is structured as a single Visual Studio solution, with 4 projects. The `Protocol` project is a C++ static library that defines core protocol entities (e.g. Packet). The `Module` project is the core WASM module and it depends on `Protocol`. The `Client` project is another C++ static library that defines the consumer-facing SDK (i.e. the Client) - it also depends on `Protocol`. Finally, the `Example` project is a simple C++ command line interface executable that shows how Jetbridge can be used by other developers - it of course depends on `Client`, and requires `Module` to be loaded in the simulator.

```
Client/
├─ Client.vcxproj
Example/
├─ Example.vcxproj
Module/
├─ Module.vcxproj
Protocol/
├─ Protocol.vcxproj
jetbridge.sln
```

#### How to use the example

You must first install the Module wasm artifact. You'll have to create a Microsoft Flight Simulator package to do this. If this has been done correctly, you should be able to see an `initialising` message in the MSFS console. Then, simply start the Example.exe artifact. You can then use the CLI to send [Reverse Polish Notation](https://docs.flightsimulator.com/html/Additional_Information/Reverse_Polish_Notation.htm) code which will be executed using `execute_calculator_code`. This should look something like this:

![2021-03-05-05-29-01](https://user-images.githubusercontent.com/7229472/110072171-478cfd80-7d75-11eb-859f-200f31bc6c6e.gif)

#### How to use the C++ Client software development kit

Make sure you've correctly imported the static library (header files need to be accessible, .lib file needs to be available to the Linker).
Create a Client instance by passing a SimConnect `HANDLE`:

```c++
client = new jetbridge::Client(simconnect);
```

Then you can send some data using `Client->request`:

```c++
auto response = client->request((char*)"x2 (>L:A320_Neo_MFD_NAV_MODE_1)");
```

This example RPN will set (`x` opchar) the `A320_Neo_MFD_NAV_MODE_1` LocalVar to `2`.
`Client->request` blocks until a response from Jetbridge has been received, or the timeout is reached.
If a response is received, it will return a Packet pointer.
**Important**: don't forget to delete this when done using it to avoid a memory leak.

```c++
delete response;
```

**Important**: You should call the `client->handle_received_client_data_event` function with received data for all `SIMCONNECT_RECV_ID_CLIENT_DATA` events where the RequestID matches `jetbridge::kDownlinkRequest`. Basically, add something along these lines to your `MyDispatchProc`:

```c++
case SIMCONNECT_RECV_ID_CLIENT_DATA: {
  auto e = static_cast<SIMCONNECT_RECV_CLIENT_DATA*>(pData);
  if (e->dwRequestID == jetbridge::kDownlinkRequest) client->handle_received_client_data_event(e);
  break;
```

---

Jetbridge is currently at a very early stage of development so bear with us whilst we get everything sorted 🧸
