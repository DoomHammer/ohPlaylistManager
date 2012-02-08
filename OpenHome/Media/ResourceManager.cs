using System;
using System.IO;
using System.Collections.Generic;

using OpenHome.Net.Device;

namespace OpenHome.Media
{
    public class ResourceManager : IResourceManager
    {
        public ResourceManager(string aRootPath)
        {
            iRootPath = aRootPath;
        }

        public void WriteResource(string aUriTail, uint aIpAddress, List<string> aLanguageList, IResourceWriter aWriter)
        {
            if(File.Exists(Path.Combine(iRootPath, aUriTail)))
            {
                Stream s = File.OpenRead(Path.Combine(iRootPath, aUriTail));

                aWriter.WriteResourceBegin((int)s.Length, string.Empty);

                byte[] buffer = new byte[4096];
                int count;
                while((count = s.Read(buffer, 0, buffer.Length)) > 0)
                {
                    aWriter.WriteResource(buffer, count);
                }

                aWriter.WriteResourceEnd();
            }
        }

        private string iRootPath;
    }
}

