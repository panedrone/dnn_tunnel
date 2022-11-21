using System;
using System.IO;

namespace TestConsole
{
    class Program
    {
        static void Main(string[] args)
        {
            Tunnel_Items_Finder.on_message = (msg) =>
            {
                Console.WriteLine(msg);
            };

            Tunnel_Items_Finder.on_item = (bytes) =>
            {
                Console.WriteLine(bytes.Length);
            };

            Tunnel_Items_Finder.run_cam_tracking(0);
        }
    }
}
