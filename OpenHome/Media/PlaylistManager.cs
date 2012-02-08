using System;
using System.IO;

using OpenHome.Net.Core;
using OpenHome.Net.Device;

namespace OpenHome.Media
{
    public class PlaylistManager : IDisposable
    {
        public PlaylistManager(NetworkAdapter aAdapter, string aRootPath, string aMachineName, string aManufacturer, string aManufacturerUrl, string aModelUrl, string aIcon)
        {
            string name = string.Format("{0} ({1})", aManufacturer, aMachineName);
            string udn = string.Format("{0}-PlaylistManager-{1}", aManufacturer, aMachineName);

            string resourcePath = Path.Combine(aRootPath, "PlaylistManager");
            Directory.CreateDirectory(resourcePath);
            iDevice = new DvDeviceStandard(udn, new ResourceManager(aRootPath));

            iDevice.SetAttribute("Upnp.Domain", "av.openhome.org");
            iDevice.SetAttribute("Upnp.Type", "PlaylistManager");
            iDevice.SetAttribute("Upnp.Version", "1");
            iDevice.SetAttribute("Upnp.FriendlyName", name);
            iDevice.SetAttribute("Upnp.Manufacturer", aManufacturer);
            iDevice.SetAttribute("Upnp.ManufacturerUrl", aManufacturerUrl);
            iDevice.SetAttribute("Upnp.ModelDescription", aManufacturer + " PlaylistManager");
            iDevice.SetAttribute("Upnp.ModelName", aManufacturer + " PlaylistManager");
            iDevice.SetAttribute("Upnp.ModelNumber", "1");
            iDevice.SetAttribute("Upnp.ModelUrl", aModelUrl);
            iDevice.SetAttribute("Upnp.SerialNumber", "");
            iDevice.SetAttribute("Upnp.Upc", "");

            string databasePath = Path.Combine(resourcePath, "Database");
            Directory.CreateDirectory(databasePath);
            iEngine = new PlaylistManagerEngine(GetResourceManagerUri(aAdapter), databasePath, name, aIcon);
            iProvider = new ProviderPlaylistManager(iDevice, iEngine, PlaylistManagerEngine.kMaxPlaylists, PlaylistManagerEngine.kMaxTracks);
        }

        public void Dispose()
        {
            iProvider.Dispose();
            iEngine.Dispose();
            iDevice.Dispose();
        }

        public void Start()
        {
            iDevice.SetEnabled();
        }

        public void Stop(System.Action aAction)
        {
            iDevice.SetDisabled(aAction);
        }

        public void SetAdapter(NetworkAdapter aAdapter)
        {
            iEngine.ResourceManagerUri = GetResourceManagerUri(aAdapter);
        }

        private string GetResourceManagerUri(NetworkAdapter aAdapter)
        {
            //iEngine.UrlPrefix = iDevice.GetResourceManagerUri(aAdapter);
            string address = new System.Net.IPAddress(aAdapter.Address()).ToString();
            return string.Format("http://{0}:55178/{1}/Upnp/resource/", address, iDevice.Udn());
        }

        private DvDeviceStandard iDevice;
        private PlaylistManagerEngine iEngine;
        private ProviderPlaylistManager iProvider;
    }
}

