#ifndef HEADER_PLAYLISTMANAGER
#define HEADER_PLAYLISTMANAGER

#include <list>
#include <utility>

#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Net/Core/DvDevice.h>

EXCEPTION(PlaylistManagerError);
EXCEPTION(PlaylistError);
EXCEPTION(PlaylistFull);

namespace OpenHome {
namespace Net {
	class ProviderPlaylistManager;
}
}

namespace OpenHome {
namespace Media {
	
class IPlaylistManagerListener
{
public:
	virtual void MetadataChanged() = 0;
	virtual void PlaylistsChanged() = 0;
	virtual void PlaylistChanged() = 0;
};
	
class IdGenerator
{
public:
	IdGenerator();
	IdGenerator(TUint aNextId);
	
	TUint NewId();
	
private:
	TUint iNextId;
};
	
	
	
class IPlaylistHeader
{
public:
	virtual const Brx& Filename() const = 0;
	virtual const Brx& Name() const = 0;
	virtual const Brx& Description() const = 0;
	virtual TUint ImageId() const = 0;
};



class IPlaylistData
{
public:
	virtual void IdArray(Bwx& aIdArray) = 0;
	
	virtual const TUint Insert(const TUint aAfterId, const Brx& aUdn, const Brx& aMetadata) = 0;
	virtual void Delete(const TUint aId) = 0;
	virtual void DeleteAll() = 0;
};
	
	
		
class PlaylistHeader : public IPlaylistHeader
{
public:
	static const TUint kMaxNameBytes = 30;
	static const TUint kMaxDescriptionBytes = 100;
	
public:
	PlaylistHeader(const Brx& aFilename, const Brx& aName, const Brx& aDescription, const TUint aImageId);
	PlaylistHeader(const Brx& aFilename, IReader& aReader);
	
	virtual const Brx& Filename() const;
	virtual const Brx& Name() const;
	virtual const Brx& Description() const;
	virtual TUint ImageId() const;
	
	void ToXml(IWriter& aWriter) const;
	
private:
	Bws<Ascii::kMaxUintStringBytes> iFilename;
	Bws<kMaxNameBytes> iName;
	Bws<kMaxDescriptionBytes> iDescription;
	TUint iImageId;
};

class INameable
{
public:
	virtual ~INameable() {}
	
	virtual void SetName(const Brx& aValue) = 0;
};

	
class Track
{
public:
	static const TUint kMaxMetadataBytes = 4096;
	
public:
	Track(const TUint aId, const Brx& aUdn, const Brx& aMetadata);
	
	TUint Id() const;
	bool IsId(const TUint aId) const;
	
	const Brn& Udn() const;
	const Brn& Metadata() const;
	
private:
	const TUint iId;
	const Brn iUdn;
	const Brn iMetadata;
};

class PlaylistData : public IPlaylistData
{
public:
	static const TUint kMaxTracks = 1000;
	
public:
	PlaylistData(const TUint aId, const Brx& aFilename);
	~PlaylistData();
	
	bool IsId(const TUint aId) const;
	
	void IdArray(Bwx& aIdArray);
	
	const TUint Insert(const TUint aAfterId, const Brx& aUdn, const Brx& aMetadata);
	void Delete(const TUint aId);
	void DeleteAll();
	
	void ToXml(IWriter& aWriter) const;
	
private:
	const TUint iId;
	
	IdGenerator iIdGenerator;
	
	std::list<Track*> iTracks;
	Bws<kMaxTracks> iIdArray;
};


class ICacheListener
{
public:
	virtual void RemovedFromCache() = 0;
};


class Playlist;

class Cache
{
public:
	Cache();
	
	PlaylistData* Data(const Playlist& aPlaylist, ICacheListener* aCacheListener);
	
private:
	
	Mutex iMutex;
	
	std::list< std::pair<PlaylistData*, ICacheListener*> > iList;
};	


class Playlist : public IPlaylistHeader, public IPlaylistData, public ICacheListener
{
public:
	Playlist(Cache* aCache, const TUint aId, const Brx& aFilename, const Brx& aName, const Brx& aDescription, const TUint aImageId);
	Playlist(Cache* aCache, const TUint aId, const Brx& aFilename, IReader& aReader);
	
	const TUint Id() const;
	bool IsId(const TUint aId) const;
	
	const TUint Token() const;
	
	virtual const Brx& Filename() const;
	virtual const Brx& Name() const;
	virtual const Brx& Description() const;
	virtual TUint ImageId() const;
	
	virtual void IdArray(Bwx& aIdArray);
	
	virtual const TUint Insert(const TUint aAfterId, const Brx& aUdn, const Brx& aMetadata);
	virtual void Delete(const TUint aId);
	virtual void DeleteAll();
	
	void ToXml(IWriter& aWriter);
	
	virtual void RemovedFromCache();
	
private:
	mutable Mutex iMutex;
	
	const TUint iId;
	TUint iToken;
	
	Cache* iCache;
	PlaylistHeader iHeader;
	PlaylistData* iData;
};

	
	

class PlaylistManager : public INameable
{	
public:
	static const TUint kMaxNameBytes = 30;
	static const TUint kMaxImageBytes = 30 * 1024;
	static const TUint kMaxMimeTypeBytes = 100;
	static const TUint kMaxPlaylists = 500;
	
public:
    PlaylistManager(OpenHome::Net::DvDevice& aDevice, const TIpAddress& aAdapter, const Brx& aName, const Brx& aImage, const Brx& aMimeType);
	virtual ~PlaylistManager();
	
	virtual void SetName(const Brx& aValue);
	
	void SetAdapter(const TIpAddress& aAdapter);
	
	const Brx& Name() const;
	const TIpAddress& Adapter() const;
	
	const TUint Token() const;
	const TBool TokenChanged(const TUint aToken) const;
	
	void Metadata(Bwx& aMetadata) const;
	void IdArray(Bwx& aIdArray) const;
	void TokenArray(Bwx& aTokenArray) const;
	void PlaylistReadMetadata(std::vector<TUint>& aIdList, IWriter& aWriter) const;
	void PlaylistRead(const TUint aId, Bwx& aName, Bwx& aDescription, TUint& aImageId) const;
	const TUint PlaylistInsert(const TUint aAfterId, const Brx& aName, const Brx& aDescription, const TUint aImageId);
	void PlaylistDelete(const TUint aId);
	
	void IdArray(const TUint aId, Bwx& aIdArray);
	const TUint Insert(const TUint aId, const TUint aAfterId, const Brx& aUdn, const Brx& aMetadata);
	void Delete(const TUint aId, const TUint aTrackId);
	void DeleteAll(const TUint aId);

private:
	void WriteToc() const;
	void WritePlaylist(Playlist& aPlaylist) const;
	
	PlaylistData* CachePlaylist(const TUint aId);
	
	void UpdateArrays();
	
	mutable Mutex iMutex;
	
	OpenHome::Net::DvDevice& iDevice;
	
	IdGenerator iIdGenerator;
	Cache iCache;
	
	Bws<kMaxNameBytes> iName;
	TIpAddress iAdapter;
	Bws<kMaxImageBytes> iImage;
	Bws<kMaxMimeTypeBytes> iMimeType;
	
	std::list<Playlist*> iPlaylists;
	TUint iToken;
	
	OpenHome::Net::ProviderPlaylistManager* iProvider;
};
	


/*class PlaylistManagerSession : public SocketTcpSession
{
    static const TUint kMaxRequestBytes = 4*1024;
    static const TUint kMaxResponseBytes = 4*1024;
public:
    OhmSenderSession(const OhmSender& aSender);
    ~OhmSenderSession();
private:
    void Run();
    void Error(const HttpStatus& aStatus);
    void Get(TBool aWriteEntity);
private:
	const OhmSender& iSender;
    Srs<kMaxRequestBytes>* iReadBuffer;
    ReaderHttpRequest* iReaderRequest;
    Sws<kMaxResponseBytes>* iWriterBuffer;
    WriterHttpResponse* iWriterResponse;
    HttpHeaderHost iHeaderHost;
    HttpHeaderExpect iHeaderExpect;
	const HttpStatus* iErrorStatus;
    TBool iResponseStarted;
    TBool iResponseEnded;
    Semaphore iSemaphore;
};*/


} // namespace Media
} // namespace OpenHome

#endif // HEADER_PLAYLISTMANAGER

