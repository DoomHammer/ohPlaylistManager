#ifndef HEADER_PLAYLISTMANAGER
#define HEADER_PLAYLISTMANAGER

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Net/Core/DvDevice.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Timer.h>
#include <OpenHome/Private/Http.h>

#include "Ohm.h"

namespace OpenHome {
namespace Net {

class ProviderPlaylistManager;

class IPlaylistManagerPersist
{
public:
    virtual TUint PlaylistCount() = 0;
    virtual TUint MaxTrackCount() = 0;
    virtual TUint MaxPlaylistCount() = 0;
    virtual void Save(const Playlist& aPlaylist) = 0;
    virtual void Delete(const Playlist& aPlaylist) = 0;
    virtual ~IPlaylistManagerPersist() {}
};

class PlaylistManager
{
public:
	static const TUint kMaxNameBytes = 30;

public:
    PlaylistManager(DvDevice& aDevice, IPlaylistManagerPersist& aPersist, const Brx& aName, const Brx& aImage, const Brx& aMimeType);
	void SetName(const Brx& aValue);
    ~PlaylistManager();

private:
    DvDevice& iDevice;
    IPlaylistManagerPersist& iPersist;
    Bws<kMaxNameBytes> iName;
    ProviderPlaylistManager* iProvider;
};

class PlaylistManagerSession : public SocketTcpSession
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
};


} // namespace Net
} // namespace OpenHome

#endif // HEADER_PLAYLISTMANAGER

