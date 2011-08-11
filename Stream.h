#ifndef HEADER_PLAYLISTMANAGER_STREAM
#define HEADER_PLAYLISTMANAGER_STREAM

#include <stdio.h>

#include <OpenHome/Private/Stream.h>
#include <OpenHome/Buffer.h>

EXCEPTION(ReaderFileError);
EXCEPTION(WriterFileError);

namespace OpenHome {
namespace Media {

class ReaderFile : public IReaderSource
{
public:
	ReaderFile(const TChar* aFilename);
	virtual ~ReaderFile();
	
	void Close();
	
	virtual void Read(Bwx& aBuffer);
	virtual void ReadFlush();
	virtual void ReadInterrupt();
	
private:
	FILE* iFile;
};

class WriterFile : public IWriter
{
public:
	WriterFile(const TChar* aFilename);
	virtual ~WriterFile();
	
	void Close();
	
	virtual void Write(TByte aValue);
	virtual void Write(const Brx& aBuffer);
	virtual void WriteFlush();
	
private:
	FILE* iFile;
};

} // Media
} // OpenHome

#endif