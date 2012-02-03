using System;
using System.IO;

using OpenHome;
using OpenHome.Net.Core;
using OpenHome.Net.Device;
using OpenHome.Media;

namespace TestPlaylistManager
{
    class MainClass
    {
        public static void Main (string[] args)
        {
            InitParams initParams = new InitParams();
            initParams.DvNumServerThreads = 20;

            Library library = Library.Create(initParams);

            SubnetList subnetList = new SubnetList();
            NetworkAdapter adapter = subnetList.SubnetAt(0);
            adapter.AddRef("PlaylistManager");
            subnetList.Destroy();

            library.StartCombined(adapter.Subnet());

            Directory.CreateDirectory("db");
            PlaylistManager pm = new PlaylistManager(adapter, "db", Environment.MachineName, "OpenHome", "http://www.openhome.org");
            adapter.RemoveRef("PlaylistManager");
            pm.Start();

            bool exit = false;
            while (!exit)
            {
                ConsoleKeyInfo keyInfo = Console.ReadKey(true);
                if (keyInfo.KeyChar == 'q')
                {
                    exit = true;
                }
            }

            pm.Stop(OnStop);
            pm.Dispose();

            library.Close();
        }

        private static void OnStop()
        {
        }
    }
}
