using System;
using System.Collections.Generic;

using OpenHome.Net.Device;
using OpenHome.Net.Device.Providers;

namespace OpenHome.Media
{
    internal class ProviderPlaylistManager : DvProviderAvOpenhomeOrgPlaylistManager1, IPlaylistManagerEngineListener, IDisposable
    {
        private static readonly uint kIdNotFound = 800;
        private static readonly string kIdNotFoundMsg = "Id not found";
        private static readonly uint kPlaylistFull = 801;
        private static readonly string kPlaylistFullMsg = "Playlist full";
        private static readonly uint kInvalidRequest = 802;
        private static readonly string kInvalidRequestMsg = "Space separated id request list invalid";

        public ProviderPlaylistManager(DvDevice aDevice, PlaylistManagerEngine aPlaylistManagerEngine, uint aMaxPlaylistCount, uint aMaxTrackCount)
            : base(aDevice)
        {
            iPlaylistManagerEngine = aPlaylistManagerEngine;

            EnablePropertyMetadata();
            EnablePropertyImagesXml();
            EnablePropertyIdArray();
            EnablePropertyTokenArray();
            EnablePropertyPlaylistsMax();
            EnablePropertyTracksMax();
            
            EnableActionMetadata();
            EnableActionImagesXml();
            EnableActionPlaylistReadArray();
            EnableActionPlaylistReadList();
            EnableActionPlaylistRead();
            EnableActionPlaylistSetName();
            EnableActionPlaylistSetDescription();
            EnableActionPlaylistSetImageId();
            EnableActionPlaylistInsert();
            EnableActionPlaylistDeleteId();
            EnableActionPlaylistsMax();
            EnableActionTracksMax();
            EnableActionPlaylistArrays();
            EnableActionPlaylistArraysChanged();
            EnableActionRead();
            EnableActionReadList();
            EnableActionInsert();
            EnableActionDeleteId();
            EnableActionDeleteAll();
            
            SetPropertyImagesXml(string.Empty);
            SetPropertyMetadata(string.Empty);
            SetPropertyPlaylistsMax(aMaxPlaylistCount);
            SetPropertyTracksMax(aMaxTrackCount);
            
            iPlaylistManagerEngine.SetListener(this);
        }

        public void UrlPrefixChanged()
        {
            UpdateUrlPrefix();
        }

        public void ImagesChanged()
        {
            UpdateImagesXml();
        }

        public void MetadataChanged()
        {
            UpdateMetadata();
        }

        public void PlaylistsChanged()
        {
            UpdateArrays();
        }

        public void PlaylistChanged()
        {
            UpdateTokenArray();
        }

        protected override void Metadata(IDvInvocation aResponse, out string aMetadata)
        {
            aMetadata = iPlaylistManagerEngine.Metadata(aResponse.ResourceUriPrefix());
        }

        protected override void ImagesXml(IDvInvocation aResponse, out string aImagesXml)
        {
            aImagesXml = iPlaylistManagerEngine.ImagesXml(aResponse.ResourceUriPrefix());
        }

        protected override void PlaylistReadArray(IDvInvocation aResponse, uint aId, out byte[] aArray)
        {
            try
            {
                iPlaylistManagerEngine.IdArray(aId, out aArray);
            }
            catch(PlaylistManagerEngine.PlaylistManagerEngineErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
        }

        protected override void PlaylistReadList(IDvInvocation aResponse, string aIdList, out string aPlaylistList)
        {
            List<uint> list = new List<uint>();
            string[] split = aIdList.Split(' ');
            try
            {
                foreach(string s in split)
                {
                    list.Add(uint.Parse(s));
                    if(list.Count > PlaylistManagerEngine.kMaxPlaylists)
                    {
                        throw new FormatException();
                    }
                }
            }
            catch(FormatException)
            {
                throw new ActionError(kInvalidRequest, kInvalidRequestMsg);
            }

            iPlaylistManagerEngine.PlaylistReadList(list, out aPlaylistList);
        }

        protected override void PlaylistRead(IDvInvocation aResponse, uint aId, out string aName, out string aDescription, out uint aImageId)
        {
            try
            {
                iPlaylistManagerEngine.PlaylistRead(aId, out aName, out aDescription, out aImageId);
            }
            catch(PlaylistManagerEngine.PlaylistManagerEngineErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
        }

        protected override void PlaylistSetName(IDvInvocation aResponse, uint aId, string aName)
        {
            try
            {
                if(aName.Length == 0)
                {
                    throw new ActionError(kInvalidRequest, kInvalidRequestMsg);
                }
                iPlaylistManagerEngine.PlaylistSetName(aId, aName);
                UpdateTokenArray();
            }
            catch(PlaylistManagerEngine.PlaylistManagerEngineErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
        }

        protected override void PlaylistSetDescription(IDvInvocation aResponse, uint aId, string aDescription)
        {
            try
            {
                iPlaylistManagerEngine.PlaylistSetDescription(aId, aDescription);
                UpdateTokenArray();
            }
            catch(PlaylistManagerEngine.PlaylistManagerEngineErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
        }

        protected override void PlaylistSetImageId(IDvInvocation aResponse, uint aId, uint aImageId)
        {
            try
            {
                iPlaylistManagerEngine.PlaylistSetImageId(aId, aImageId);
                UpdateTokenArray();
            }
            catch(PlaylistManagerEngine.PlaylistManagerEngineErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
        }

        protected override void PlaylistInsert(IDvInvocation aResponse, uint aAfterId, string aName, string aDescription, uint aImageId, out uint aNewId)
        {
            try
            {
                if(aName.Length == 0)
                {
                    throw new ActionError(kInvalidRequest, kInvalidRequestMsg);
                }
                aNewId = iPlaylistManagerEngine.PlaylistInsert(aAfterId, aName, aDescription, aImageId);
                UpdateArrays();
            }
            catch(PlaylistManagerEngine.PlaylistManagerEngineErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
        }

        protected override void PlaylistDeleteId(IDvInvocation aResponse, uint aValue)
        {
            iPlaylistManagerEngine.PlaylistDelete(aValue);
            UpdateArrays();
        }

        protected override void PlaylistMove(IDvInvocation aResponse, uint aId, uint aAfterId)
        {
            try
            {
                iPlaylistManagerEngine.PlaylistMove(aId, aAfterId);
                UpdateArrays();
            }
            catch(PlaylistManagerEngine.PlaylistManagerEngineErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
        }

        protected override void PlaylistsMax(IDvInvocation aResponse, out uint aValue)
        {
            aValue = PropertyPlaylistsMax();
        }

        protected override void TracksMax(IDvInvocation aResponse, out uint aValue)
        {
            aValue = PropertyTracksMax();
        }

        protected override void PlaylistArrays(IDvInvocation aResponse, out uint aToken, out byte[] aIdArray, out byte[] aTokenArray)
        {
            aToken = iPlaylistManagerEngine.Token;
            aIdArray = PropertyIdArray();
            aTokenArray = PropertyTokenArray();
        }

        protected override void PlaylistArraysChanged(IDvInvocation aResponse, uint aToken, out bool aValue)
        {
            aValue = iPlaylistManagerEngine.TokenChanged(aToken);
        }

        protected override void Read(IDvInvocation aResponse, uint aId, uint aTrackId, out string aMetadata)
        {
            try
            {
                iPlaylistManagerEngine.Read(aId, aTrackId, out aMetadata);
            }
            catch(PlaylistManagerEngine.PlaylistManagerEngineErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
            catch(Playlist.PlaylistErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
        }

        protected override void ReadList(IDvInvocation aResponse, uint aId, string aTrackIdList, out string aTrackList)
        {
            List<uint> list = new List<uint>();
            string[] split = aTrackIdList.Split(' ');
            try
            {
                foreach(string s in split)
                {
                    list.Add(uint.Parse(s));
                    if(list.Count > PlaylistManagerEngine.kMaxTracks)
                    {
                        throw new FormatException();
                    }
                }
            }
            catch(FormatException)
            {
                throw new ActionError(kInvalidRequest, kInvalidRequestMsg);
            }

            if(iPlaylistManagerEngine.PlaylistExists(aId))
            {
                iPlaylistManagerEngine.ReadList(aId, list, out aTrackList);
            }
            else
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
        }

        protected override void Insert(IDvInvocation aResponse, uint aId, uint aAfterTrackId, string aMetadataId, out uint aNewTrackId)
        {
            try
            {
                aNewTrackId = iPlaylistManagerEngine.Insert(aId, aAfterTrackId, aMetadataId);
                UpdateArrays();
            }
            catch(PlaylistManagerEngine.PlaylistManagerEngineErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
            catch(Playlist.PlaylistErrorException)
            {
                throw new ActionError(kIdNotFound, kIdNotFoundMsg);
            }
            catch(Playlist.PlaylistFullException)
            {
                throw new ActionError(kPlaylistFull, kPlaylistFullMsg);
            }
        }

        protected override void DeleteId(IDvInvocation aResponse, uint aId, uint aTrackId)
        {
            iPlaylistManagerEngine.Delete(aId, aTrackId);
        }

        protected override void DeleteAll(IDvInvocation aResponse, uint aTrackId)
        {
            iPlaylistManagerEngine.DeleteAll(aTrackId);
        }

        private void UpdateUrlPrefix()
        {
            PropertiesLock();

            UpdateMetadata();
            UpdateImagesXml();

            PropertiesUnlock();
        }

        private void UpdateImagesXml()
        {
            SetPropertyImagesXml(iPlaylistManagerEngine.ImagesXml());
        }

        private void UpdateMetadata()
        {
            SetPropertyMetadata(iPlaylistManagerEngine.Metadata());
        }

        private void UpdateIdArray()
        {
            SetPropertyIdArray(iPlaylistManagerEngine.PlaylistIdArray);
        }

        private void UpdateTokenArray()
        {
            SetPropertyTokenArray(iPlaylistManagerEngine.PlaylistTokenArray);
        }

        private void UpdateArrays()
        {
            PropertiesLock();

            UpdateIdArray();
            UpdateTokenArray();

            PropertiesUnlock();
        }
        
        private PlaylistManagerEngine iPlaylistManagerEngine;
    }
}

