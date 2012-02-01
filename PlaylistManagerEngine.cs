using System;
using System.IO;
using System.Xml;
using System.Collections.Generic;

namespace OpenHome.Media
{
    public interface IPlaylistManagerListener
    {
        void ImagesChanged();
        void MetadataChanged();
        void PlaylistsChanged();
        void PlaylistChanged();
    }

    internal interface INameable
    {
        string Name { set; }
    }

    public class PlaylistManager : INameable, IPlaylistManagerListener, IDisposable
    {
        public class PlaylistManagerErrorException : Exception { }

        public PlaylistManager(string aName)
        {
            iName = aName;

            iLock = new object();
            iPlaylists = new List<Playlist>();
            iCache = new Cache();
            iToken = 0;

            // read any playlists from disk
            uint lastId = 0;
            FileStream f = File.OpenRead("Toc.txt");
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
    
                    Playlist p = new Playlist(iCache, id, filename);
                    iPlaylists.Add(p);
                }

                s.Close();
            }

            iIdGenerator = new IdGenerator(lastId);
        }

        public void Dispose()
        {
        }
        
        public void SetListener(IPlaylistManagerListener aListener)
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

        public void ImagesChanged()
        {
            lock(iLock)
            {
                iListener.ImagesChanged();
            }
        }

        public void MetadataChanged()
        {
            lock(iLock)
            {
                iListener.MetadataChanged();
            }
        }

        public void PlaylistsChanged()
        {
            lock(iLock)
            {
                ++iToken;
                iListener.PlaylistsChanged();
            }
        }

        public void PlaylistChanged()
        {
            lock(iLock)
            {
                ++iToken;
                iListener.PlaylistChanged();
            }
        }
        
        public string ImagesXml(string aUrlPrefix)
        {
            lock(iLock)
            {
                // just some test code!!!
                XmlDocument doc = new XmlDocument();
                XmlElement imageList = doc.CreateElement("ImageList");
                doc.AppendChild(imageList);

                for(int i = 0; i < 1; ++i)
                {
                    XmlElement entry = doc.CreateElement("Entry");

                    XmlElement id = doc.CreateElement("Id");
                    id.AppendChild(doc.CreateTextNode((i + 1).ToString()));
                    entry.AppendChild(id);

                    XmlElement uri = doc.CreateElement("Uri");
                    uri.AppendChild(doc.CreateTextNode("http://10.2.9.146:26125/aa/2224581218/cover.jpg"));
                    entry.AppendChild(uri);

                    imageList.AppendChild(entry);
                }

                return doc.OuterXml;
            }
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
                albumArtUri.AppendChild(doc.CreateTextNode(string.Format("{0}/images/Icon.png", aUrlPrefix)));
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
                    throw new PlaylistManagerErrorException();
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
                    throw new PlaylistManagerErrorException();
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
                    throw new PlaylistManagerErrorException();
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
                    throw new PlaylistManagerErrorException();
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
                        throw new PlaylistManagerErrorException();
                    }
                    ++index;    // we insert after the id, not before
                }

                id = iIdGenerator.NewId();
                string filename = string.Format("{0}.txt", id);

                Playlist playlist = new Playlist(iCache, id, filename, aName, aDescription, aImageId);
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
                // delete old playlist file
            }

            PlaylistsChanged();
        }

        public void PlaylistMove(uint aId, uint aAfterId)
        {
            if(aId == 0)
            {
                throw new PlaylistManagerErrorException();
            }

            lock(iLock)
            {
                Playlist playlist = FindPlaylist(aId);
                if(playlist == null)
                {
                    throw new PlaylistManagerErrorException();
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
                        throw new PlaylistManagerErrorException();
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
                    throw new PlaylistManagerErrorException();
                }

                Playlist p = FindPlaylist(aId);
                if(p == null)
                {
                    throw new PlaylistManagerErrorException();
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
                    throw new PlaylistManagerErrorException();
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
                    throw new PlaylistManagerErrorException();
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
                    throw new PlaylistManagerErrorException();
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
            StreamWriter s = File.CreateText("Toc.txt");

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

        private object iLock;
        
        private IPlaylistManagerListener iListener;
        
        private IdGenerator iIdGenerator;
        private Cache iCache;
        
        private string iName;
        
        private List<Playlist> iPlaylists;
        private uint iToken;
    }
}

