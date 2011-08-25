#include <functional>
#include <algorithm>

#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Parser.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Converter.h>

#include "PlaylistManager.h"
#include "Stream.h"

#ifdef _WIN32
# pragma warning(disable:4355) // use of 'this' in ctor lists safe in this case
#endif

using namespace std;
using namespace OpenHome;
using namespace OpenHome::Media;
using namespace OpenHome::Net;

static const TInt kIdNotFound = 800;
static const Brn kIdNotFoundMsg("Id not found");
static const TInt kPlaylistFull = 801;
static const Brn kPlaylistFullMsg("Playlist full");
static const TInt kInvalidRequest = 802;
static const Brn kInvalidRequestMsg("Space separated id request list invalid");

ProviderPlaylistManager::ProviderPlaylistManager(DvDevice& aDevice, PlaylistManager& aPlaylistManager, const TUint aMaxPlaylistCount, const TUint aMaxTrackCount)
	: DvProviderAvOpenhomeOrgPlaylistManager1(aDevice)
	, iPlaylistManager(aPlaylistManager)
{
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
	
	SetPropertyImagesXml(Brx::Empty());
	SetPropertyPlaylistsMax(aMaxPlaylistCount);
	SetPropertyTracksMax(aMaxTrackCount);
	
	UpdateMetadata();
	UpdateArrays();
}

void ProviderPlaylistManager::Metadata(IDvInvocation& aResponse, IDvInvocationResponseString& aMetadata)
{
	Brhz metadata;
	GetPropertyMetadata(metadata);
	
	aResponse.StartResponse();
	aMetadata.Write(metadata);
	aMetadata.WriteFlush();
	aResponse.EndResponse();
}

void ProviderPlaylistManager::ImagesXml(IDvInvocation& aResponse, IDvInvocationResponseString& aImagesXml)
{
	aResponse.StartResponse();
	iPlaylistManager.ImagesXml(aImagesXml);
	aResponse.EndResponse();
}

void ProviderPlaylistManager::PlaylistReadArray(IDvInvocation& aResponse, TUint aId, IDvInvocationResponseBinary& aArray)
{
	Bws<PlaylistData::kMaxTracks> idArray;
	
	try
	{
		iPlaylistManager.IdArray(aId, idArray);
		
		aResponse.StartResponse();
		aArray.Write(idArray);
		aArray.WriteFlush();
		aResponse.EndResponse();
	}
	catch(PlaylistManagerError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistReadList(IDvInvocation& aResponse, const Brx& aIdList, IDvInvocationResponseString& aPlaylistList)
{
	TUint playlistsMax;
    GetPropertyPlaylistsMax(playlistsMax);
    vector<TUint> v;
    v.reserve(playlistsMax);
	
	try {
        Parser parser(aIdList);
        Brn id;
        id.Set(parser.Next(' '));
		
        for( ; id != Brx::Empty(); id.Set(parser.Next(' ')) ) {
            v.push_back(Ascii::Uint(id));
            if(v.size() > playlistsMax) {
                THROW(AsciiError);
            }
        }
    }
    catch(AsciiError)
	{
        aResponse.Error(kInvalidRequest, kInvalidRequestMsg);
    }
	
	aResponse.StartResponse();
	iPlaylistManager.PlaylistReadList(v, aPlaylistList);
	aResponse.EndResponse();
}

void ProviderPlaylistManager::PlaylistRead(IDvInvocation& aResponse, TUint aId, IDvInvocationResponseString& aName, IDvInvocationResponseString& aDescription, IDvInvocationResponseUint& aImageId)
{
	Bws<PlaylistHeader::kMaxNameBytes> name;
	Bws<PlaylistHeader::kMaxDescriptionBytes> description;
	TUint imageId;
	
	try
	{
		iPlaylistManager.PlaylistRead(aId, name, description, imageId);
	
		aResponse.StartResponse();
		
		aName.Write(name);
		aName.WriteFlush();
		
		aDescription.Write(description);
		aDescription.WriteFlush();
		
		aImageId.Write(imageId);
		
		aResponse.EndResponse();
	}
	catch(PlaylistManagerError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistSetName(IDvInvocation& aResponse, TUint aId, const Brx& aName)
{
	try
	{
		iPlaylistManager.PlaylistSetName(aId, aName);
		
		aResponse.StartResponse();
		aResponse.EndResponse();
		
		UpdateTokenArray();
	}
	catch (PlaylistManagerError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistSetDescription(IDvInvocation& aResponse, TUint aId, const Brx& aDescription)
{
	try
	{
		iPlaylistManager.PlaylistSetDescription(aId, aDescription);
		
		aResponse.StartResponse();
		aResponse.EndResponse();
		
		UpdateTokenArray();
	}
	catch (PlaylistManagerError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistSetImageId(IDvInvocation& aResponse, TUint aId, TUint aImageId)
{
	try
	{
		iPlaylistManager.PlaylistSetImageId(aId, aImageId);
		
		aResponse.StartResponse();
		aResponse.EndResponse();
		
		UpdateTokenArray();
	}
	catch (PlaylistManagerError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistInsert(IDvInvocation& aResponse, TUint aAfterId, const Brx& aName, const Brx& aDescription, TUint aImageId, IDvInvocationResponseUint& aNewId)
{
	try
	{
		TUint newId = iPlaylistManager.PlaylistInsert(aAfterId, aName, aDescription, aImageId);
		
		aResponse.StartResponse();
		aNewId.Write(newId);
		aResponse.EndResponse();
		
		UpdateArrays();
	}
	catch (PlaylistManagerError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistDeleteId(IDvInvocation& aResponse, TUint aValue)
{
	iPlaylistManager.PlaylistDelete(aValue);
	
	aResponse.StartResponse();
	aResponse.EndResponse();
	
	UpdateArrays();
}

void ProviderPlaylistManager::PlaylistMove(IDvInvocation& aResponse, TUint aId, TUint aAfterId)
{
	try
	{
		iPlaylistManager.PlaylistMove(aId, aAfterId);
		
		aResponse.StartResponse();
		aResponse.EndResponse();
		
		UpdateArrays();
	}
	catch(PlaylistManagerError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::PlaylistsMax(IDvInvocation& aResponse, IDvInvocationResponseUint& aValue)
{
	TUint count;
	GetPropertyPlaylistsMax(count);
	
	aResponse.StartResponse();
	aValue.Write(count);
	aResponse.EndResponse();
}

void ProviderPlaylistManager::TracksMax(IDvInvocation& aResponse, IDvInvocationResponseUint& aValue)
{
	TUint count;
	GetPropertyTracksMax(count);
	
	aResponse.StartResponse();
	aValue.Write(count);
	aResponse.EndResponse();
}

void ProviderPlaylistManager::PlaylistArrays(IDvInvocation& aResponse, IDvInvocationResponseUint& aToken, IDvInvocationResponseBinary& aIdArray, IDvInvocationResponseBinary& aTokenArray)
{
	Brh idArray;
	GetPropertyIdArray(idArray);
	
	Brh tokenArray;
	GetPropertyTokenArray(tokenArray);
	
	aResponse.StartResponse();
	aToken.Write(iPlaylistManager.Token());
	aIdArray.Write(idArray);
	aIdArray.WriteFlush();
	aTokenArray.Write(tokenArray);
	aTokenArray.WriteFlush();
	aResponse.EndResponse();
}

void ProviderPlaylistManager::PlaylistArraysChanged(IDvInvocation& aResponse, TUint aToken, IDvInvocationResponseBool& aValue)
{
	aResponse.StartResponse();
	aValue.Write(iPlaylistManager.TokenChanged(aToken));
	aResponse.EndResponse();
}

void ProviderPlaylistManager::Read(IDvInvocation& aResponse, TUint aId, TUint aTrackId, IDvInvocationResponseString& aUdn, IDvInvocationResponseString& aMetadata)
{
	Bws<Track::kMaxUdnBytes> udn;
	Bws<Track::kMaxMetadataBytes> metadata;
	
	try
	{
		iPlaylistManager.Read(aId, aTrackId, udn, metadata);
		
		aResponse.StartResponse();
		aUdn.Write(udn);
		aUdn.WriteFlush();
		aMetadata.Write(metadata);
		aMetadata.WriteFlush();
		aResponse.EndResponse();
	}
	catch(PlaylistManagerError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
	catch(PlaylistError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
}

void ProviderPlaylistManager::ReadList(IDvInvocation& aResponse, TUint aId, const Brx& aTrackIdList, IDvInvocationResponseString& aTrackList)
{
	TUint tracksMax;
    GetPropertyTracksMax(tracksMax);
    vector<TUint> v;
    v.reserve(tracksMax);
	
	try {
        Parser parser(aTrackIdList);
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
	
	if(iPlaylistManager.PlaylistExists(aId))
	{
		aResponse.StartResponse();
		iPlaylistManager.ReadList(aId, v, aTrackList);
		aResponse.EndResponse();
	}
	else
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}

}

void ProviderPlaylistManager::Insert(IDvInvocation& aResponse, TUint aId, TUint aAfterTrackId, const Brx& aUdn, const Brx& aMetadataId, IDvInvocationResponseUint& aNewTrackId)
{
	try
	{
		const TUint newId = iPlaylistManager.Insert(aId, aAfterTrackId, aUdn, aMetadataId);
		
		aResponse.StartResponse();
		aNewTrackId.Write(newId);
		aResponse.EndResponse();
		
		UpdateArrays();
	}
	catch (PlaylistManagerError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
	catch(PlaylistError)
	{
		aResponse.Error(kIdNotFound, kIdNotFoundMsg);
	}
	catch(PlaylistFull)
	{
		aResponse.Error(kPlaylistFull, kPlaylistFullMsg);
	}
}
void ProviderPlaylistManager::DeleteId(IDvInvocation& aResponse, TUint aId, TUint aTrackId)
{
	iPlaylistManager.Delete(aId, aTrackId);
	
	aResponse.StartResponse();
	aResponse.EndResponse();
}

void ProviderPlaylistManager::DeleteAll(IDvInvocation& aResponse, TUint aTrackId)
{
	iPlaylistManager.DeleteAll(aTrackId);
	
	aResponse.StartResponse();
	aResponse.EndResponse();
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
	PropertiesLock();
	
	UpdateIdArray();
	UpdateTokenArray();
	
	PropertiesUnlock();
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

PlaylistData& Cache::Data(const Playlist& aPlaylist, ICacheListener* aCacheListener)
{
	iMutex.Wait();
	
	const TUint id = aPlaylist.Id();
	list< pair<PlaylistData*, ICacheListener*> >::iterator i = iList.begin();
	for(; i != iList.end(); ++i)
	{
		if((*i).first->IsId(id))
		{
			pair<PlaylistData*, ICacheListener*> d = (*i);
			iList.erase(i);
			iList.push_back(d);
			iMutex.Signal();
			return *d.first;
		}
	}
	
	if(iList.size() == kMaxCacheSize)
	{
		iList.front().second->RemovedFromCache();
		
		delete(iList.front().second);
		
		iList.pop_front();
	}
		
	PlaylistData* playlistData = new PlaylistData(aPlaylist.Id(), aPlaylist.Filename());
	
	iList.push_back(pair<PlaylistData*, ICacheListener*>(playlistData, aCacheListener));
	
	iMutex.Signal();
	
	return *playlistData;
}


PlaylistHeader::PlaylistHeader(const Brx& aFilename, const Brx& aName, const Brx& aDescription, const TUint aImageId)
	: iFilename(aFilename)
    ,iName(aName)
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

void PlaylistHeader::Name(Bwx& aName) const
{
	aName.Replace(iName);
}

void PlaylistHeader::Description(Bwx& aDescription) const
{
	aDescription.Replace(iDescription);
}

void PlaylistHeader::ImageId(TUint& aImageId) const
{
	aImageId = iImageId;
}

void PlaylistHeader::SetName(const Brx& aName)
{
	iName.Replace(aName);
}

void PlaylistHeader::SetDescription(const Brx& aDescription)
{
	iDescription.Replace(aDescription);
}

void PlaylistHeader::SetImageId(const TUint& aImageId)
{
	iImageId = aImageId;
}

void PlaylistHeader::ToXml(IWriter& aWriter) const
{
	WriterAscii ascii(aWriter);
	
	ascii.Write(Brn("  <Name>")); ascii.Write(iName); ascii.Write(Brn("</Name>\n"));
	ascii.Write(Brn("  <Description>")); ascii.Write(iDescription); ascii.Write(Brn("</Description>\n"));
	ascii.Write(Brn("  <ImageId>")); ascii.WriteUint(iImageId); ascii.Write(Brn("</ImageId>\n"));
	
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
	
	try
	{
		// skip over header information
		for(;;)
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
		
		for(;;)
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
	catch(ReaderFileError)
	{
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

void PlaylistData::Read(const TUint aTrackId, Bwx& aUdn, Bwx& aMetadata)
{
	list<Track*>::const_iterator i = find_if(iTracks.begin(), iTracks.end(), bind2nd(mem_fun(&Track::IsId), aTrackId));
	if(i == iTracks.end())
	{
		THROW(PlaylistError);
	}
	
	aUdn.Append((*i)->Udn());
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
	: iMutex("PList")
	, iId(aId)
	, iToken(0)
    , iCache(aCache)
	, iHeader(aFilename, aName, aDescription, aImageId)
	, iData(0)
{
}

Playlist::Playlist(Cache* aCache, const TUint aId, const Brx& aFilename, IReader& aReader)
	: iMutex("PList")
	, iId(aId)
	, iToken(0)
    , iCache(aCache)
	, iHeader(aFilename, aReader)
	, iData(0)
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

void Playlist::Name(Bwx& aName) const
{
	iHeader.Name(aName);
}

void Playlist::Description(Bwx& aDescription) const
{
	iMutex.Wait();
	
	iHeader.Description(aDescription);
	
	iMutex.Signal();
}

void Playlist::ImageId(TUint& aImageId) const
{
	iMutex.Wait();
	
	iHeader.ImageId(aImageId);
	
	iMutex.Signal();
}

void Playlist::SetName(const Brx& aName)
{
	iMutex.Wait();
	
	iHeader.SetName(aName);
	++iToken;
	
	iMutex.Signal();
}

void Playlist::SetDescription(const Brx& aDescription)
{
	iMutex.Wait();
	
	iHeader.SetDescription(aDescription);
	++iToken;
	
	iMutex.Signal();
}

void Playlist::SetImageId(const TUint& aImageId)
{
	iMutex.Wait();
	
	iHeader.SetImageId(aImageId);
	++iToken;
	
	iMutex.Signal();
}

void Playlist::IdArray(Bwx& aIdArray)
{
	iMutex.Wait();
	
	if(iData == NULL)
	{
		iData = &iCache->Data(*this, this);
	}
	
	iData->IdArray(aIdArray);
	
	iMutex.Signal();
}

void Playlist::Read(const TUint aTrackId, Bwx& aUdn, Bwx& aMetadata)
{
	iMutex.Wait();
	
	if(iData == NULL)
	{
		iData = &iCache->Data(*this, this);
	}
	
	try
	{
		iData->Read(aTrackId, aUdn, aMetadata);
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
		iData = &iCache->Data(*this, this);
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
		iData = &iCache->Data(*this, this);
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
		iData = &iCache->Data(*this, this);
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
		iData = &iCache->Data(*this, this);
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
	: iMutex("PMngr")
    , iDevice(aDevice)
    , iName(aName)
    , iAdapter(aAdapter)
	, iImage(aImage)
	, iMimeType(aMimeType)
	, iToken(0)
{
	try 
	{
		ReaderFile toc("Toc.txt");
		Srs<20> tocReader(toc);
		
		TUint lastId = 0;
		TUint count = Ascii::Uint(tocReader.ReadUntil('\n'));
		for(TUint i = 0; i < count; ++i)
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
}

PlaylistManager::~PlaylistManager()
{
	for(list<Playlist*>::iterator i = iPlaylists.begin(); i != iPlaylists.end(); ++i)
	{
		delete *i;
	}
}

void PlaylistManager::SetListener(IPlaylistManagerListener& aListener)
{
	iListener = &aListener;
	
	MetadataChanged();
	PlaylistsChanged();
	PlaylistChanged();
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

void PlaylistManager::ImagesXml(IWriter& aWriter) const
{
	/*Brn entryStart("<Entry>");
	Brn entryEnd("</Entry>");
	Brn idStart("<Id>");
	Brn idEnd("</Id>");
	Brn uriStart("<Uri>");
	Brn uriEnd("</Uri>");*/
	
	aWriter.Write(Brn("<ImageList>"));
	aWriter.Write(Brn("</ImageList>"));
}

void PlaylistManager::Metadata(Bwx& aMetadata) const
{
	WriterBuffer buffer(aMetadata);
	
	iMutex.Wait();
	
	buffer.Write(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\""));
	Converter::ToXmlEscaped(buffer, iName);
	buffer.Write(Brn("\" parentID=\"\" restricted=\"True\"><dc:title>"));
	Converter::ToXmlEscaped(buffer, iName);
	buffer.Write(Brn("</dc:title><upnp:albumArtURI>http://"));
	
	Endpoint endPoint;
	endPoint.SetAddress(iAdapter);
	endPoint.AppendAddress(aMetadata);
	
	iMutex.Signal();
	
	buffer.Write(Brn("/images/Icon.png</upnp:albumArtURI><upnp:class>object.container</upnp:class></item></DIDL-Lite>"));
}

void PlaylistManager::MetadataChanged()
{
	iListener->MetadataChanged();
}

void PlaylistManager::PlaylistsChanged()
{
	++iToken;
	iListener->PlaylistsChanged();
}

void PlaylistManager::PlaylistChanged()
{
	++iToken;
	iListener->PlaylistChanged();
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
	
	(*i)->Name(aName);
	(*i)->Description(aDescription);
	(*i)->ImageId(aImageId);
	
	iMutex.Signal();
}

void PlaylistManager::PlaylistReadList(std::vector<TUint>& aIdList, IWriter& aWriter) const
{
	Brn entryStart("<Entry>");
	Brn entryEnd("</Entry>");
	Brn idStart("<Id>");
	Brn idEnd("</Id>");
	Brn nameStart("<Name>");
	Brn nameEnd("</Name>");
	Brn descriptionStart("<Description>");
	Brn descriptionEnd("</Description>");
	Brn imageIdStart("<ImageId>");
	Brn imageIdEnd("</ImageId>");
	
	aWriter.Write(Brn("<PlaylistList>"));
	
	iMutex.Wait();
	
	for(vector<TUint>::const_iterator id = aIdList.begin(); id != aIdList.end(); ++id)
	{
		list<Playlist*>::const_iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), (*id)));
		if(i != iPlaylists.end())
		{
			aWriter.Write(entryStart);
			
			aWriter.Write(idStart); Ascii::StreamWriteUint(aWriter, (*i)->Id()); aWriter.Write(idEnd);
			
			Bws<PlaylistHeader::kMaxNameBytes> name;
			(*i)->Name(name);
			aWriter.Write(nameStart); aWriter.Write(name); aWriter.Write(nameEnd);
			
			Bws<PlaylistHeader::kMaxDescriptionBytes> description;
			(*i)->Description(description);
			aWriter.Write(descriptionStart); aWriter.Write(description); aWriter.Write(descriptionEnd);
			
			TUint imageId;
			(*i)->ImageId(imageId);
			aWriter.Write(imageIdStart); Ascii::StreamWriteUint(aWriter, imageId); aWriter.Write(imageIdEnd);
			
			aWriter.Write(entryEnd);
		}
	}
	
	iMutex.Signal();
	
	aWriter.Write(Brn("</PlaylistList>"));
	aWriter.WriteFlush();
}

void PlaylistManager::PlaylistSetName(const TUint aId, const Brx& aName)
{
	iMutex.Wait();
	
	list<Playlist*>::iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}
	
	(*i)->SetName(aName);
	
	WritePlaylist(**i);
	
	iMutex.Signal();
}

void PlaylistManager::PlaylistSetDescription(const TUint aId, const Brx& aDescription)
{
	iMutex.Wait();
	
	list<Playlist*>::iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}
	
	(*i)->SetDescription(aDescription);
	
	WritePlaylist(**i);
	
	iMutex.Signal();
}

void PlaylistManager::PlaylistSetImageId(const TUint aId, TUint& aImageId)
{
	iMutex.Wait();
	
	list<Playlist*>::iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}
	
	(*i)->SetImageId(aImageId);
	
	WritePlaylist(**i);
	
	iMutex.Signal();
	
	PlaylistChanged();
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
	Bws<Ascii::kMaxUintStringBytes + 5> filename;
	Ascii::AppendDec(filename, id);
	filename.Append(Brn(".txt"));
	
	WriterFile f(filename.PtrZ());
	f.Close();
	
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
	delete (*i);
	
	try
	{
		WriteToc();
		// delete old playlist file.
	}
	catch(ReaderFileError)
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}
	
	iMutex.Signal();
	
	PlaylistsChanged();
}

void PlaylistManager::PlaylistMove(const TUint aId, const TUint aAfterId)
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
	
	list<Playlist*>::iterator j;
    if(aAfterId == 0)
	{
        j = iPlaylists.begin();
    }
    else
	{
        j = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aAfterId));
        if(j == iPlaylists.end())
		{
            iMutex.Signal();
            THROW(PlaylistManagerError);
        }
        ++j;  // we insert after the id, not before
    }
	
	iPlaylists.insert(j, (*i));
	iPlaylists.erase(i);
	
	try
	{
		WriteToc();
	}
	catch(ReaderFileError)
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

bool PlaylistManager::PlaylistExists(const TUint aId) const
{
	iMutex.Wait();
	
	list<Playlist*>::const_iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		return false;
	}
	
	iMutex.Signal();
	
	return true;
}

void PlaylistManager::Read(const TUint aId, const TUint aTrackId, Bwx& aUdn, Bwx& aMetadata)
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
		(*i)->Read(aTrackId, aUdn, aMetadata);
	
		iMutex.Signal();
	}
	catch(PlaylistError& e)
	{
		iMutex.Signal();
		throw e;
	}
}

void PlaylistManager::ReadList(const TUint aId, std::vector<TUint>& aIdList, IWriter& aWriter)
{
	Brn entryStart("<Entry>");
	Brn entryEnd("</Entry>");
	Brn idStart("<Id>");
	Brn idEnd("</Id>");
	Brn udnStart("<Udn>");
	Brn udnEnd("</Udn>");
	Brn metadataStart("<Metadata>");
	Brn metadataEnd("</Metadata>");
	
	aWriter.Write(Brn("<TrackList>"));
	
	iMutex.Wait();
	
	list<Playlist*>::iterator i = find_if(iPlaylists.begin(), iPlaylists.end(), bind2nd(mem_fun(&Playlist::IsId), aId));
	if(i == iPlaylists.end())
	{
		iMutex.Signal();
		THROW(PlaylistManagerError);
	}	
	
	for(vector<TUint>::const_iterator id = aIdList.begin(); id != aIdList.end(); ++id)
	{
		Bws<Track::kMaxUdnBytes> udn;
		Bws<Track::kMaxMetadataBytes> metadata;
		
		try
		{
			(*i)->Read((*id), udn, metadata);
			
			aWriter.Write(entryStart);
			
			aWriter.Write(idStart); Ascii::StreamWriteUint(aWriter, (*id)); aWriter.Write(idEnd);
			aWriter.Write(udnStart); aWriter.Write(udn); aWriter.Write(udnEnd);
			aWriter.Write(metadataStart); aWriter.Write(metadata); aWriter.Write(metadataEnd);
			
			aWriter.Write(entryEnd);
		}
		catch(PlaylistError)
		{
		}
	}
	
	iMutex.Signal();
	
	aWriter.Write(Brn("</TrackList>"));
	aWriter.WriteFlush();
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
		return;
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
		return;
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

