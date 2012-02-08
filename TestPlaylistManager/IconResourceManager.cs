using System;
using System.IO;

using OpenHome.Net.Device;

namespace TestPlaylistManager
{
    public class IconResourceManager : IResourceManager
    {
        public IconResourceManager(string aRootPath)
        {
            iRootPath = aRootPath;
        }

        public void WriteResource (string aUriTail, uint aIpAddress, System.Collections.Generic.List<string> aLanguageList, IResourceWriter aWriter)
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

