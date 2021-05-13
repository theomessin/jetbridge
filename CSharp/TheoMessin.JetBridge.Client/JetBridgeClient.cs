using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;
using Microsoft.FlightSimulator.SimConnect;

namespace TheoMessin.JetBridge.Client
{
    /// <summary>
    /// Client for JetBridge
    /// </summary>
    public class JetBridgeClient
    {
        #region Fields

        /// <summary>
        /// Generate random IDs
        /// </summary>
        private readonly Random _idGenerator = new Random();

        /// <summary>
        /// Map of request id to response
        /// </summary>
        private readonly Dictionary<int, PacketResponse> _responses = new Dictionary<int, PacketResponse>();

        /// <summary>
        /// SimConnect instance
        /// </summary>
        private readonly SimConnect _simConnect;

        #endregion

        #region Constructors / Finalizers

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="simConnect">SimConnect instance to communicate with</param>
        public JetBridgeClient(SimConnect simConnect)
        {
            _simConnect = simConnect;
        }

        #endregion

        #region Enums

        /// <summary>
        /// Client data areas
        /// </summary>
        private enum ClientDataAreas
        {
            /// <summary>
            /// Data returned from JetBridge
            /// </summary>
            PublicDownlinkArea = 5321,

            /// <summary>
            /// Data sent to JetBridge
            /// </summary>
            PublicUplinkArea = 5322,
        };

        /// <summary>
        /// Data definitions
        /// </summary>
        private enum ClientDataDefinitions
        {
            /// <summary>
            /// <see cref="Packet" /> data type
            /// </summary>
            PacketDefinition = 5321,
        };

        /// <summary>
        /// Request identifiers
        /// </summary>
        private enum DataRequest
        {
            /// <summary>
            /// Not currently used
            /// </summary>
            // ReSharper disable once UnusedMember.Local
            UplinkRequest = 5321,

            /// <summary>
            /// Return data from JetBridge
            /// </summary>
            DownlinkRequest = 5322,
        };

        #endregion

        #region Methods

        /// <summary>
        /// Connect to SimConnect
        /// </summary>
        /// <remarks>
        /// Call this after SimConnect has connected to the simulator
        /// </remarks>
        public void Connect()
        {
            _simConnect.OnRecvClientData += OnRecvClientData;
            _simConnect.AddToClientDataDefinition(ClientDataDefinitions.PacketDefinition, 0, (uint)Marshal.SizeOf<Packet>(), 0, 0);
            _simConnect.RegisterStruct<SIMCONNECT_RECV_CLIENT_DATA, Packet>(ClientDataDefinitions.PacketDefinition);
            _simConnect.MapClientDataNameToID(ChannelNames.PublicDownlinkChannel, ClientDataAreas.PublicDownlinkArea);
            _simConnect.MapClientDataNameToID(ChannelNames.PublicUplinkChannel, ClientDataAreas.PublicUplinkArea);
            _simConnect.RequestClientData(ClientDataAreas.PublicDownlinkArea, DataRequest.DownlinkRequest, ClientDataDefinitions.PacketDefinition, SIMCONNECT_CLIENT_DATA_PERIOD.ON_SET, SIMCONNECT_CLIENT_DATA_REQUEST_FLAG.CHANGED, 0, 0, 0);
        }


        /// <summary>
        /// Send a command to JetBridge
        /// </summary>
        /// <param name="command">Command to send</param>
        /// <returns>Response to the command, or <c>null</c> if no response was received</returns>
        public string Request(string command)
        {
            return Request(command, TimeSpan.FromSeconds(1));
        }

        /// <summary>
        /// Send a command to JetBridge
        /// </summary>
        /// <param name="command">Command to send</param>
        /// <param name="timeout">Time to wait for a response</param>
        /// <returns>Response to the command, or <c>null</c> if no response was received</returns>
        public string Request(string command, TimeSpan timeout)
        {
            var p = new Packet { id = _idGenerator.Next(), data = command };

            using (var response = new PacketResponse())
            {
                _responses.Add(p.id, response);
                _simConnect.SetClientData(ClientDataAreas.PublicUplinkArea, ClientDataDefinitions.PacketDefinition, 0, 0, p);
                var received = response.Received.WaitOne(timeout);
                _responses.Remove(p.id);
                return received ? response.Response : null;
            }
        }

        /// <summary>
        /// Handle received data
        /// </summary>
        /// <param name="sender">SimConnect client</param>
        /// <param name="data">Received data</param>
        private void OnRecvClientData(SimConnect sender, SIMCONNECT_RECV_CLIENT_DATA data)
        {
            if (data.dwDefineID == (uint)ClientDataDefinitions.PacketDefinition)
            {
                if (data.dwData[0] is Packet p)
                {
                    if (_responses.TryGetValue(p.id, out var response))
                    {
                        response.Response = p.data;
                    }
                }
            }
        }

        #endregion

        #region Structs

        /// <summary>
        /// Wire format of packet data
        /// </summary>
        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
        public struct Packet
        {
            /// <summary>
            /// Packet Id
            /// </summary>
            public int id;

            /// <summary>
            /// Packet data
            /// </summary>
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
            public string data;
        }

        #endregion

        #region Nested Types

        /// <summary>
        /// Registered channel names
        /// </summary>
        private static class ChannelNames
        {
            #region Fields

            /// <summary>
            /// Receive data from JetBridge
            /// </summary>
            public const string PublicDownlinkChannel = "theomessin.jetbridge.downlink";

            /// <summary>
            /// Send data to JetBridge
            /// </summary>
            public const string PublicUplinkChannel = "theomessin.jetbridge.uplink";

            #endregion
        }

        /// <summary>
        /// Hold a response from JetBridge
        /// </summary>
        private class PacketResponse : IDisposable
        {
            #region Fields

            /// <summary>
            /// Set when <see cref="Response" /> is set
            /// </summary>
            private readonly ManualResetEvent _done = new ManualResetEvent(false);

            /// <summary>
            /// Backing field for <see cref="Response" />
            /// </summary>
            private string _response;

            #endregion

            #region Properties

            /// <summary>
            /// Handle that is set when <see cref="Response" /> is set
            /// </summary>
            public WaitHandle Received => _done;

            /// <summary>
            /// Response from JetBridge
            /// </summary>
            public string Response
            {
                get => _response;
                set
                {
                    _response = value;
                    _done.Set();
                }
            }

            #endregion

            #region Methods

            /// <summary>
            /// Free up resources
            /// </summary>
            public void Dispose()
            {
                _done?.Dispose();
            }

            #endregion
        }

        #endregion
    }
}
