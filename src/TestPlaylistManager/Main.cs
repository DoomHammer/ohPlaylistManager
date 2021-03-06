using System;
using System.IO;
using System.Reflection;

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
            subnetList.Dispose();

            library.StartCombined(adapter.Subnet());

            string rootPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            IconResourceManager rm = new IconResourceManager(rootPath);
            PlaylistManager pm = new PlaylistManager(adapter, rootPath, Environment.MachineName, "OpenHome", "http://www.openhome.org", "TestPlaylistManager", "http://www.openhome.org", rm, "Icon.png", "image/png", 114, 114, 32);
            adapter.RemoveRef("PlaylistManager");
            pm.Start();

            Console.WriteLine("Press 'q' to quit");
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

            library.Dispose();
        }

        private static void OnStop()
        {
        }
    }
}
