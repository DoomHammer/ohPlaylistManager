#include <functional>
#include <algorithm>

#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Parser.h>
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
	static const TInt kPlaylistFull = 801;
	static const Brn kPlaylistFullMsg("Playlist full");
	static const TInt kInvalidRequest = 802;
	static const Brn kInvalidRequestMsg("Space separated id request list invalid");
	
	class ProviderPlaylistManager : public DvProviderAvOpenhomeOrgPlaylistManager1, IPlaylistManagerListener
	{
		static const TUint kMaxNameBytes = 60;
		
	public:
		ProviderPlaylistManager(DvDevice& aDevice, PlaylistManager& aPlaylistManager, const TUint aMaxPlaylistCount, const TUint aMaxTrackCount);
		~ProviderPlaylistManager() {}
		
		virtual void MetadataChanged();
		virtual void PlaylistsChanged();
		virtual void PlaylistChanged();
		
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
		
		void UpdateMetadata();
		void UpdateIdArray();
		void UpdateTokenArray();
		void UpdateArrays();
		
		PlaylistManager& iPlaylistManager;
	};
} // Net
} // OpenHome

using namespace OpenHome;
using namespace OpenHome::Net;

ProviderPlaylistManager::ProviderPlaylistManager(DvDevice& aDevice, PlaylistManager& aPlaylistManager, const TUint aMaxPlaylistCount, const TUint aMaxTrackCount)
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
	Brhz metadata;
	GetPropertyMetadata(metadata);
	
	aResponse.Start();
	aMetadata.Write(metadata);
	aMetadata.WriteFlush();
	aResponse.End();
}

void ProviderPlaylistManager::ImagesXml(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aImagesXml)
{
}

void ProviderPlaylistManager::PlaylistReadArray(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseBinary& aArray)
{
	Bws<PlaylistData::kMaxTracks> idArray;
	
	try
	{
		iPlaylistManager.IdArray(aId, idArray);
		
		aResponse.Start();
		aArray.Write(idArray);
		aArray.WriteFlush();
		aResponse.End();
	}
	catch(PlaylistManagerError& e)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistReadMetadata(IInvocationResponse& aResponse, TUint aVersion, const Brx& aIdList, IInvocationResponseString& aMetadata)
{
	TUint tracksMax;
    GetPropertyTracksMax(tracksMax);
    vector<TUint> v;
    v.reserve(tracksMax);
	
	try {
        Parser parser(aIdList);
        Brn id;
        id.Set(parser.Next(' '));
		
        for( ; id != Brx::Empty(); id.Set(parser.Next(' ')) ) {
            v.push_back(Ascii::Uint(id));
            if(v.size() > tracksMax) {
                THROW(AsciiError);
            }
        }
    }
    catch(AsciiError)
	{
        aResponse.Error(kInvalidRequest, kInvalidRequestMsg);
    }
	
	aResponse.Start();
	iPlaylistManager.PlaylistReadMetadata(v, aMetadata);
	aResponse.End();
}

void ProviderPlaylistManager::PlaylistRead(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseString& aName, IInvocationResponseString& aDescription, IInvocationResponseUint& aImageId)
{
	Bws<PlaylistHeader::kMaxNameBytes> name;
	Bws<PlaylistHeader::kMaxDescriptionBytes> description;
	TUint imageId;
	
	try
	{
		iPlaylistManager.PlaylistRead(aId, name, description, imageId);
	
		aResponse.Start();
		
		aName.Write(name);
		aName.WriteFlush();
		
		aDescription.Write(description);
		aDescription.WriteFlush();
		
		aImageId.Write(imageId);
		
		aResponse.End();
	}
	catch(PlaylistManagerError& e)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistUpdate(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aName, const Brx& aDescription, TUint aImageId)
{
	try
	{
		iPlaylistManager.PlaylistInsert(aId, aName, aDescription, aImageId);
		iPlaylistManager.PlaylistDelete(aId);
		
		aResponse.Start();
		aResponse.End();
		
		UpdateArrays();
	}
	catch (PlaylistManagerError& e)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistInsert(IInvocationResponse& aResponse, TUint aVersion, TUint aAfterId, const Brx& aName, const Brx& aDescription, TUint aImageId, IInvocationResponseUint& aNewId)
{
	try
	{
		TUint newId = iPlaylistManager.PlaylistInsert(aAfterId, aName, aDescription, aImageId);
		
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
	iPlaylistManager.PlaylistDelete(aValue);
	
	aResponse.Start();
	aResponse.End();
	
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
	Brh idArray;
	GetPropertyIdArray(idArray);
	
	Brh tokenArray;
	GetPropertyTokenArray(tokenArray);
	
	aResponse.Start();
	aToken.Write(iPlaylistManager.Token());
	aIdArray.Write(idArray);
	aIdArray.WriteFlush();
	aTokenArray.Write(tokenArray);
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
	Bws<Track::kMaxMetadataBytes> metadata;
	
	try
	{
		iPlaylistManager.Read(aId, aTrackId, metadata);
		
		aResponse.Start();
		aMetadata.Write(metadata);
		aMetadata.WriteFlush();
		aResponse.End();
	}
	catch(PlaylistManagerError& e)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
	catch(PlaylistError& e)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::ReadList(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aTrackIdList, IInvocationResponseString& aTrackList)
{
}

void ProviderPlaylistManager::Insert(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aAfterTrackId, const Brx& aUdn, const Brx& aMetadataId, IInvocationResponseUint& aNewTrackId)
{
	try
	{
		const TUint newId = iPlaylistManager.Insert(aId, aAfterTrackId, aUdn, aMetadataId);
		
		aResponse.Start();
		aNewTrackId.Write(newId);
		aResponse.End();
		
		UpdateArrays();
	}
	catch (PlaylistManagerError& e)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
	catch(PlaylistError& e)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
	catch(PlaylistFull& e)
	{
		aResponse.Error(kPlaylistFull, kPlaylistFullMsg);
	}
}

void ProviderPlaylistManager::DeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId, TUint aValue)
{
	iPlaylistManager.Delete(aTrackId, aValue);
	
	aResponse.Start();
	aResponse.End();
}

void ProviderPlaylistManager::DeleteAll(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId)
{
	iPlaylistManager.DeleteAll(aTrackId);
	
	aResponse.Start();
	aResponse.End();
}

void ProviderPlaylistManager::MetadataChanged()
{
	UpdateMetadata();
}

void ProviderPlaylistManager::PlaylistsChanged()
{
	UpdateArrays();
}

void ProviderPlaylistManager::PlaylistChanged()
{
	UpdateTokenArray();
}

void ProviderPlaylistManager::UpdateMetadata()
{
	Bws<PlaylistManager::kMaxMetadataBytes * 2> metadata;
	
	iPlaylistManager.Metadata(metadata);
	
	SetPropertyMetadata(metadata);
}

void ProviderPlaylistManager::UpdateIdArray()
{
	Bws<PlaylistManager::kMaxPlaylists> idArray;
	iPlaylistManager.IdArray(idArray);
	SetPropertyIdArray(idArray);
}

void ProviderPlaylistManager::UpdateTokenArray()
{
	Bws<PlaylistManager::kMaxPlaylists> tokenArray;
	iPlaylistManager.TokenArray(tokenArray);
	SetPropertyTokenArray(tokenArray);
}

void ProviderPlaylistManager::UpdateArrays()
{
	UpdateIdArray();
	UpdateTokenArray();
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

void Metadata::Condense(const Brx& aIn, Bwx& aOut)
{
	if(aIn.Bytes() > Track::kMaxMetadataBytes)
	{
		aOut.Append(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>"));
		aOut.Append(Brn("Metadata too large"));
		aOut.Append(Brn("</dc:title><upnp:class>object.item</upnp:class></item></DIDL-Lite>"));
	}
	else
	{
		aOut.Append(aIn);
	}
}



IdGenerator::IdGenerator(const TUint aNextId)
: iNextId(aNextId)
{
}


IdGenerator::IdGenerator()
	: iNextId(0)
{
}

TUint IdGenerator::NewId()
{
	return ++iNextId;
}



Cache::Cache()
	: iMutex("Cache")
{
}

PlaylistData* Cache::Data(const Playlist& aPlaylist, ICacheListener* aCacheListener)
{
	iMutex.Wait();
	
	const TUint id = aPlaylist.Id();
	list< pair<PlaylistData*, ICacheListener*> >::iterator i = iList.begin();
	for(; i != iList.end(); ++i)
	{
		if((*i).first->IsId(id))
		{
			iMutex.Signal();
			return (*i).first;
		}
	}
		
	PlaylistData* playlistData = new PlaylistData(aPlaylist.Id(), aPlaylist.Filename());
	
	iList.push_back(pair<PlaylistData*, ICacheListener*>(playlistData, aCacheListener));
	
	iMutex.Signal();
	
	return playlistData;
}


PlaylistHeader::PlaylistHeader(const Brx& aFilename, const Brx& aName, const Brx& aDescription, const TUint aImageId)
	: iName(aName)
	, iFilename(aFilename)
	, iDescription(aDescription)
	, iImageId(aImageId)
{
}

PlaylistHeader::PlaylistHeader(const Brx& aFilename, IReader& aReader)
	: iFilename(aFilename)
{
	aReader.ReadUntil('<');
	Brn nameTag = aReader.ReadUntil('>');
	if(nameTag == Brn("Name"))
	{
		iName.Replace(aReader.ReadUntil('<'));
		
		aReader.ReadUntil('<');
		Brn descriptionTag = aReader.ReadUntil('>');
		if(descriptionTag == Brn("Description"))
		{
			iDescription.Replace(aReader.ReadUntil('<'));
			
			aReader.ReadUntil('<');
			Brn imageTag = aReader.ReadUntil('>');
			if(imageTag == Brn("ImageId"))
			{
				iImageId = Ascii::Uint(aReader.ReadUntil('<'));
				aReader.ReadUntil('>');
			}
		}
	}
}

const Brx& PlaylistHeader::Filename() const
{
	return iFilename;
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

void PlaylistHeader::ToXml(IWriter& aWriter) const
{
	WriterAscii ascii(aWriter);
	
	ascii.Write(Brn("  <Name>")); ascii.Write(Name()); ascii.Write(Brn("</Name>\n"));
	ascii.Write(Brn("  <Description>")); ascii.Write(Description()); ascii.Write(Brn("</Description>\n"));
	ascii.Write(Brn("  <ImageId>")); ascii.WriteUint(ImageId()); ascii.Write(Brn("</ImageId>\n"));
	
	ascii.WriteFlush();
}



Track::Track(const TUint aId, const Brx& aUdn, const Brx& aMetadata)
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

const Brx& Track::Udn() const
{
	return iUdn;
}

const Brx& Track::Metadata() const
{
	return iMetadata;
}



PlaylistData::PlaylistData(const TUint aId, const Brx& aFilename)
	: iId(aId)
{
	// load playlist from file
	Bws<Ascii::kMaxUintStringBytes + 1> filename(aFilename);
	ReaderFile file(filename.PtrZ());
	Srs<Track::kMaxMetadataBytes> reader(file);
	
	// skip over header information
	while(true)
	{
		reader.ReadUntil('<');
		if(reader.ReadUntil('>') == Brn("/ImageId"))
		{
			break;
		}
	}
	
	Brn trackTag("Track");
	Brn udnTag("Udn");
	Brn metadataTag("Metadata");
	
	while(true)
	{
		reader.ReadUntil('<');								// beginning of <Track>
		if(reader.ReadUntil('>') == trackTag)				// end of <Track>
		{
			reader.ReadUntil('<');							// beginning of <Udn>
			if(reader.ReadUntil('>') == udnTag)				// end of <Udn>
			{
				Brn udn = reader.ReadUntil('<');			// beginning of </Udn>
				reader.ReadUntil('<');						// end of </Udn>
				if(reader.ReadUntil('>') == metadataTag)
				{
					Brn escaped = reader.ReadUntil('<');	// beginning of </Metadata>
					Bws<Track::kMaxMetadataBytes * 2> metadata;
					metadata.Replace(escaped);
					Converter::FromXmlEscaped(metadata);
					
					iTracks.push_back(new Track(iIdGenerator.NewId(), udn, metadata));
					
					reader.ReadUntil('>');					// end of </Metadata>
					reader.ReadUntil('>');					// end of </Track>
				}
			}
		}
		else
		{
			break;
		}

	}
}

PlaylistData::~PlaylistData()
{
	for(list<Track*>::iterator i = iTracks.begin(); i != iTracks.end(); ++i)
	{
		delete *i;
	}
}

bool PlaylistData::IsId(const TUint aId) const
{
	return aId == iId;
}
 
void PlaylistData::IdArray(Bwx& aIdArray)
{
	aIdArray.SetBytes(0);
    WriterBuffer writer(aIdArray);
    WriterBinary binary(writer);
	
    for(list<Track*>::const_iterator i = iTracks.begin(); i != iTracks.end(); ++i)
	{
        binary.WriteUint32Be((*i)->Id());
    }
}

void PlaylistData::Read(const TUint aTrackId, Bwx& aMetadata)
{
	list<Track*>::const_iterator i = find_if(iTracks.begin(), iTracks.end(), bind2nd(mem_fun(&Track::IsId), aTrackId));
	if(i == iTracks.end())
	{
		THROW(PlaylistError);
	}
	
	aMetadata.Append((*i)->Metadata());
}

const TUint PlaylistData::Insert(const TUint aAfterId, const Brx& aUdn, const Brx& aMetadata)
{
    if(iTracks.size() == kMaxTracks)
	{
        THROW(PlaylistFull);
    }
	
    list<Track*>::iterator i;
    if(aAfterId == 0)
	{
        i = iTracks.begin();
    }
    else
	{
        i = find_if(iTracks.begin(), iTracks.end(), bind2nd(mem_fun(&Track::IsId), aAfterId));
        if(i == iTracks.end())
		{
            THROW(PlaylistError);
        }
        ++i;  //we insert after the id, not before
    }
	
	TUint id = iIdGenerator.NewId();
	Bws<Track::kMaxMetadataBytes> metadata;
	
	Metadata::Condense(aMetadata, metadata);
    iTracks.insert(i, new Track(id, aUdn, metadata));

	return id;
}

void PlaylistData::Delete(const TUint aId)
{
	list<Track*>::iterator i = find_if(iTracks.begin(), iTracks.end(), bind2nd(mem_fun(&Track::IsId), aId));
	if(i != iTracks.end())
	{
		delete (*i);
		iTracks.erase(i);
	}
}

void PlaylistData::DeleteAll()
{
	for(list<Track*>::iterator i = iTracks.begin(); i != iTracks.end(); ) {
		delete (*i);
		i = iTracks.erase(i);
	}
}

void PlaylistData::ToXml(IWriter& aWriter) const
{
	WriterAscii ascii(aWriter);
	
	Brn trackStart("  <Track>\n");
	Brn udnStart("    <Udn>");
	Brn metadataStart("    <Metadata>");
	Brn trackEnd("  </Track>\n");
	Brn udnEnd("</Udn>\n");
	Brn metadataEnd("</Metadata>\n");
	
	for(list<Track*>::const_iterator i = iTracks.begin(); i != iTracks.end(); ++i)
	{
		ascii.Write(trackStart);
		
		ascii.Write(udnStart); ascii.Write((*i)->Udn()); ascii.Write(udnEnd);
		ascii.Write(metadataStart); Converter::ToXmlEscaped(ascii, (*i)->Metadata()); ascii.Write(metadataEnd);
		
		ascii.Write(trackEnd);
	}
	
	ascii.WriteFlush();
}


Playlist::Playlist(Cache* aCache, const TUint aId, const Brx& aFilename, const Brx& aName, const Brx& aDescription, const TUint aImageId)
	: iCache(aCache)
	, iId(aId)
	, iToken(0)
	, iMutex("PList")
	, iHeader(aFilename, aName, aDescription, aImageId)
{
}

Playlist::Playlist(Cache* aCache, const TUint aId, const Brx& aFilename, IReader& aReader)
	: iCache(aCache)
	, iId(aId)
	, iToken(0)
	, iMutex("PList")
	, iHeader(aFilename, aReader)
{
}

const TUint Playlist::Id() const
{
	return iId;
}

bool Playlist::IsId(const TUint aId) const
{
	return aId == iId;
}

const TUint Playlist::Token() const
{
	return iToken;
}

const Brx& Playlist::Filename() const
{
	return iHeader.Filename();
}

const Brx& Playlist::Name() const
{
	return iHeader.Name();
}

const Brx& Playlist::Description() const
{
	return iHeader.Description();
}

TUint Playlist::ImageId() const
{
	return iHeader.ImageId();
}

void Playlist::IdArray(Bwx& aIdArray)
{
	iMutex.Wait();
	
	if(iData == NULL)
	{
		iData = iCache->Data(*this, this);
	}
	
	iData->IdArray(aIdArray);
	
	iMutex.Signal();
}

void Playlist::Read(const TUint aTrackId, Bwx& aMetadata)
{
	iMutex.Wait();
	
	if(iData == NULL)
	{
		iData = iCache->Data(*this, this);
	}
	
	try
	{
		iData->Read(aTrackId, aMetadata);
	}
	catch(PlaylistError& e)
	{
		iMutex.Signal();
		throw e;
	}
	
	iMutex.Signal();
}

const TUint Playlist::Insert(const TUint aAfterId, const Brx& aUdn, const Brx& aMetadata)
{
	iMutex.Wait();
	
	if(iData == NULL)
	{
		iData = iCache->Data(*this, this);
	}
	
	try
	{
		TUint newId = iData->Insert(aAfterId, aUdn, aMetadata);
		++iToken;
		
		iMutex.Signal();
		
		return newId;
	}
	catch(PlaylistFull& e)
	{
		iMutex.Signal();
		throw e;
	}
	catch(PlaylistError& e)
	{
		iMutex.Signal();
		throw e;
	}
}

void Playlist::Delete(const TUint aId)
{
	iMutex.Wait();
	
	if(iData == NULL)
	{
		iData = iCache->Data(*this, this);
	}
	
	iData->Delete(aId);
	++iToken;
	
	iMutex.Signal();
}

void Playlist::DeleteAll()
{
	iMutex.Wait();
	
	if(iData == NULL)
	{
		iData = iCache->Data(*this, this);
	}
	
	iData->DeleteAll();
	++iToken;
	
	iMutex.Signal();
}

void Playlist::ToXml(IWriter& aWriter)
{
	iMutex.Wait();
	
	if(iData == NULL)
	{
		iData = iCache->Data(*this, this);
	}
	
	aWriter.Write(Brn("<Playlist>\n"));
	
	iHeader.ToXml(aWriter);
	iData->ToXml(aWriter);
	
	aWriter.Write(Brn("</Playlist>\n"));
	
	iMutex.Signal();
}

void Playlist::RemovedFromCache()
{
	iMutex.Wait();
	
	iData = NULL;
	
	iMutex.Signal();
}





PlaylistManager::PlaylistManager(DvDevice& aDevice, const TIpAddress& aAdapter, const Brx& aName, const Brx& aImage, const Brx& aMimeType)
	: iDevice(aDevice)
	, iImage(aImage)
	, iMimeType(aMimeType)
	, iName(aName)
	, iAdapter(aAdapter)
	, iToken(0)
	, iMutex("PMngr")
{
	try 
	{
		ReaderFile toc("Toc.txt");
		Srs<20> tocReader(toc);
		
		TUint lastId = 0;
		TUint count = Ascii::Uint(tocReader.ReadUntil('\n'));
		for(int i = 0; i < count; ++i)
		{
			const Brn name = tocReader.ReadUntil('\n');
			ReaderBuffer nameReader;
			nameReader.Set(name);
			TUint id = Ascii::Uint(nameReader.ReadUntil('.'));
			
			if(id > lastId)
			{
				lastId = id;
			}
			
			Bws<Ascii::kMaxUintStringBytes + 1> filename(name);
			ReaderFile file(filename.PtrZ());
			Srs<Track::kMaxMetadataBytes> playlistReader(file);
			
			playlistReader.ReadUntil('<');
			Brn playlistTag = playlistReader.ReadUntil('>');
			if(playlistTag == Brn("Playlist"))
			{
				Playlist* playlist = new Playlist(&iCache, id, filename, playlistReader);
				iPlaylists.push_back(playlist);
			}
			
			file.Close();
		}
		
		toc.Close();
		
		iIdGenerator = IdGenerator(lastId);
	}
	catch(ReaderFileError)
	{
	}
	
	iProvider = new ProviderPlaylistManager(aDevice, *this, kMaxPlaylists, PlaylistData::kMaxTracks);
	
	MetadataChanged();
	PlaylistsChanged();
	PlaylistChanged();
}

PlaylistManager::~PlaylistManager()
{
	delete iProvider;
	
	for(list<Playlist*>::iterator i = iPlaylists.begin(); i != iPlaylists.end(); ++i)
	{
		delete *i;
	}
}

void PlaylistManager::SetName(const Brx& aValue)
{
	iName.Replace(aValue);
	MetadataChanged();
}

void PlaylistManager::SetAdapter(const TIpAddress& aAdapter)
{
	iAdapter = aAdapter;
	MetadataChanged();
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

void PlaylistManager::Metadata(Bwx& aMetadata) const
{
	WriterBuffer buffer(aMetadata);
	
	iMutex.Wait();
	
	buffer.Write(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>"));
	Converter::ToXmlEscaped(buffer, iName);
	buffer.Write(Brn("</dc:title><upnp:albumArtURI>http://"));
	
	Endpoint endPoint;
	endPoint.SetAddress(iAdapter);
	endPoint.AppendAddress(aMetadata);
	
	iMutex.Signal();
	
	buffer.Write(Brn("/images/Icon.png</upnp:albumArtURI><upnp:class>object.container</upnp:class></item></DIDL-Lite>"));
}

void PlaylistManager::PlaylistReadMetadata(std::vector<TUint>& aIdList, IWriter& aWriter) const
{
	Brn containerStart("<container id=");
	Brn containerMiddle(" restricted=\"True\">");
	Brn containerEnd("</container>");
	Brn titleStart("<dc:title>");
	Brn titleEnd("</dc:title>");
	Brn albumArtStart("<upnp:albumArtURI>");
	Brn albumArtEnd("</upnp:albumArtURI>");
	Brn classTag("<upnp:class>object.container</upnp:class>");
	Brn image("image:");
	
	aWriter.Write(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\">"));
	
	iMutex.Wait();
	
	for(vector<TUint>::const_iterator id = aIdList.begin(); id != aIdList.end(); ++id)
	{
		list<Playlist*>::const_iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), (*id)));
		if(i != iPlaylists.end())
		{
			aWriter.Write(containerStart); Ascii::StreamWriteUint(aWriter, (*i)->Id()); aWriter.Write(containerMiddle);
			aWriter.Write(titleStart); aWriter.Write((*i)->Name()); aWriter.Write(titleEnd);
			aWriter.Write(albumArtStart); aWriter.Write(image); Ascii::StreamWriteUint(aWriter, (*i)->ImageId()); aWriter.Write(albumArtEnd);
			aWriter.Write(containerEnd);
		}
	}
	
	iMutex.Signal();
	
	aWriter.Write(Brn("</DIDL-Lite>"));
	aWriter.WriteFlush();
}

void PlaylistManager::MetadataChanged()
{
	iProvider->MetadataChanged();
}

void PlaylistManager::PlaylistsChanged()
{
	++iToken;
	iProvider->PlaylistsChanged();
}

void PlaylistManager::PlaylistChanged()
{
	++iToken;
	iProvider->PlaylistChanged();
}

void PlaylistManager::IdArray(Bwx& aIdArray) const
{
	iMutex.Wait();
	
	WriterBuffer writer(aIdArray);
	WriterBinary binary(writer);
	
	for(list<Playlist*>::const_iterator i = iPlaylists.begin(); i != iPlaylists.end(); ++i)
	{
		binary.WriteUint32Be((*i)->Id());
	}
	
	iMutex.Signal();
}

void PlaylistManager::TokenArray(Bwx& aTokenArray) const
{
	iMutex.Wait();
	
	WriterBuffer writer(aTokenArray);
	WriterBinary binary(writer);
	
	for(list<Playlist*>::const_iterator i = iPlaylists.begin(); i != iPlaylists.end(); ++i)
	{
		binary.WriteUint32Be((*i)->Token());
	}
	
	iMutex.Signal();
}

void PlaylistManager::PlaylistRead(const TUint aId, Bwx& aName, Bwx& aDescription, TUint& aImageId) const
{
	iMutex.Wait();
	
	list<Playlist*>::const_iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}
	
	aName.Replace((*i)->Name());
	aDescription.Replace((*i)->Description());
	aImageId = (*i)->ImageId();
	
	iMutex.Signal();
}

const TUint PlaylistManager::PlaylistInsert(const TUint aAfterId, const Brx& aName, const Brx& aDescription, const TUint aImageId)
{
	iMutex.Wait();
	
	list<Playlist*>::iterator i;
    if(aAfterId == 0)
	{
        i = iPlaylists.begin();
    }
    else
	{
        i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aAfterId));
        if(i == iPlaylists.end())
		{
            iMutex.Signal();
            THROW(PlaylistManagerError);
        }
        ++i;  // we insert after the id, not before
    }
	
	TUint id = iIdGenerator.NewId();
	Bws<Ascii::kMaxUintStringBytes> filename;
	Ascii::AppendDec(filename, id);
	filename.Append(Brn(".txt"));
	
	Playlist* playlist = new Playlist(&iCache, id, filename, aName, aDescription, aImageId);
	iPlaylists.insert(i, playlist);
	
	WriteToc();
	WritePlaylist(*playlist);
	
	iMutex.Signal();
	
	PlaylistsChanged();
	
	return id;
}

void PlaylistManager::PlaylistDelete(const TUint aId)
{
	iMutex.Wait();
	
	if(aId == 0)
	{
		iMutex.Signal();
        return;
    }
	list<Playlist*>::iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		return;
	}
	iPlaylists.erase(i);
	
	try
	{
		WriteToc();
		// delete old playlist file.
	}
	catch(ReaderFileError& e)
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}
	
	iMutex.Signal();
	
	PlaylistsChanged();
}

void PlaylistManager::IdArray(const TUint aId, Bwx& aIdArray)
{
	iMutex.Wait();
	
	if(aId == 0)
	{
		iMutex.Signal();
        THROW(PlaylistManagerError);
    }
	
	list<Playlist*>::iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}
	
	(*i)->IdArray(aIdArray);
	
	iMutex.Signal();
}

void PlaylistManager::Read(const TUint aId, const TUint aTrackId, Bwx& aMetadata)
{
	iMutex.Wait();
	
	list<Playlist*>::iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}

	try
	{
		(*i)->Read(aTrackId, aMetadata);
	
		iMutex.Signal();
	}
	catch(PlaylistError& e)
	{
		iMutex.Signal();
		throw e;
	}
}

const TUint PlaylistManager::Insert(const TUint aId, const TUint aAfterId, const Brx& aUdn, const Brx& aMetadata)
{
	iMutex.Wait();
	
	list<Playlist*>::iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}
	
	try
	{
		const TUint newId = (*i)->Insert(aAfterId, aUdn, aMetadata);
		
		WritePlaylist(*(*i));
		
		iMutex.Signal();
		
		PlaylistChanged();
		
		return newId;
	}
	catch(PlaylistFull& e)
	{
		iMutex.Signal();
		throw e;
	}
	catch(PlaylistError& e)
	{
		iMutex.Signal();
		throw e;
	}
}

void PlaylistManager::Delete(const TUint aId, const TUint aTrackId)
{
	iMutex.Wait();
	
	list<Playlist*>::iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}
	
	(*i)->Delete(aTrackId);
	
	WritePlaylist(*(*i));
	
	iMutex.Signal();
	
	PlaylistChanged();
}

void PlaylistManager::DeleteAll(const TUint aId)
{
	iMutex.Wait();
	
	list<Playlist*>::iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}
	
	(*i)->DeleteAll();
	
	WritePlaylist(*(*i));
	
	iMutex.Signal();
	
	PlaylistChanged();
}

void PlaylistManager::WriteToc() const
{
	WriterFile file("Toc.txt");
	Sws<1024> writer(file);
	WriterAscii ascii(writer);
	
	ascii.WriteUint(iPlaylists.size());
	ascii.Write(Brn("\n"));
	for(list<Playlist*>::const_iterator i = iPlaylists.begin(); i != iPlaylists.end(); ++i)
	{
		writer.Write((*i)->Filename());
		writer.Write(Brn("\n"));
	}
	
	writer.WriteFlush();
	
	file.Close();
}

void PlaylistManager::WritePlaylist(Playlist& aPlaylist) const
{
	Bws<Ascii::kMaxUintStringBytes + 5> filename;
	Ascii::AppendDec(filename, aPlaylist.Id());
	filename.Append(".txt");
	
	WriterFile file(filename.PtrZ());
	Sws<1024> writer(file);
	WriterAscii ascii(writer);
	
	aPlaylist.ToXml(ascii);
	
	ascii.WriteFlush();
	
	file.Close();
}

