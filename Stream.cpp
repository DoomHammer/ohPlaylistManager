#include "Stream.h"

using namespace OpenHome::Media;

ReaderFile::ReaderFile(const TChar* aFilename)
{
	iFile = fopen(aFilename, "rt");
}

ReaderFile::~ReaderFile()
{
	Close();
}

void ReaderFile::Close()
{
	if(iFile != NULL)
	{
		fclose(iFile);
		iFile = NULL;
	}
}

void ReaderFile::Read(Bwx& aBuffer)
{
	fread((void*)aBuffer.Ptr(), 1, aBuffer.Bytes(), iFile);
}

void ReaderFile::ReadFlush()
{
}

void ReaderFile::ReadInterrupt()
{
}
	
WriterFile::WriterFile(const TChar* aFilename)
{
	iFile = fopen(aFilename, "wt");
}

WriterFile::~WriterFile()
{
	Close();
}

void WriterFile::Close()
{
	if(iFile != NULL)
	{
		fclose(iFile);
		iFile = NULL;
	}
}

void WriterFile::Write(TByte aValue)
{
	fwrite(&aValue, 1, 1, iFile);
}

void WriterFile::Write(const Brx& aBuffer)
{
	fwrite(aBuffer.Ptr(), 1, aBuffer.Bytes(), iFile);
}

void WriterFile::WriteFlush()
{
	
}