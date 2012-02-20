using System;
using System.IO;

using OpenHome.Net.Core;
using OpenHome.Net.Device;
using OpenHome.Net.ControlPoint;

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

            iControlPoint = new CpDeviceListUpnpServiceType("upnp.org", "ContentDirectory", 1, Added, Removed);

            string databasePath = Path.Combine(resourcePath, "Database");
            Directory.CreateDirectory(databasePath);
            iEngine = new PlaylistManagerEngine(GetResourceManagerUri(aAdapter), databasePath, name, aIconUri);
            iProvider = new ProviderPlaylistManager(iDevice, iEngine, PlaylistManagerEngine.kMaxPlaylists, PlaylistManagerEngine.kMaxTracks);
        }

        public void Dispose()
        {
            if(iProvider != null)
            {
                iProvider.Dispose();
            }

            if(iEngine != null)
            {
                iEngine.Dispose();
            }

            if(iControlPoint != null)
            {
                iControlPoint.Dispose();
            }

            if(iDevice != null)
            {
                iDevice.Dispose();
            }
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

        private void Added(CpDeviceList aList, CpDevice aDevice)
        {
            //Console.WriteLine("Added: " + aDevice.Udn());
        }

        private void Removed(CpDeviceList aList, CpDevice aDevice)
        {
            //Console.WriteLine("Removed: " + aDevice.Udn());
        }

        private string GetResourceManagerUri(NetworkAdapter aAdapter)
        {
            return iDevice.ResourceManagerUri(aAdapter);
        }

        private DvDeviceStandard iDevice;
        private CpDeviceList iControlPoint;

        private PlaylistManagerEngine iEngine;
        private ProviderPlaylistManager iProvider;
    }
}

