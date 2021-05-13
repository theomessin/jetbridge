using System;

namespace TheoMessin.JetBridge.Example
{
    /// <summary>
    /// Main program
    /// </summary>
    /// <remarks>
    /// Reads commands from the console and sends them to JetBridge.
    /// Writes out the result.
    /// </remarks>
    internal class Program
    {
        #region Methods

        /// <summary>
        /// Main program
        /// </summary>
        private static void Main()
        {
            using (var scr = new SimConnectRunner())
            {
                var ct = new ExampleClient(scr.Start());
                scr.ReceiveMessageAction = ct.ReceiveMessage;

                Console.WriteLine("Enter commands, blank line to exit");
                while (true)
                {
                    var cmd = Console.ReadLine().Trim();
                    if (string.IsNullOrEmpty(cmd))
                        break;
                    var response = ct.Request(cmd);
                    if (response == null)
                    {
                        Console.WriteLine("No response");
                    }
                    else
                    {
                        Console.WriteLine("RX: " + response);
                    }
                }
            }

            Console.WriteLine("Main thread exited");
        }

        #endregion
    }
}
