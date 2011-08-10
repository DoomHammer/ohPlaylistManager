#include <functional>
#include <algorithm>

#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Converter.h>
#include <OpenHome/Net/Core/DvAvOpenhomeOrgPlaylistManager1.h>

#include "PlaylistManager.h"
#include "Stream.h"

#ifdef _WIN32
# pragma warning(disable:4355) // use of 'this' in ctor lists safe in this case
#endif

using namespace std;
using namespace OpenHome;
using namespace OpenHome::Media;

namespace OpenHome {
namespace Net {
	
	static const TInt kIdNotFound = 800;
	static const Brn kIdNotFoundMsg("Id not found");
	
	class ProviderPlaylistManager : public DvProviderAvOpenhomeOrgPlaylistManager1
	{
		static const TUint kMaxNameBytes = 60;
		
	public:
		ProviderPlaylistManager(DvDevice& aDevice, PlaylistManager& aPlaylistManager, const TIpAddress& aAdapter, const Brx& aName, const TUint aMaxPlaylistCount, const TUint aMaxTrackCount);
		~ProviderPlaylistManager() {}
		
		void UpdateMetadata();
		void UpdateArrays();
		
	private:
		virtual void Metadata(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aMetadata);
		virtual void ImagesXml(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aImagesXml);
		virtual void PlaylistReadArray(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseBinary& aArray);
		virtual void PlaylistReadMetadata(IInvocationResponse& aResponse, TUint aVersion, const Brx& aIdList, IInvocationResponseString& aMetadata);
		virtual void PlaylistRead(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseString& aName, IInvocationResponseString& aDescription, IInvocationResponseUint& aImageId);
		virtual void PlaylistUpdate(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aName, const Brx& aDescription, TUint aImageId);
		virtual void PlaylistInsert(IInvocationResponse& aResponse, TUint aVersion, TUint aAfterId, const Brx& aName, const Brx& aDescription, TUint aImageId, IInvocationResponseUint& aNewId);
		virtual void PlaylistDeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aValue);
		virtual void PlaylistsMax(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aValue);
		virtual void TracksMax(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aValue);
		virtual void PlaylistArrays(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aToken, IInvocationResponseBinary& aIdArray, IInvocationResponseBinary& aTokenArray);
		virtual void PlaylistArraysChanged(IInvocationResponse& aResponse, TUint aVersion, TUint aToken, IInvocationResponseBool& aValue);
		virtual void Read(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aTrackId, IInvocationResponseString& aMetadata);
		virtual void ReadList(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aTrackIdList, IInvocationResponseString& aTrackList);
		virtual void Insert(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aAfterTrackId, const Brx& aUdn, const Brx& aMetadataId, IInvocationResponseUint& aNewTrackId);
		virtual void DeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId, TUint aValue);
		virtual void DeleteAll(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId);
		
		PlaylistManager& iPlaylistManager;
	};
} // Net
} // OpenHome

using namespace OpenHome;
using namespace OpenHome::Net;

ProviderPlaylistManager::ProviderPlaylistManager(DvDevice& aDevice, PlaylistManager& aPlaylistManager, const TIpAddress& aAdapter, const Brx& aName, const TUint aMaxPlaylistCount, const TUint aMaxTrackCount)
	: DvProviderAvOpenhomeOrgPlaylistManager1(aDevice)
	, iPlaylistManager(aPlaylistManager)
{
	EnableActionMetadata();
	EnableActionImagesXml();
	EnableActionPlaylistReadArray();
	EnableActionPlaylistReadMetadata();
	EnableActionPlaylistRead();
	EnableActionPlaylistUpdate();
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
	
	SetPropertyImagesXml(Brx::Empty());
	SetPropertyPlaylistsMax(aMaxPlaylistCount);
	SetPropertyTracksMax(aMaxTrackCount);
	
	UpdateMetadata();
	UpdateArrays();
}

void ProviderPlaylistManager::Metadata(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aMetadata)
{
	Brhz buf;
	GetPropertyMetadata(buf);
	
	aResponse.Start();
	aMetadata.Write(buf);
	aMetadata.WriteFlush();
	aResponse.End();
}

void ProviderPlaylistManager::ImagesXml(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aImagesXml)
{
}

void ProviderPlaylistManager::PlaylistReadArray(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseBinary& aArray)
{
	const Playlist* playlist = iPlaylistManager.GetPlaylist(aId);
	
	if(playlist == NULL)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
	else
	{
		aResponse.Start();
		aArray.Write(playlist->IdArray());
		aArray.WriteFlush();
		aResponse.End();
	}
}

void ProviderPlaylistManager::PlaylistReadMetadata(IInvocationResponse& aResponse, TUint aVersion, const Brx& aIdList, IInvocationResponseString& aMetadata)
{
}

void ProviderPlaylistManager::PlaylistRead(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseString& aName, IInvocationResponseString& aDescription, IInvocationResponseUint& aImageId)
{
	const PlaylistHeader* header = iPlaylistManager.Header(aId);
	
	if(header == NULL)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
	else
	{
		aResponse.Start();
		aName.Write(header->Name());
		aName.WriteFlush();
		aDescription.Write(header->Description());
		aDescription.WriteFlush();
		aImageId.Write(header->ImageId());
		aResponse.End();
	}
}

void ProviderPlaylistManager::PlaylistUpdate(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aName, const Brx& aDescription, TUint aImageId)
{
	UpdateArrays();
}

void ProviderPlaylistManager::PlaylistInsert(IInvocationResponse& aResponse, TUint aVersion, TUint aAfterId, const Brx& aName, const Brx& aDescription, TUint aImageId, IInvocationResponseUint& aNewId)
{
	try
	{
		TUint newId = iPlaylistManager.Insert(aAfterId, aName, aDescription, aImageId);
		
		aResponse.Start();
		aNewId.Write(newId);
		aResponse.End();
		
		UpdateArrays();
	}
	catch (PlaylistManagerError& e)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistDeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aValue)
{
	UpdateArrays();
}

void ProviderPlaylistManager::PlaylistsMax(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aValue)
{
	TUint count;
	GetPropertyPlaylistsMax(count);
	
	aResponse.Start();
	aValue.Write(count);
	aResponse.End();
}

void ProviderPlaylistManager::TracksMax(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aValue)
{
	TUint count;
	GetPropertyTracksMax(count);
	
	aResponse.Start();
	aValue.Write(count);
	aResponse.End();
}

void ProviderPlaylistManager::PlaylistArrays(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aToken, IInvocationResponseBinary& aIdArray, IInvocationResponseBinary& aTokenArray)
{
	aResponse.Start();
	aToken.Write(iPlaylistManager.Token());
	aIdArray.Write(iPlaylistManager.IdArray());
	aIdArray.WriteFlush();
	aTokenArray.Write(iPlaylistManager.TokenArray());
	aTokenArray.WriteFlush();
	aResponse.End();
}

void ProviderPlaylistManager::PlaylistArraysChanged(IInvocationResponse& aResponse, TUint aVersion, TUint aToken, IInvocationResponseBool& aValue)
{
	aResponse.Start();
	aValue.Write(iPlaylistManager.TokenChanged(aToken));
	aResponse.End();
}

void ProviderPlaylistManager::Read(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aTrackId, IInvocationResponseString& aMetadata)
{
}

void ProviderPlaylistManager::ReadList(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aTrackIdList, IInvocationResponseString& aTrackList)
{
}

void ProviderPlaylistManager::Insert(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aAfterTrackId, const Brx& aUdn, const Brx& aMetadataId, IInvocationResponseUint& aNewTrackId)
{
}

void ProviderPlaylistManager::DeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId, TUint aValue)
{
}

void ProviderPlaylistManager::DeleteAll(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId)
{
}

void ProviderPlaylistManager::UpdateMetadata()
{
	Bws<500> buf;
	WriterBuffer writer(buf);
	
	writer.Write(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>"));
	Converter::ToXmlEscaped(writer, iPlaylistManager.Name());
	writer.Write(Brn("</dc:title><upnp:albumArtURI>http://"));
	
	Endpoint endPoint;
	endPoint.SetAddress(iPlaylistManager.Adapter());
	endPoint.AppendAddress(buf);
	
	writer.Write(Brn("/images/Icon.png</upnp:albumArtURI><upnp:class>object.container</upnp:class></item></DIDL-Lite>"));
	
	SetPropertyMetadata(buf);
}

void ProviderPlaylistManager::UpdateArrays()
{
	SetPropertyIdArray(iPlaylistManager.IdArray());
	SetPropertyTokenArray(iPlaylistManager.TokenArray());
}




/*PlaylistManagerPersistFs::PlaylistManagerPersistFs()
{
}

TUint PlaylistManagerPersistFs::PlaylistCount() const
{
	return 0;
}

TUint PlaylistManagerPersistFs::MaxTrackCount() const
{
	return -1;
}

TUint PlaylistManagerPersistFs::MaxPlaylistCount() const
{
	return -1;
}

void PlaylistManagerPersistFs::Save(const Playlist& aPlaylist)
{
}

void PlaylistManagerPersistFs::Delete(const Playlist& aPlaylist)
{
}*/


IdGenerator::IdGenerator()
	: iNextId(0)
{
}

TUint IdGenerator::NewId()
{
	return ++iNextId;
}


PlaylistHeader::PlaylistHeader(const TUint aId, const Brx& aName, const Brx& aDescription, const TUint aImageId)
	: iId(aId)
	, iName(aName)
	, iDescription(aDescription)
	, iImageId(aImageId)
    , iToken(0)
{
}

TUint PlaylistHeader::Id() const
{
	return iId;
}

bool PlaylistHeader::IsId(TUint aValue) const
{
	return aValue == iId;
}

TUint PlaylistHeader::Token() const
{
	return iToken;
}

void PlaylistHeader::Modified()
{
	++iToken;
}

const Brx& PlaylistHeader::Name() const
{
	return iName;
}

const Brx& PlaylistHeader::Description() const
{
	return iDescription;
}

TUint PlaylistHeader::ImageId() const
{
	return iImageId;
}



Track::Track(const TUint aId, const Brn& aUdn, const Brn& aMetadata)
	: iId(aId)
	, iUdn(aUdn)
	, iMetadata(aMetadata)
{
}

TUint Track::Id() const
{
	return iId;
}

bool Track::IsId(const TUint aId) const
{
	return aId == iId;
}

const Brn& Track::Udn() const
{
	return iUdn;
}

const Brn& Track::Metadata() const
{
	return iMetadata;
}



Playlist::Playlist(const IdGenerator& aIdGenerator, const PlaylistHeader& aHeader)
	: iIdGenerator(aIdGenerator)
	, iId(aHeader.Id())
{
	// load playlist from file
}

Playlist::~Playlist()
{
	for(list<Track*>::iterator i = iTracks.begin(); i != iTracks.end(); ++i)
	{
		delete *i;
	}
}

TUint Playlist::Id() const
{
	return iId;
}

bool Playlist::IsId(const TUint aId) const
{
	return aId == iId;
}
 
const Brx& Playlist::IdArray() const
{
	return iIdArray;
}

const Track* Playlist::GetTrack(const TUint aId) const
{
	if(aId == 0)
	{
		return NULL;
	}
	else
	{
        list<Track*>::const_iterator i = find_if(iTracks.begin(), iTracks.end(), bind2nd(mem_fun(&Track::IsId), aId));
        if(i != iTracks.end()) {
			return *i;
        }
		return NULL;
	}
}

void Playlist::Insert(const Brn& aUdn, const Brn& aMetadata)
{
}

void Playlist::Delete(const TUint aId)
{
}

void Playlist::DeleteAll()
{
}




PlaylistManager::PlaylistManager(DvDevice& aDevice, const TIpAddress& aAdapter, const Brx& aName, const Brx& aImage, const Brx& aMimeType)
	: iDevice(aDevice)
	, iImage(aImage)
	, iMimeType(aMimeType)
	, iName(aName)
	, iAdapter(aAdapter)
	, iToken(0)
	, iIdArray(Brx::Empty())
	, iTokenArray(Brx::Empty())
	, iMutex("PMngr")
{
	iProvider = new ProviderPlaylistManager(aDevice, *this, aAdapter, aName, kMaxPlaylists, Playlist::kMaxTracks);
}

PlaylistManager::~PlaylistManager()
{
	delete iProvider;
	
	for(list<PlaylistHeader*>::iterator i = iPlaylistHeaders.begin(); i != iPlaylistHeaders.end(); ++i)
	{
		delete *i;
	}
	
	for(list<Playlist*>::iterator i = iPlaylists.begin(); i != iPlaylists.end(); ++i)
	{
		delete *i;
	}
}

void PlaylistManager::SetName(const Brx& aValue)
{
	iName.Replace(aValue);
	iProvider->UpdateMetadata();
}

void PlaylistManager::SetAdapter(const TIpAddress& aAdapter)
{
	iAdapter = aAdapter;
	iProvider->UpdateMetadata();
}

const Brx& PlaylistManager::Name() const
{
	return iName;
}

const TIpAddress& PlaylistManager::Adapter() const
{
	return iAdapter;
}

const TUint PlaylistManager::Token() const
{
	return iToken;
}

const Brx& PlaylistManager::IdArray() const
{
	return iIdArray;
}

const Brx& PlaylistManager::TokenArray() const
{
	return iTokenArray;
}

const TBool PlaylistManager::TokenChanged(const TUint aToken) const
{
	TBool changed = false;
	
	iMutex.Wait();
	if(aToken != iToken)
	{
		changed = true;
	}
	iMutex.Signal();
	
	return changed;
}

const PlaylistHeader* PlaylistManager::Header(TUint aId) const
{
	iMutex.Wait();
	
	if(aId == 0)
	{
		iMutex.Signal();
		return NULL;
	}
	else
	{
        list<PlaylistHeader*>::const_iterator i = find_if(iPlaylistHeaders.begin(), iPlaylistHeaders.end(), bind2nd(mem_fun(&PlaylistHeader::IsId), aId));
        if(i != iPlaylistHeaders.end()) 
		{
			iMutex.Signal();
			return *i;
        }
		iMutex.Signal();
		return NULL;
	}
}

const Playlist* PlaylistManager::GetPlaylist(TUint aId)
{
	iMutex.Wait();
	
	if(aId == 0)
	{
		iMutex.Signal();
		return NULL;
	}
	else
	{
        list<Playlist*>::const_iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
        if(i != iPlaylists.end()) {
			iMutex.Signal();
			return *i;
        }
		
		// playlist not in cache, try to load it
		
		const PlaylistHeader* header = Header(aId);
		if(header != NULL)
		{
			Playlist* playlist = new Playlist(iIdGenerator, *header);
			iPlaylists.push_back(playlist);
			
			iMutex.Signal();
			return playlist;
		}
		
		iMutex.Signal();
		return NULL;
	}
}

const TUint PlaylistManager::Insert(const TUint aAfterId, const Brx& aName, const Brx& aDescription, const TUint aImageId)
{
	iMutex.Wait();
	/*WriterFile writer("");
	writer.Write(aName);
	writer.Write(Brn("\n"));
	writer.Write(aDescription);
	writer.Write(Brn("\n"));
	writer.Close();*/
	list<PlaylistHeader*>::iterator i;
    if(aAfterId == 0)
	{
        i = iPlaylistHeaders.begin();
    }
    else
	{
        i = find_if(iPlaylistHeaders.begin(), iPlaylistHeaders.end(), bind2nd(mem_fun(&PlaylistHeader::IsId), aAfterId));
        if(i == iPlaylistHeaders.end()) {
            iMutex.Signal();
            THROW(PlaylistManagerError);
        }
        ++i;  // we insert after the id, not before
    }
	
	TUint id = iIdGenerator.NewId();
	PlaylistHeader* header = new PlaylistHeader(id, aName, aDescription, aImageId);
	iPlaylistHeaders.insert(i, header);
	
	UpdateArrays();
	
	WriteToc();
	WritePlaylist(*header);
	
	iMutex.Signal();
	
	return id;
}

void PlaylistManager::UpdateArrays()
{
	++iToken;
	
	iIdArray.SetBytes(0);
	WriterBuffer writerId(iIdArray);
	WriterBinary binaryId(writerId);
	
	iTokenArray.SetBytes(0);
	WriterBuffer writerToken(iTokenArray);
	WriterBinary binaryToken(writerToken);	
	
	for(list<PlaylistHeader*>::const_iterator i = iPlaylistHeaders.begin(); i != iPlaylistHeaders.end(); ++i)
	{
		binaryId.WriteUint32Be((*i)->Id());
		binaryToken.WriteUint32Be((*i)->Token());
	}
}

void PlaylistManager::WriteToc() const
{
	WriterFile file("Toc.txt");
	Sws<1024> writer(file);
	WriterAscii ascii(writer);
	
	for(list<PlaylistHeader*>::const_iterator i = iPlaylistHeaders.begin(); i != iPlaylistHeaders.end(); ++i)
	{
		ascii.WriteUint((*i)->Id());
		writer.Write(Brn(".txt\n"));
	}
	
	writer.WriteFlush();
	
	file.Close();
}

void PlaylistManager::WritePlaylist(const PlaylistHeader& aHeader) const
{
	Bws<Ascii::kMaxUintStringBytes + 5> filename;
	Ascii::AppendDec(filename, aHeader.Id());
	filename.Append(".txt");
	
	WriterFile file(filename.PtrZ());
	Sws<1024> writer(file);
	WriterAscii ascii(writer);
	
	ascii.Write(Brn("<Playlist>\n"));
	
	ascii.Write(Brn("  <name>")); ascii.Write(aHeader.Name()); ascii.Write(Brn("</name>\n"));
	ascii.Write(Brn("  <description>")); ascii.Write(aHeader.Description()); ascii.Write(Brn("</description>\n"));
	ascii.Write(Brn("  <imageId>")); ascii.WriteUint(aHeader.ImageId()); ascii.Write(Brn("</imageId>\n"));
	
	ascii.Write(Brn("</Playlist>\n"));
	
	writer.WriteFlush();
	
	file.Close();
}

void PlaylistManager::WritePlaylist(const PlaylistHeader& aHeader, const Playlist& aPlaylist) const
{
}

//void PlaylistManager::WritePlaylist(
