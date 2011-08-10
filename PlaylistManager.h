#ifndef HEADER_PLAYLISTMANAGER
#define HEADER_PLAYLISTMANAGER

#include <list>

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

	
class PlaylistHeader
{
public:
	static const TUint kMaxNameBytes = 30;
	static const TUint kMaxDescriptionBytes = 100;
	
public:
	PlaylistHeader(const TUint aId, const Brx& aName, const Brx& aDescription, const TUint aImageId);
	
	TUint Id() const;
	bool IsId(TUint aId) const;
	
	const Brx& Name() const;
	const Brx& Description() const;
	TUint ImageId() const;
	
private:
	TUint iId;
	
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

class PlaylistManager : public INameable
{
public:
	static const TUint kMaxNameBytes = 30;
	static const TUint kMaxImageBytes = 30 * 1024;
	static const TUint kMaxMimeTypeBytes = 100;
	static const TUint kMaxTracks = 1000;

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
	const TBool TokenChanged(const TUint aValue) const;
	
	const PlaylistHeader* Header(TUint aId) const;
	
	const TUint Insert(const TUint aAfterId, const Brx& aName, const Brx& aDescription, const TUint aImageId);
	const TUint NewId();

private:
	OpenHome::Net::DvDevice& iDevice;
	
	Bws<kMaxNameBytes> iName;
	TIpAddress iAdapter;
	Bws<kMaxImageBytes> iImage;
	Bws<kMaxMimeTypeBytes> iMimeType;
	
	std::list<PlaylistHeader*> iPlaylistHeaders;
	TUint iToken;
	Bwh iIdArray;
	Bwh iTokenArray;
	
	TUint iNextId;
	
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

