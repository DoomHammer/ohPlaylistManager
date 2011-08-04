#ifndef HEADER_PLAYLISTMANAGER
#define HEADER_PLAYLISTMANAGER

#include <OpenHome/Buffer.h>
#include <OpenHome/Net/Core/DvDevice.h>

namespace OpenHome {
namespace Net {
	class ProviderPlaylistManager;
}
}

namespace OpenHome {
namespace Media {
	
class Playlist;

class IPlaylistManagerPersist
{
public:
	virtual ~IPlaylistManagerPersist() {}
	
    virtual TUint PlaylistCount() const = 0;
    virtual TUint MaxTrackCount() const = 0;
    virtual TUint MaxPlaylistCount() const = 0;
    virtual void Save(const Playlist& aPlaylist) = 0;
    virtual void Delete(const Playlist& aPlaylist) = 0;
};
	
class PlaylistManagerPersistFs : public IPlaylistManagerPersist
{
public:
	PlaylistManagerPersistFs();
	virtual ~PlaylistManagerPersistFs() {}
	
	virtual TUint PlaylistCount() const;
    virtual TUint MaxTrackCount() const;
    virtual TUint MaxPlaylistCount() const;
    virtual void Save(const Playlist& aPlaylist);
    virtual void Delete(const Playlist& aPlaylist);
};

class INameable
{
public:
	virtual ~INameable() {}
	
	virtual void SetName(const Brx& aValue) = 0;
};

class PlaylistManager : public INameable
{
public:
	static const TUint kMaxNameBytes = 30;
	static const TUint kMaxImageBytes = 30 * 1024;
	static const TUint kMaxMimeTypeBytes = 100;

public:
    PlaylistManager(OpenHome::Net::DvDevice& aDevice, IPlaylistManagerPersist& aPersist, const TIpAddress& aAdapter, const Brx& aName, const Brx& aImage, const Brx& aMimeType);
	virtual ~PlaylistManager();
	
	virtual void SetName(const Brx& aValue);
	
	void SetAdapter(const TIpAddress& aAdapter);
private:
	OpenHome::Net::DvDevice& iDevice;
    IPlaylistManagerPersist& iPersist;
    Bws<kMaxNameBytes> iName;
	Bws<kMaxImageBytes> iImage;
	Bws<kMaxMimeTypeBytes> iMimeType;
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

