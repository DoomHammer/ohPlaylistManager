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
	static const TInt kInvalidRequest = 802;
	static const Brn kInvalidRequestMsg("Space separated id request list invalid");
	
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
    catch(AsciiError) {
        aResponse.Error(kInvalidRequest, kInvalidRequestMsg);
    }
	
	Brn containerStart("<container id=");
	Brn containerMiddle(" restricted=\"True\">");
	Brn containerEnd("</container>");
	Brn titleStart("<dc:title>");
	Brn titleEnd("</dc:title>");
	Brn albumArtStart("<upnp:albumArtURI>");
	Brn albumArtEnd("</upnp:albumArtURI>");
	Brn image("image:");
	
	aResponse.Start();
	
	aMetadata.Write(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\">"));
	for(vector<TUint>::const_iterator id = v.begin(); id != v.end(); ++id)
	{
		const PlaylistHeader* header = iPlaylistManager.Header(*id);
		
		if(header != NULL)
		{
			aMetadata.Write(containerStart); Ascii::StreamWriteUint(aMetadata, header->Id()); aMetadata.Write(containerMiddle);
			aMetadata.Write(titleStart); aMetadata.Write(header->Name()); aMetadata.Write(titleEnd);
			aMetadata.Write(albumArtStart); aMetadata.Write(image); Ascii::StreamWriteUint(aMetadata, header->ImageId()); aMetadata.Write(albumArtEnd);
			aMetadata.Write(containerEnd);
		}
	}
	aMetadata.Write(Brn("</DIDL-Lite>"));
	aMetadata.WriteFlush();
	
	aResponse.End();
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
	try
	{
		iPlaylistManager.Insert(aId, aName, aDescription, aImageId);
		iPlaylistManager.Delete(aId);
		
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
	iPlaylistManager.Delete(aValue);
	
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


PlaylistHeader::PlaylistHeader(const TUint aId, const Brx& aName, const Brx& aDescription, const TUint aImageId)
	: iId(aId)
	, iName(aName)
	, iDescription(aDescription)
	, iImageId(aImageId)
    , iToken(0)
{
}

PlaylistHeader::PlaylistHeader(const TUint aId, IReader& aReader)
	: iId(aId)
	
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

void PlaylistHeader::ToXml(IWriter& aWriter) const
{
	WriterAscii ascii(aWriter);
	
	ascii.Write(Brn("  <Name>")); ascii.Write(Name()); ascii.Write(Brn("</Name>\n"));
	ascii.Write(Brn("  <Description>")); ascii.Write(Description()); ascii.Write(Brn("</Description>\n"));
	ascii.Write(Brn("  <ImageId>")); ascii.WriteUint(ImageId()); ascii.Write(Brn("</ImageId>\n"));
	
	ascii.WriteFlush();
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

void Track::ToXml(IWriter& aWriter) const
{
	WriterAscii ascii(aWriter);
	
	ascii.Write(Brn("    <Udn>")); ascii.Write(Udn()); ascii.Write(Brn("    </Udn>"));
	
	ascii.Write(Brn("    <Metadata>")); Converter::ToXmlEscaped(ascii, Metadata()); ascii.Write(Brn("    </Metadata>"));
	
	ascii.WriteFlush();
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

void Playlist::ToXml(IWriter& aWriter) const
{
	WriterAscii ascii(aWriter);
	
	Brn trackStart("  <Track>");
	Brn trackEnd("  </Track>");
	
	for(list<Track*>::const_iterator i = iTracks.begin(); i != iTracks.end(); ++i)
	{
		ascii.Write(trackStart);
		(*i)-> ToXml(ascii);
		ascii.Write(trackEnd);
	}
	
	ascii.WriteFlush();
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
			ReaderFile playlist(filename.PtrZ());
			Srs<Track::kMaxMetadataBytes> playlistReader(playlist);
			
			playlistReader.ReadUntil('<');
			Brn playlistTag = playlistReader.ReadUntil('>');
			if(playlistTag == Brn("Playlist"))
			{
				PlaylistHeader* header = new PlaylistHeader(id, playlistReader);
				iPlaylistHeaders.push_back(header);
			}
			
			playlist.Close();
		}
		
		toc.Close();
		
		UpdateArrays();
		
		iIdGenerator = IdGenerator(lastId);
	}
	catch(ReaderFileError)
	{
	}
	
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

const PlaylistHeader* PlaylistManager::Header(const TUint aId) const
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

const Playlist* PlaylistManager::GetPlaylist(const TUint aId)
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

void PlaylistManager::Delete(const TUint aId)
{
	iMutex.Wait();
	
	if(aId == 0)
	{
		iMutex.Signal();
        return;
    }
	list<PlaylistHeader*>::iterator i = find_if(iPlaylistHeaders.begin(), iPlaylistHeaders.end(), bind2nd(mem_fun(&PlaylistHeader::IsId), aId));
	if(i == iPlaylistHeaders.end()) {
		iMutex.Signal();
		return;
	}
	iPlaylistHeaders.erase(i);
	
	UpdateArrays();
	
	WriteToc();
	// delete old playlist file.
	
	iMutex.Signal();
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
	
	ascii.WriteUint(iPlaylistHeaders.size());
	ascii.Write(Brn("\n"));
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
	
	aHeader.ToXml(ascii);
	
	ascii.Write(Brn("</Playlist>\n"));
	
	ascii.WriteFlush();
	
	file.Close();
}

void PlaylistManager::WritePlaylist(const PlaylistHeader& aHeader, const Playlist& aPlaylist) const
{
	Bws<Ascii::kMaxUintStringBytes + 5> filename;
	Ascii::AppendDec(filename, aHeader.Id());
	filename.Append(".txt");
	
	WriterFile file(filename.PtrZ());
	Sws<1024> writer(file);
	WriterAscii ascii(writer);
	
	ascii.Write(Brn("<Playlist>\n"));
	
	aHeader.ToXml(ascii);
	aPlaylist.ToXml(ascii);
	
	ascii.Write(Brn("</Playlist>\n"));
	
	ascii.WriteFlush();
	
	file.Close();	
}

//void PlaylistManager::WritePlaylist(
