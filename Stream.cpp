#include "Stream.h"

using namespace OpenHome;
using namespace OpenHome::Media;

ReaderFile::ReaderFile(const TChar* aFilename)
{
	iFile = fopen(aFilename, "rt");
	if(iFile == NULL)
	{
		THROW(ReaderFileError);
	}
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
	if(iFile == NULL)
	{
		THROW(ReaderFileError);
	}
	
	aBuffer.SetBytes(0);
	int count = fread((void*)aBuffer.Ptr(), 1, aBuffer.MaxBytes(), iFile);
	aBuffer.SetBytes(count);
	
	if(count < 1)
	{
		THROW(ReaderFileError);
	}
}

void ReaderFile::ReadFlush()
{
	if(iFile == NULL)
	{
		THROW(ReaderFileError);
	}
	
	fflush(iFile);
}

void ReaderFile::ReadInterrupt()
{
}
	
WriterFile::WriterFile(const TChar* aFilename)
{
	iFile = fopen(aFilename, "wt");
	
	if(iFile == NULL)
	{
		THROW(WriterFileError);
	}
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
	if(iFile == NULL)
	{
		THROW(WriterFileError);
	}
	
	fwrite(&aValue, 1, 1, iFile);
}

void WriterFile::Write(const Brx& aBuffer)
{
	if(iFile == NULL)
	{
		THROW(WriterFileError);
	}
	
	fwrite(aBuffer.Ptr(), 1, aBuffer.Bytes(), iFile);
}

void WriterFile::WriteFlush()
{
	if(iFile == NULL)
	{
		THROW(WriterFileError);
	}
	
	fflush(iFile);
}