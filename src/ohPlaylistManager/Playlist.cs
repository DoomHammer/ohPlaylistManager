using System;
using System.IO;
using System.Xml;
using System.Collections.Generic;

namespace OpenHome.Media
{
    internal interface IPlaylistHeader
    {
        string Filename { get; }
        string Name { get; set; }
        string Description { get; set; }
        uint ImageId { get; set; }
    }
     
    internal class PlaylistHeader : IPlaylistHeader
    {
        public PlaylistHeader(string aRootPath, string aFilename)
        {
            iFilename = aFilename;

            XmlDocument doc = new XmlDocument();
            doc.Load(Path.Combine(aRootPath, aFilename));

            iName = doc.SelectSingleNode("Playlist/Name").FirstChild.Value;
            XmlNode desc = doc.SelectSingleNode("Playlist/Description");
            if(desc.FirstChild != null)
            {
                iDescription = desc.FirstChild.Value;
            }
            iImageId = uint.Parse(doc.SelectSingleNode("Playlist/ImageId").FirstChild.Value);
        }

        public PlaylistHeader(string aFilename, string aName, string aDescription, uint aImageId)
        {
            iFilename = aFilename;
            iName = aName;
            iDescription = aDescription;
            iImageId = aImageId;
        }

        public string Filename
        {
            get
            {
                return iFilename;
            }
        }

        public string Name
        {
            get
            {
                return iName;
            }
            set
            {
                iName = value;
            }
        }

        public string Description
        {
            get
            {
                return iDescription;
            }
            set
            {
                iDescription = value;
            }
        }

        public uint ImageId
        {
            get
            {
                return iImageId;
            }
            set
            {
                iImageId = value;
            }
        }

        public void ToXml(XmlDocument aDocument, XmlNode aNode)
        {
            XmlElement name = aDocument.CreateElement("Name");
            name.AppendChild(aDocument.CreateTextNode(iName));
            aNode.AppendChild(name);

            XmlElement desc = aDocument.CreateElement("Description");
            desc.AppendChild(aDocument.CreateTextNode(iDescription));
            aNode.AppendChild(desc);

            XmlElement imageId = aDocument.CreateElement("ImageId");
            imageId.AppendChild(aDocument.CreateTextNode(iImageId.ToString()));
            aNode.AppendChild(imageId);
        }

        private readonly string iFilename;
        private string iName;
        private string iDescription;
        private uint iImageId;
    }

    internal class IdGenerator
    {
        public IdGenerator()
        {
            iNextId = 0;
        }

        public IdGenerator(uint aNextId)
        {
            iNextId = aNextId;
        }

        public uint NewId()
        {
            return ++iNextId;
        }
    
        private uint iNextId;
    }

    internal interface IPlaylistData
    {
        void IdArray(out byte[] aIdArray);

        string Read(uint aTrackId);
        uint Insert(uint aAfterId, string aMetadata);
        void Delete(uint aId);
        void DeleteAll();
    }

    internal class PlaylistData : IPlaylistData, IDisposable
    {
        public PlaylistData(uint aId, string aFilename)
        {
            iId = aId;
            iIdGenerator = new IdGenerator();
            iTracks = new List<Track>();

            XmlDocument doc = new XmlDocument();
            doc.Load(aFilename);

            XmlNodeList tracks = doc.SelectNodes("Playlist/Track/Metadata");
            foreach(XmlNode t in tracks)
            {
                iTracks.Add(new Track(iIdGenerator.NewId(), t.FirstChild.Value));
            }
        }

        public void Dispose()
        {
            iTracks.Clear();
        }

        public bool IsId(uint aId)
        {
            return iId == aId;
        }
        
        public void IdArray(out byte[] aIdArray)
        {
            MemoryStream ms = new MemoryStream();
            foreach(Track t in iTracks)
            {
                byte[] bytes = BigEndianConverter.Uint32ToBigEndian(t.Id);
                ms.Write(bytes, 0, bytes.Length);
            }
            aIdArray = ms.ToArray();
        }
        
        public string Read(uint aTrackId)
        {
            foreach(Track t in iTracks)
            {
                if(t.Id == aTrackId)
                {
                    return t.Metadata;
                }
            }

            throw new Playlist.PlaylistErrorException();
        }

        public uint Insert(uint aAfterId, string aMetadata)
        {
            if(iTracks.Count == PlaylistManagerEngine.kMaxTracks)
            {
                throw new Playlist.PlaylistFullException();
            }

            int index = 0;
            if(aAfterId > 0)
            {
                foreach(Track t in iTracks)
                {
                    if(t.IsId(aAfterId))
                    {
                        break;
                    }
                    ++index;
                }
                if(index == iTracks.Count)
                {
                    throw new Playlist.PlaylistErrorException();
                }
                ++index;    // we insert after the id, not before
            }

            uint id = iIdGenerator.NewId();
            iTracks.Insert(index, new Track(id, aMetadata));

            return id;
        }

        public void Delete(uint aId)
        {
            foreach(Track t in iTracks)
            {
                if(t.Id == aId)
                {
                    iTracks.Remove(t);
                    return;
                }
            }
        }

        public void DeleteAll()
        {
            iTracks.Clear();
        }

        public void ToXml(XmlDocument aDocument, XmlNode aNode)
        {
            foreach(Track t in iTracks)
            {
                XmlElement track = aDocument.CreateElement("Track");
                aNode.AppendChild(track);

                XmlElement metadata = aDocument.CreateElement("Metadata");
                metadata.AppendChild(aDocument.CreateTextNode(t.Metadata));
                track.AppendChild(metadata);
            }
        }

        private readonly uint iId;
        private IdGenerator iIdGenerator;
        
        private List<Track> iTracks;
    }

    internal class Track
    {
        public Track(uint aId, string aMetadata)
        {
            iId = aId;
            iMetadata = aMetadata;
        }
        
        public uint Id
        {
            get
            {
                return iId;
            }
        }

        public bool IsId(uint aId)
        {
            return iId == aId;
        }
        
        public string Metadata
        {
            get
            {
                return iMetadata;
            }
        }

        private readonly uint iId;
        private readonly string iMetadata;
    };

    internal interface ICacheListener
    {
        void RemovedFromCache();
    }

    internal class Cache
    {
        public Cache(string aRootPath)
        {
            iRootPath = aRootPath;

            iLock = new object();
            iList = new List<KeyValuePair<PlaylistData, ICacheListener>>();
        }

        public PlaylistData Data(Playlist aPlaylist, ICacheListener aCacheListener)
        {
            lock(iLock)
            {
                uint id = aPlaylist.Id;

                foreach(KeyValuePair<PlaylistData, ICacheListener> vp in iList)
                {
                    if(vp.Key.IsId(id))
                    {
                        iList.Remove(vp);
                        iList.Add(vp);
                        return vp.Key;
                    }
                }

                if(iList.Count == kMaxCacheSize)
                {
                    iList[0].Value.RemovedFromCache();
                    iList[0].Key.Dispose();
                    iList.RemoveAt(0);
                }

                PlaylistData data = new PlaylistData(id, Path.Combine(iRootPath, aPlaylist.Filename));
                iList.Add(new KeyValuePair<PlaylistData, ICacheListener>(data, aCacheListener));

                return data;
            }
        }

        public static readonly uint kMaxCacheSize = 1000;

        private string iRootPath;
        private object iLock;
        private List<KeyValuePair<PlaylistData, ICacheListener>> iList;
    };


    internal class Playlist : IPlaylistHeader, IPlaylistData, ICacheListener
    {
        public class PlaylistFullException : Exception { }
        public class PlaylistErrorException : Exception { }

        public Playlist(Cache aCache, string aRootPath, uint aId, string aFilename, string aName, string aDescription, uint aImageId)
        {
            iCache = aCache;
            iRootPath = aRootPath;
            iId = aId;

            iHeader = new PlaylistHeader(aFilename, aName, aDescription, aImageId);
            iLock = new object();
            iData = null;
            iToken = 0;
        }

        public Playlist(Cache aCache, string aRootPath, uint aId, string aFilename)
        {
            iCache = aCache;
            iRootPath = aRootPath;
            iId = aId;

            iHeader = new PlaylistHeader(aRootPath, aFilename);
            iLock = new object();
            iData = null;
            iToken = 1;
        }
    
        public uint Id
        {
            get
            {
                return iId;
            }
        }

        public bool IsId(uint aId)
        {
            return iId == aId;
        }
    
        public uint Token
        {
            get
            {
                return iToken;
            }
        }

        public string Filename
        {
            get
            {
                return iHeader.Filename;
            }
        }

        public string Name
        {
            get
            {
                lock(iLock)
                {
                    return iHeader.Name;
                }
            }
            set
            {
                lock(iLock)
                {
                    iHeader.Name = value;
                    ++iToken;
                }
            }
        }
        public string Description
        {
            get
            {
                lock(iLock)
                {
                    return iHeader.Description;
                }
            }
            set
            {
                lock(iLock)
                {
                    iHeader.Description = value;
                    ++iToken;
                }
            }
        }

        public uint ImageId
        {
            get
            {
                lock(iLock)
                {
                    return iHeader.ImageId;
                }
            }
            set
            {
                lock(iLock)
                {
                    iHeader.ImageId = value;
                    ++iToken;
                }
            }
        }
    
        public void IdArray(out byte[] aIdArray)
        {
            lock(iLock)
            {
                if(iData == null)
                {
                    iData = iCache.Data(this, this);
                }

                iData.IdArray(out aIdArray);
            }
        }
    
        public string Read(uint aTrackId)
        {
            lock(iLock)
            {
                if(iData == null)
                {
                    iData = iCache.Data(this, this);
                }

                return iData.Read(aTrackId);
            }
        }

        public uint Insert(uint aAfterId, string aMetadata)
        {
            lock(iLock)
            {
                if(iData == null)
                {
                    iData = iCache.Data(this, this);
                }

                uint newId = iData.Insert(aAfterId, aMetadata);
                ++iToken;

                return newId;
            }
        }

        public void Delete(uint aId)
        {
            lock(iLock)
            {
                if(iData == null)
                {
                    iData = iCache.Data(this, this);
                }

                iData.Delete(aId);
                ++iToken;
            }
        }

        public void DeleteAll()
        {
            lock(iLock)
            {
                if(iData == null)
                {
                    iData = iCache.Data(this, this);
                }

                iData.DeleteAll();
                ++iToken;
            }
        }
        
        public void Save()
        {
            lock(iLock)
            {
                if(File.Exists(Filename))
                {
                    if(iData == null)
                    {
                        iData = iCache.Data(this, this);
                    }
                }

                XmlDocument doc = new XmlDocument();
                XmlElement playlist = doc.CreateElement("Playlist");
                doc.AppendChild(playlist);

                iHeader.ToXml(doc, playlist);
                if(iData != null)
                {
                    iData.ToXml(doc, playlist);
                }

                doc.Save(Path.Combine(iRootPath, Filename));
            }
        }
    
        public void RemovedFromCache()
        {
            lock(iLock)
            {
                iData = null;
            }
        }

        private object iLock;

        private string iRootPath;

        private readonly uint iId;
        private uint iToken;

        private Cache iCache;
        private PlaylistHeader iHeader;
        private PlaylistData iData;
    }
}

