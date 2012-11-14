using System;
using System.IO;
using System.Xml;

using OpenHome.Net.Core;
using OpenHome.Net.Device;
using OpenHome.Net.ControlPoint;

namespace OpenHome.Media
{
    public class PlaylistManager : IDisposable
    {
        public PlaylistManager(NetworkAdapter aAdapter, string aRootPath, string aMachineName, string aManufacturer, string aManufacturerUrl, string aModel, string aModelUrl, IResourceManager aResourceManager, string aIconUri, string aIconMimeType, uint aIconWidth, uint aIconHeight, uint aIconDepth)
        {
            string upnpname = string.Format("{0} {1} PlaylistManager ({2})", aManufacturer, aModel, aMachineName);
            // no need to include PlaylistManager in metadata as a user will protentially not be aware that they are running a playlist manager
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

            if (!string.IsNullOrEmpty(aIconUri))
            {
                XmlDocument result = new XmlDocument();
                var icon = result.CreateElement("icon");
                result.AppendChild(icon);
                var mimetype = result.CreateElement("mimetype");
                mimetype.InnerText = aIconMimeType;
                icon.AppendChild(mimetype);
                var width = result.CreateElement("width");
                width.InnerText = aIconWidth.ToString();
                icon.AppendChild(width);
                var height = result.CreateElement("height");
                height.InnerText = aIconHeight.ToString();
                icon.AppendChild(height);
                var depth = result.CreateElement("depth");
                depth.InnerText = aIconDepth.ToString();
                icon.AppendChild(depth);
                var url = result.CreateElement("url");
                url.InnerText = string.Format("/{0}/resource/{1}", udn, aIconUri);
                icon.AppendChild(url);
                iDevice.SetAttribute("Upnp.IconList", result.OuterXml);
            }

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

