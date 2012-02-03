using System;

using OpenHome.Net.Device;

namespace OpenHome.Media
{
    public class PlaylistManager : IDisposable
    {
        public PlaylistManager(string aRootPath, string aMachineName, string aManufacturer, string aManufacturerUrl)
        {
            string name = string.Format("{0} ({1})", aManufacturer, aMachineName);
            string udn = string.Format("{0}-PlaylistManager-{1}", aManufacturer, aMachineName);

            iDevice = new DvDeviceStandard(udn);

            iDevice.SetAttribute("Upnp.Domain", "av.openhome.org");
            iDevice.SetAttribute("Upnp.Type", "PlaylistManager");
            iDevice.SetAttribute("Upnp.Version", "1");
            iDevice.SetAttribute("Upnp.FriendlyName", name);
            iDevice.SetAttribute("Upnp.Manufacturer", aManufacturer);
            iDevice.SetAttribute("Upnp.ManufacturerUrl", aManufacturerUrl);
            iDevice.SetAttribute("Upnp.ModelDescription", aManufacturer + " PlaylistManager");
            iDevice.SetAttribute("Upnp.ModelName", aManufacturer + " PlaylistManager");
            iDevice.SetAttribute("Upnp.ModelNumber", "1");
            iDevice.SetAttribute("Upnp.ModelUrl", aManufacturerUrl);
            iDevice.SetAttribute("Upnp.SerialNumber", "");
            iDevice.SetAttribute("Upnp.Upc", "");

            iEngine = new PlaylistManagerEngine(aRootPath, name);
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

        public void Stop(Action aAction)
        {
            iDevice.SetDisabled(aAction);
        }

        private DvDevice iDevice;
        private PlaylistManagerEngine iEngine;
        private ProviderPlaylistManager iProvider;
    }
}

