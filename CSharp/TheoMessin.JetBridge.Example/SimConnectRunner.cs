using System;
using System.Threading;
using System.Windows.Forms;

namespace TheoMessin.JetBridge.Example
{
    /// <summary>
    /// Background thread, window, and message loop for SimConnect
    /// </summary>
    internal class SimConnectRunner : IDisposable
    {
        #region Fields

        /// <summary>
        /// Id of the SimConnect Windows User message
        /// </summary>
        public const int SimConnectMessageId = 0x0402;

        /// <summary>
        /// Event set when window is created
        /// </summary>
        private readonly ManualResetEvent _started = new(false);

        /// <summary>
        /// Application context for window thread
        /// </summary>
        private ApplicationContext _ac;

        /// <summary>
        /// Window handle
        /// </summary>
        /// <remarks>
        /// Only valid after <see cref="_started" /> is set
        /// </remarks>
        private IntPtr _handle;

        /// <summary>
        /// Background thread
        /// </summary>
        private Thread _thread;

        #endregion

        #region Properties

        /// <summary>
        /// Callback to SimConnect ReceiveMessage
        /// </summary>
        public Action ReceiveMessageAction { get; set; }

        #endregion

        #region Methods

        /// <summary>
        /// Stop thread and clean up windows
        /// </summary>
        public void Dispose()
        {
            _ac?.ExitThread();
            _thread?.Join();
            _started?.Dispose();
        }

        /// <summary>
        /// Create the window and start the thread
        /// </summary>
        /// <returns>Handle of the hidden window</returns>
        public IntPtr Start()
        {
            _thread = new Thread(ThreadProc);
            _thread.Start();
            _started.WaitOne();
            return _handle;
        }


        /// <summary>
        /// Respond to a <see cref="SimConnectMessageId" /> message
        /// </summary>
        private void OnSimConnectMessage()
        {
            ReceiveMessageAction?.Invoke();
        }

        /// <summary>
        /// Background window thread
        /// </summary>
        private void ThreadProc()
        {
            using var scw = new SimConnectWindow(OnSimConnectMessage);
            _handle = scw.GetHandle();
            using (_ac = new ApplicationContext())
            {
                _started.Set();
                Application.Run(_ac);
            }

            _ac = null;
            _handle = IntPtr.Zero;
        }

        #endregion

        #region Nested Types

        /// <summary>
        /// Window that will call back on <see cref="SimConnectRunner.SimConnectMessageId" /> messages
        /// </summary>
        private class SimConnectWindow : NativeWindow, IDisposable
        {
            #region Fields

            /// <summary>
            /// Action to take on <see cref="SimConnectRunner.SimConnectMessageId" />
            /// </summary>
            private readonly Action _action;

            #endregion

            #region Constructors / Finalizers

            /// <summary>
            /// Constructor
            /// </summary>
            /// <param name="action">Action to take on <see cref="SimConnectRunner.SimConnectMessageId" /></param>
            public SimConnectWindow(Action action)
            {
                _action = action;
            }

            #endregion

            #region Methods

            public void Dispose()
            {
                DestroyHandle();
            }

            /// <summary>
            /// Create and return the handle
            /// </summary>
            /// <returns>Window handle</returns>
            public IntPtr GetHandle()
            {
                CreateHandle(new CreateParams());
                return Handle;
            }

            /// <summary>
            /// Invokes the default window procedure associated with this window.
            /// </summary>
            /// <param name="m">
            /// A <see cref="T:System.Windows.Forms.Message" /> that is associated with the current Windows message.
            /// </param>
            protected override void WndProc(ref Message m)
            {
                if (m.Msg == SimConnectMessageId)
                {
                    try
                    {
                        _action();
                    }
                    catch (Exception e)
                    {
                        Console.WriteLine(e);
                    }

                    return;
                }


                base.WndProc(ref m);
            }

            #endregion
        }

        #endregion
    }
}
