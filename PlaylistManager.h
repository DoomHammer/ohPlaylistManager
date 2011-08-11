#ifndef HEADER_PLAYLISTMANAGER
#define HEADER_PLAYLISTMANAGER

#include <list>

#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Net/Core/DvDevice.h>

EXCEPTION(PlaylistManagerError);

namespace OpenHome {
namespace Net {
	class ProviderPlaylistManager;
}
}

namespace OpenHome {
namespace Media {
	
class IdGenerator
{
public:
	IdGenerator();
	IdGenerator(TUint aNextId);
	
	TUint NewId();
	
private:
	TUint iNextId;
};
		
class PlaylistHeader
{
public:
	static const TUint kMaxNameBytes = 30;
	static const TUint kMaxDescriptionBytes = 100;
	
public:
	PlaylistHeader(const TUint aId, const Brx& aName, const Brx& aDescription, const TUint aImageId);
	PlaylistHeader(const TUint aId, IReader& aReader);
	
	TUint Id() const;
	bool IsId(const TUint aId) const;
	
	TUint Token() const;
	void Modified();
	
	const Brx& Name() const;
	const Brx& Description() const;
	TUint ImageId() const;
	
	void ToXml(IWriter& aWriter) const;
	
private:
	const TUint iId;
	TUint iToken;
	
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
	Track(const TUint aId, const Brn& aUdn, const Brn& aMetadata);
	
	TUint Id() const;
	bool IsId(const TUint aId) const;
	
	const Brn& Udn() const;
	const Brn& Metadata() const;
	
	void ToXml(IWriter& aWriter) const;
	
private:
	const TUint iId;
	const Brn iUdn;
	const Brn iMetadata;
};

class Playlist
{
public:
	static const TUint kMaxTracks = 1000;

public:
	Playlist(const IdGenerator& aIdGenerator, const PlaylistHeader& aHeader);
	~Playlist();
	
	TUint Id() const;
	bool IsId(const TUint aId) const;
	
	const Brx& IdArray() const;
	
	const Track* GetTrack(const TUint aId) const;
	
	void Insert(const Brn& aUdn, const Brn& aMetadata);
	void Delete(const TUint aId);
	void DeleteAll();
	
	void ToXml(IWriter& aWriter) const;
	
private:
	IdGenerator iIdGenerator;
	
	const TUint iId;
	
	std::list<Track*> iTracks;
	Bws<kMaxTracks> iIdArray;
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
	const Brx& IdArray() const;
	const Brx& TokenArray() const;
	const TBool TokenChanged(const TUint aToken) const;
	
	const PlaylistHeader* Header(const TUint aId) const;
	const Playlist* GetPlaylist(const TUint aId);
	
	const TUint Insert(const TUint aAfterId, const Brx& aName, const Brx& aDescription, const TUint aImageId);
	void Delete(const TUint aId);

private:
	void WriteToc() const;
	void WritePlaylist(const PlaylistHeader& aHeader) const;
	void WritePlaylist(const PlaylistHeader& aHeader, const Playlist& aPlaylist) const;
	
	void UpdateArrays();
	
	mutable Mutex iMutex;
	
	OpenHome::Net::DvDevice& iDevice;
	
	IdGenerator iIdGenerator;
	
	Bws<kMaxNameBytes> iName;
	TIpAddress iAdapter;
	Bws<kMaxImageBytes> iImage;
	Bws<kMaxMimeTypeBytes> iMimeType;
	
	std::list<PlaylistHeader*> iPlaylistHeaders;
	std::list<Playlist*> iPlaylists;
	TUint iToken;
	Bws<kMaxPlaylists> iIdArray;
	Bws<kMaxPlaylists> iTokenArray;
	
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

