using System;
using Microsoft.FlightSimulator.SimConnect;
using TheoMessin.JetBridge.Client;

namespace TheoMessin.JetBridge.Example
{
	/// <summary>
	/// Example client that sends commands to JetBridge and displays the response
	/// </summary>
	class ExampleClient : IDisposable
	{
		#region Fields

		/// <summary>
		/// JetBridge client
		/// </summary>
		readonly JetBridgeClient _client;

		/// <summary>
		/// SimConnect client
		/// </summary>
		readonly SimConnect _simConnect;

		#endregion

		#region Constructors / Finalizers

		/// <summary>
		/// Create a new client
		/// </summary>
		/// <param name="handle">Handle of the window to process messages with</param>
		public ExampleClient(IntPtr handle)
		{
			_simConnect = new SimConnect("JetBridge C# Client", handle, SimConnectRunner.SimConnectMessageId, null, 0);
			_client = new JetBridgeClient(_simConnect);
			_simConnect.OnRecvException += OnRecvException;
			_simConnect.OnRecvOpen += OnRecvOpen;
		}

		#endregion

		#region Methods

		/// <summary>
		/// Disconnect the SimConnect client
		/// </summary>
		public void Dispose()
		{
			_simConnect?.Dispose();
		}


		/// <summary>
		/// Call <see cref="SimConnect.ReceiveMessage" /> because of window message
		/// </summary>
		public void ReceiveMessage()
		{
			_simConnect.ReceiveMessage();
		}

		/// <summary>
		/// Send a command to JetBridge
		/// </summary>
		/// <param name="command">Command to send to JetBridge</param>
		/// <returns>The response from JetBridge, or <c>null</c> if no response was received</returns>
		public string SendCommand(string command)
		{
			return _client.SendCommand("x" + command);
		}

		/// <summary>
		/// Handle Exceptions
		/// </summary>
		/// <param name="sender">SimConnect Object</param>
		/// <param name="data">Exception data</param>
		void OnRecvException(SimConnect sender, SIMCONNECT_RECV_EXCEPTION data)
		{
			Console.WriteLine("Exception: {0}", (SIMCONNECT_EXCEPTION)data.dwException);
		}

		/// <summary>
		/// Handle a connection to the simulator
		/// </summary>
		/// <param name="sender">SimConnect Object</param>
		/// <param name="data">Simulator info</param>
		void OnRecvOpen(SimConnect sender, SIMCONNECT_RECV_OPEN data)
		{
			Console.WriteLine($"Connected to {data.szApplicationName}");
			_client.Connect();
		}

		#endregion
	}
}