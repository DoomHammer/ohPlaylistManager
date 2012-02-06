using System;
using System.IO;
using System.Xml;
using System.Collections.Generic;

namespace OpenHome.Media
{
    internal interface IPlaylistManagerEngineListener
    {
        void UrlPrefixChanged();
        void ImagesChanged();
        void MetadataChanged();
        void PlaylistsChanged();
        void PlaylistChanged();
    }

    internal interface INameable
    {
        string Name { set; }
    }

    internal class PlaylistManagerEngine : INameable, IPlaylistManagerEngineListener, IDisposable
    {
        public class PlaylistManagerEngineErrorException : Exception { }

        public PlaylistManagerEngine(string aUrlPrefix, string aRootPath, string aName, string aIcon)
        {
            iUrlPrefix = aUrlPrefix;
            iRootPath = aRootPath;
            iName = aName;
            iIcon = aIcon;

            iLock = new object();
            iPlaylists = new List<Playlist>();
            iCache = new Cache(aRootPath);
            iToken = 0;

            // read any playlists from disk
            uint lastId = 0;
            string tocFilename = Path.Combine(aRootPath, "Toc.txt");
            if(File.Exists(tocFilename))
            {
                FileStream f = File.OpenRead(tocFilename);
                StreamReader s = new StreamReader(f);
                if(s != null)
                {
                    uint count = uint.Parse(s.ReadLine());
                    for(uint i = 0; i < count; ++i)
                    {
                        string filename = s.ReadLine();
                        string[] split = filename.Split(new char[] { '.' }, 2);
                        uint id = uint.Parse(split[0]);
    
                        if(id > lastId)
                        {
                            lastId = id;
                        }
    
                        Playlist p = new Playlist(iCache, aRootPath, id, filename);
                        iPlaylists.Add(p);
                    }
    
                    s.Close();
                }
            }

            iIdGenerator = new IdGenerator(lastId);
        }

        public void Dispose()
        {
        }
        
        public void SetListener(IPlaylistManagerEngineListener aListener)
        {
            lock(iLock)
            {
                iListener = aListener;
            }

            ImagesChanged();
            MetadataChanged();
            PlaylistsChanged();
            PlaylistChanged();
        }

        public string Name
        {
            get
            {
                lock(iLock)
                {
                    return iName;
                }
            }
            set
            {
                lock(iLock)
                {
                    iName = value;
                }
                MetadataChanged();
            }
        }

        public string ResourceManagerUri
        {
            get
            {
                lock(iLock)
                {
                    return iUrlPrefix;
                }
            }
            set
            {
                lock(iLock)
                {
                    iUrlPrefix = value;
                }

                UrlPrefixChanged();
            }
        }

        public uint Token
        {
            get
            {
                lock(iLock)
                {
                    return iToken;
                }
            }
        }

        public bool TokenChanged(uint aToken)
        {
            lock(iLock)
            {
                return aToken == iToken;
            }
        }

        public void UrlPrefixChanged()
        {
            lock(iLock)
            {
                if(iListener != null)
                {
                    iListener.UrlPrefixChanged();
                }
            }
        }

        public void ImagesChanged()
        {
            lock(iLock)
            {
                if(iListener != null)
                {
                    iListener.ImagesChanged();
                }
            }
        }

        public void MetadataChanged()
        {
            lock(iLock)
            {
                if(iListener != null)
                {
                    iListener.MetadataChanged();
                }
            }
        }

        public void PlaylistsChanged()
        {
            lock(iLock)
            {
                ++iToken;
                if(iListener != null)
                {
                    iListener.PlaylistsChanged();
                }
            }
        }

        public void PlaylistChanged()
        {
            lock(iLock)
            {
                ++iToken;
                if(iListener != null)
                {
                    iListener.PlaylistChanged();
                }
            }
        }

        public string ImagesXml()
        {
            return ImagesXml(iUrlPrefix);
        }
        
        public string ImagesXml(string aUrlPrefix)
        {
            lock(iLock)
            {
                XmlDocument doc = new XmlDocument();
                XmlElement imageList = doc.CreateElement("ImageList");
                doc.AppendChild(imageList);

                // just some test code!!!
                /*for(int i = 0; i < 1; ++i)
                {
                    XmlElement entry = doc.CreateElement("Entry");

                    XmlElement id = doc.CreateElement("Id");
                    id.AppendChild(doc.CreateTextNode((i + 1).ToString()));
                    entry.AppendChild(id);

                    XmlElement uri = doc.CreateElement("Uri");
                    uri.AppendChild(doc.CreateTextNode("http://10.2.9.146:26125/aa/2224581218/cover.jpg"));
                    entry.AppendChild(uri);

                    imageList.AppendChild(entry);
                }*/

                return doc.OuterXml;
            }
        }

        public string Metadata()
        {
            return Metadata(iUrlPrefix);
        }

        public string Metadata(string aUrlPrefix)
        {
            lock(iLock)
            {
                XmlDocument doc = new XmlDocument();
                XmlElement didl = doc.CreateElement("DIDL-Lite", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
                doc.AppendChild(didl);

                XmlElement item = doc.CreateElement("item", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
                item.SetAttribute("id", iName);
                item.SetAttribute("parentID", "");
                item.SetAttribute("restricted", "True");
                didl.AppendChild(item);

                XmlElement title = doc.CreateElement("dc", "title", "http://purl.org/dc/elements/1.1/");
                title.AppendChild(doc.CreateTextNode(iName));
                item.AppendChild(title);

                XmlElement albumArtUri = doc.CreateElement("upnp", "albumArtURI", "urn:schemas-upnp-org:metadata-1-0/upnp/");
                albumArtUri.AppendChild(doc.CreateTextNode(string.Format("{0}{1}", aUrlPrefix, iIcon)));
                item.AppendChild(albumArtUri);

                XmlElement classType = doc.CreateElement("upnp", "class", "urn:schemas-upnp-org:metadata-1-0/upnp/");
                classType.AppendChild(doc.CreateTextNode("object.container"));
                item.AppendChild(classType);

                return doc.OuterXml;
            }
        }

        public byte[] PlaylistIdArray
        {
            get
            {
                lock(iLock)
                {
                    MemoryStream ms = new MemoryStream();
                    foreach(Playlist p in iPlaylists)
                    {
                        byte[] bytes = BigEndianConverter.Uint32ToBigEndian(p.Id);
                        ms.Write(bytes, 0, bytes.Length);
                    }
                    return ms.ToArray();
                }
            }
        }

        public byte[] PlaylistTokenArray
        {
            get
            {
                lock(iLock)
                {
                    MemoryStream ms = new MemoryStream();
                    foreach(Playlist p in iPlaylists)
                    {
                        byte[] bytes = BigEndianConverter.Uint32ToBigEndian(p.Token);
                        ms.Write(bytes, 0, bytes.Length);
                    }
                    return ms.ToArray();
                }
            }
        }

        public void PlaylistReadList(IList<uint> aIdList, out string aPlaylistList)
        {
            lock(iLock)
            {
                XmlDocument doc = new XmlDocument();
                XmlElement list = doc.CreateElement("PlaylistList");
                doc.AppendChild(list);

                foreach(uint i in aIdList)
                {
                    Playlist p = FindPlaylist(i);
                    if(p != null)
                    {
                        XmlElement entry = doc.CreateElement("Entry");
                        list.AppendChild(entry);

                        XmlElement id = doc.CreateElement("Id");
                        id.AppendChild(doc.CreateTextNode(p.Id.ToString()));
                        entry.AppendChild(id);

                        XmlElement name = doc.CreateElement("Name");
                        name.AppendChild(doc.CreateTextNode(p.Name));
                        entry.AppendChild(name);

                        XmlElement desc = doc.CreateElement("Description");
                        desc.AppendChild(doc.CreateTextNode(p.Description));
                        entry.AppendChild(desc);

                        XmlElement imageId = doc.CreateElement("ImageId");
                        imageId.AppendChild(doc.CreateTextNode(p.ImageId.ToString()));
                        entry.AppendChild(imageId);
                    }
                }

                aPlaylistList = list.OuterXml;
            }
        }

        public void PlaylistRead(uint aId, out string aName, out string aDescription, out uint aImageId)
        {
            lock(iLock)
            {
                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    throw new PlaylistManagerEngineErrorException();
                }

                aName = p.Name;
                aDescription = p.Description;
                aImageId = p.ImageId;
            }


        }

        public void PlaylistSetName(uint aId, string aName)
        {
            lock(iLock)
            {
                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    throw new PlaylistManagerEngineErrorException();
                }
                p.Name = aName;

                WritePlaylist(p);
            }

            PlaylistChanged();
        }

        public void PlaylistSetDescription(uint aId, string aDescription)
        {
            lock(iLock)
            {
                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    throw new PlaylistManagerEngineErrorException();
                }
                p.Description = aDescription;

                WritePlaylist(p);
            }

            PlaylistChanged();
        }

        public void PlaylistSetImageId(uint aId, uint aImageId)
        {
            lock(iLock)
            {
                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    throw new PlaylistManagerEngineErrorException();
                }
                p.ImageId = aImageId;

                WritePlaylist(p);
            }

            PlaylistChanged();
        }

        public uint PlaylistInsert(uint aAfterId, string aName, string aDescription, uint aImageId)
        {
            uint id = 0;
            lock(iLock)
            {
                int index = 0;
                if(aAfterId > 0)
                {
                    foreach(Playlist p in iPlaylists)
                    {
                        if(p.IsId(aAfterId))
                        {
                            break;
                        }
                        ++index;
                    }
                    if(index == iPlaylists.Count)
                    {
                        throw new PlaylistManagerEngineErrorException();
                    }
                    ++index;    // we insert after the id, not before
                }

                id = iIdGenerator.NewId();
                string filename = string.Format("{0}.txt", id);

                Playlist playlist = new Playlist(iCache, iRootPath, id, filename, aName, aDescription, aImageId);
                iPlaylists.Insert(index, playlist);

                WriteToc();
                WritePlaylist(playlist);
            }

            PlaylistsChanged();

            return id;
        }

        public void PlaylistDelete(uint aId)
        {
            lock(iLock)
            {
                if(aId == 0)
                {
                    return;
                }

                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    return;
                }

                iPlaylists.Remove(p);

                WriteToc();

                File.Delete(Path.Combine(iRootPath, p.Filename));
            }

            PlaylistsChanged();
        }

        public void PlaylistMove(uint aId, uint aAfterId)
        {
            if(aId == 0)
            {
                throw new PlaylistManagerEngineErrorException();
            }

            lock(iLock)
            {
                Playlist playlist = FindPlaylist(aId);
                if(playlist == null)
                {
                    throw new PlaylistManagerEngineErrorException();
                }

                int index = 0;
                if(aAfterId > 0)
                {
                    foreach(Playlist p in iPlaylists)
                    {
                        if(p.IsId(aAfterId))
                        {
                            break;
                        }
                    }
                    if(index == iPlaylists.Count)
                    {
                        throw new PlaylistManagerEngineErrorException();
                    }
                    ++index;    // we insert after the id, not before
                }

                int i = iPlaylists.IndexOf(playlist);
                iPlaylists.Remove(playlist);
                if(i < index)
                {
                    --index;
                }
                iPlaylists.Insert(index, playlist);

                WriteToc();
            }

            PlaylistsChanged();
        }

        public bool PlaylistExists(uint aId)
        {
            lock(iLock)
            {
                Playlist p = FindPlaylist(aId);
                return p != null;
            }
        }
        
        public void IdArray(uint aId, out byte[] aIdArray)
        {
            lock(iLock)
            {
                if(aId == 0)
                {
                    throw new PlaylistManagerEngineErrorException();
                }

                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    throw new PlaylistManagerEngineErrorException();
                }

                p.IdArray(out aIdArray);
            }
        }

        public void Read(uint aId, uint aTrackId, out string aMetadata)
        {
            lock(iLock)
            {
                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    throw new PlaylistManagerEngineErrorException();
                }

                aMetadata = p.Read(aTrackId);
            }
        }

        public void ReadList(uint aId, IList<uint> aIdList, out string aTrackList)
        {
            lock(iLock)
            {
                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    throw new PlaylistManagerEngineErrorException();
                }

                XmlDocument doc = new XmlDocument();
                XmlElement list = doc.CreateElement("TrackList");
                doc.AppendChild(list);

                foreach(uint i in aIdList)
                {
                    try
                    {
                        string metadata = p.Read(i);
    
                        XmlElement entry = doc.CreateElement("Entry");
                        list.AppendChild(entry);

                        XmlElement id = doc.CreateElement("Id");
                        id.AppendChild(doc.CreateTextNode(i.ToString()));
                        entry.AppendChild(id);

                        XmlElement md = doc.CreateElement("Metadata");
                        md.InnerXml = metadata;
                        entry.AppendChild(md);
                    }
                    catch(Playlist.PlaylistErrorException) { }
                }

                aTrackList = list.OuterXml;
            }
        }

        public uint Insert(uint aId, uint aAfterId, string aMetadata)
        {
            uint newId = 0;
            lock(iLock)
            {
                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    throw new PlaylistManagerEngineErrorException();
                }

                newId = p.Insert(aAfterId, aMetadata);

                WritePlaylist(p);
            }

            PlaylistChanged();

            return newId;
        }

        public void Delete(uint aId, uint aTrackId)
        {
            lock(iLock)
            {
                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    return;
                }

                p.Delete(aTrackId);

                WritePlaylist(p);
            }

            PlaylistChanged();
        }

        public void DeleteAll(uint aId)
        {
            lock(iLock)
            {
                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    return;
                }

                p.DeleteAll();

                WritePlaylist(p);
            }

            PlaylistChanged();
        }

        private Playlist FindPlaylist(uint aId)
        {
            foreach(Playlist p in iPlaylists)
            {
                if(p.IsId(aId))
                {
                    return p;
                }
            }

            return null;
        }

        private void WriteToc()
        {
            StreamWriter s = File.CreateText(Path.Combine(iRootPath, "Toc.txt"));

            s.WriteLine(iPlaylists.Count);
            foreach(Playlist p in iPlaylists)
            {
                s.WriteLine(p.Filename);
            }

            s.Close();
        }

        private void WritePlaylist(Playlist aPlaylist)
        {
            aPlaylist.Save();
        }

        public static readonly uint kMaxPlaylists = 500;
        public static readonly uint kMaxTracks = 1000;

        private string iRootPath;
        private string iUrlPrefix;
        private string iName;
        private string iIcon;

        private object iLock;
        
        private IPlaylistManagerEngineListener iListener;
        
        private IdGenerator iIdGenerator;
        private Cache iCache;
        
        private List<Playlist> iPlaylists;
        private uint iToken;
    }
}

