using System;
using System.IO;
using System.Collections.Generic;

using OpenHome.Net.Device;

namespace OpenHome.Media
{
    public class ResourceManager : IResourceManager
    {
        public ResourceManager(IResourceManager aResourceManager)
        {
            iResourceManager = aResourceManager;
        }

        public void WriteResource(string aUriTail, uint aIpAddress, List<string> aLanguageList, IResourceWriter aWriter)
        {
            iResourceManager.WriteResource(aUriTail, aIpAddress, aLanguageList, aWriter);
        }

        private IResourceManager iResourceManager;
    }
}

