using System;
using System.IO;

using OpenHome.Net.Core;
using OpenHome.Net.Device;

namespace OpenHome.Media
{
    public class PlaylistManager : IDisposable
    {
        public PlaylistManager(NetworkAdapter aAdapter, string aRootPath, string aMachineName, string aManufacturer, string aManufacturerUrl, string aModel, string aModelUrl, IResourceManager aResourceManager, string aIconUri)
        {
            string upnpname = string.Format("{0} {1} PlaylistManager ({2})", aManufacturer, aModel, aMachineName);
            string name = string.Format("{0} {1} ({2})", aManufacturer, aModel, aMachineName);
            string udn = string.Format("{0}-{1}-PlaylistManager-{2}", aManufacturer, aModel, aMachineName);

            string resourcePath = Path.Combine(aRootPath, "PlaylistManager");
            Directory.CreateDirectory(resourcePath);
            iDevice = new DvDeviceStandard(udn, new ResourceManager(aResourceManager));

            iDevice.SetAttribute("Upnp.Domain", "av.openhome.org");
            iDevice.SetAttribute("Upnp.Type", "PlaylistManager");
            iDevice.SetAttribute("Upnp.Version", "1");
            iDevice.SetAttribute("Upnp.FriendlyName", upnpname);
            iDevice.SetAttribute("Upnp.Manufacturer", aManufacturer);
            iDevice.SetAttribute("Upnp.ManufacturerUrl", aManufacturerUrl);
            iDevice.SetAttribute("Upnp.ModelDescription", aModel);
            iDevice.SetAttribute("Upnp.ModelName", aModel);
            iDevice.SetAttribute("Upnp.ModelNumber", "1");
            iDevice.SetAttribute("Upnp.ModelUrl", aModelUrl);
            iDevice.SetAttribute("Upnp.SerialNumber", "");
            iDevice.SetAttribute("Upnp.Upc", "");

            string databasePath = Path.Combine(resourcePath, "Database");
            Directory.CreateDirectory(databasePath);
            iEngine = new PlaylistManagerEngine(GetResourceManagerUri(aAdapter), databasePath, name, aIconUri);
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

