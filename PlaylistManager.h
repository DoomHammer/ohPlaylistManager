#ifndef HEADER_PLAYLISTMANAGER
#define HEADER_PLAYLISTMANAGER

#include <list>
#include <utility>

#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Net/Core/DvDevice.h>
#include <OpenHome/Net/Core/DvAvOpenhomeOrgPlaylistManager1.h>

EXCEPTION(PlaylistManagerError);
EXCEPTION(PlaylistError);
EXCEPTION(PlaylistFull);

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
	
	
	
class Metadata
{
public:
	static void Condense(const Brx& aIn, Bwx& aOut);
};
	
	
	
class IPlaylistHeader
{
public:
	virtual const Brx& Filename() const = 0;
	virtual void Name(Bwx& aName) const = 0;
	virtual void Description(Bwx& aDescription) const = 0;
	virtual void ImageId(TUint& aImageId) const = 0;
	
	virtual void SetName(const Brx& aName) = 0;
	virtual void SetDescription(const Brx& aDescription) = 0;
	virtual void SetImageId(const TUint& aImageId) = 0;
};



class IPlaylistData
{
public:
	virtual void IdArray(Bwx& aIdArray) = 0;
	
	virtual void Read(const TUint aTrackId, Bwx& aMetadata) = 0;
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
	virtual void Name(Bwx& aName) const;
	virtual void Description(Bwx& aDescription) const;
	virtual void ImageId(TUint& aImageId) const;
	
	virtual void SetName(const Brx& aName);
	virtual void SetDescription(const Brx& aDescription);
	virtual void SetImageId(const TUint& aImageId);
	
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
	static const TUint kMaxUdnBytes = 1024;
	static const TUint kMaxMetadataBytes = 4096;
	
public:
	Track(const TUint aId, const Brx& aUdn, const Brx& aMetadata);
	
	TUint Id() const;
	bool IsId(const TUint aId) const;
	
	const Brx& Udn() const;
	const Brx& Metadata() const;
	
private:
	const TUint iId;
	const Bws<kMaxUdnBytes> iUdn;
	const Bws<kMaxMetadataBytes> iMetadata;
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
	
	virtual void Read(const TUint aTrackId, Bwx& aMetadata);
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
	static const TUint kMaxCacheSize = 1000;
	
public:
	Cache();
	
	PlaylistData& Data(const Playlist& aPlaylist, ICacheListener* aCacheListener);
	
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
	virtual void Name(Bwx& aName) const;
	virtual void Description(Bwx& aDescription) const;
	virtual void ImageId(TUint& aImageId) const;
	
	virtual void SetName(const Brx& aName);
	virtual void SetDescription(const Brx& aDescription);
	virtual void SetImageId(const TUint& aImageId);
	
	virtual void IdArray(Bwx& aIdArray);
	
	virtual void Read(const TUint aTrackId, Bwx& aMetadata);
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

	
	

class PlaylistManager : public INameable, public IPlaylistManagerListener
{	
public:
	static const TUint kMaxNameBytes = 30;
	static const TUint kMaxImageBytes = 30 * 1024;
	static const TUint kMaxMimeTypeBytes = 100;
	static const TUint kMaxMetadataBytes = 1024;
	static const TUint kMaxPlaylists = 500;
	
public:
    PlaylistManager(OpenHome::Net::DvDevice& aDevice, const TIpAddress& aAdapter, const Brx& aName, const Brx& aImage, const Brx& aMimeType);
	virtual ~PlaylistManager();
	
	void SetListener(IPlaylistManagerListener& aListener);
	
	virtual void MetadataChanged();
	virtual void PlaylistsChanged();
	virtual void PlaylistChanged();
	
	virtual void SetName(const Brx& aValue);
	
	void SetAdapter(const TIpAddress& aAdapter);
	
	const Brx& Name() const;
	const TIpAddress& Adapter() const;
	
	const TUint Token() const;
	const TBool TokenChanged(const TUint aToken) const;
	
	void Metadata(Bwx& aMetadata) const;
	void IdArray(Bwx& aIdArray) const;
	void TokenArray(Bwx& aTokenArray) const;
	void PlaylistReadList(std::vector<TUint>& aIdList, IWriter& aWriter) const;
	void PlaylistRead(const TUint aId, Bwx& aName, Bwx& aDescription, TUint& aImageId) const;
	void PlaylistSetName(const TUint aId, const Brx& aName);
	void PlaylistSetDescription(const TUint aId, const Brx& aDescription);
	void PlaylistSetImageId(const TUint aId, TUint& aImageId);
	const TUint PlaylistInsert(const TUint aAfterId, const Brx& aName, const Brx& aDescription, const TUint aImageId);
	void PlaylistDelete(const TUint aId);
	void PlaylistMove(const TUint aId, const TUint aAfterId);
	
	void IdArray(const TUint aId, Bwx& aIdArray);
	
	void Read(const TUint aId, const TUint aTrackId, Bwx& aMetadata);
	const TUint Insert(const TUint aId, const TUint aAfterId, const Brx& aUdn, const Brx& aMetadata);
	void Delete(const TUint aId, const TUint aTrackId);
	void DeleteAll(const TUint aId);

private:
	void WriteToc() const;
	void WritePlaylist(Playlist& aPlaylist) const;
	
	mutable Mutex iMutex;
	
	OpenHome::Net::DvDevice& iDevice;
	IPlaylistManagerListener* iListener;
	
	IdGenerator iIdGenerator;
	Cache iCache;
	
	Bws<kMaxNameBytes> iName;
	TIpAddress iAdapter;
	Bws<kMaxImageBytes> iImage;
	Bws<kMaxMimeTypeBytes> iMimeType;
	
	std::list<Playlist*> iPlaylists;
	TUint iToken;
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

namespace OpenHome {
namespace Net {
	
	class ProviderPlaylistManager : public DvProviderAvOpenhomeOrgPlaylistManager1, public OpenHome::Media::IPlaylistManagerListener
	{
		static const TUint kMaxNameBytes = 60;
		
	public:
		ProviderPlaylistManager(DvDevice& aDevice, OpenHome::Media::PlaylistManager& aPlaylistManager, const TUint aMaxPlaylistCount, const TUint aMaxTrackCount);
		~ProviderPlaylistManager() {}
		
		virtual void MetadataChanged();
		virtual void PlaylistsChanged();
		virtual void PlaylistChanged();
		
	private:
		virtual void Metadata(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aMetadata);
		virtual void ImagesXml(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseString& aImagesXml);
		virtual void PlaylistReadArray(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseBinary& aArray);
		virtual void PlaylistReadList(IInvocationResponse& aResponse, TUint aVersion, const Brx& aIdList, IInvocationResponseString& aPlaylistList);
		virtual void PlaylistRead(IInvocationResponse& aResponse, TUint aVersion, TUint aId, IInvocationResponseString& aName, IInvocationResponseString& aDescription, IInvocationResponseUint& aImageId);
		virtual void PlaylistSetName(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aName);
		virtual void PlaylistSetDescription(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aDescription);
		virtual void PlaylistSetImageId(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aImageId);
		virtual void PlaylistInsert(IInvocationResponse& aResponse, TUint aVersion, TUint aAfterId, const Brx& aName, const Brx& aDescription, TUint aImageId, IInvocationResponseUint& aNewId);
		virtual void PlaylistDeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aValue);
		virtual void PlaylistMove(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aAfterId);
		virtual void PlaylistsMax(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aValue);
		virtual void TracksMax(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aValue);
		virtual void PlaylistArrays(IInvocationResponse& aResponse, TUint aVersion, IInvocationResponseUint& aToken, IInvocationResponseBinary& aIdArray, IInvocationResponseBinary& aTokenArray);
		virtual void PlaylistArraysChanged(IInvocationResponse& aResponse, TUint aVersion, TUint aToken, IInvocationResponseBool& aValue);
		virtual void Read(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aTrackId, IInvocationResponseString& aMetadata);
		virtual void ReadList(IInvocationResponse& aResponse, TUint aVersion, TUint aId, const Brx& aTrackIdList, IInvocationResponseString& aTrackList);
		virtual void Insert(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aAfterTrackId, const Brx& aUdn, const Brx& aMetadataId, IInvocationResponseUint& aNewTrackId);
		virtual void DeleteId(IInvocationResponse& aResponse, TUint aVersion, TUint aId, TUint aTrackId);
		virtual void DeleteAll(IInvocationResponse& aResponse, TUint aVersion, TUint aTrackId);
		
		void UpdateMetadata();
		void UpdateIdArray();
		void UpdateTokenArray();
		void UpdateArrays();
		
		OpenHome::Media::PlaylistManager& iPlaylistManager;
	};
	
} // namespace Net
} // namespace OpenHome


#endif // HEADER_PLAYLISTMANAGER

