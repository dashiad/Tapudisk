#ifndef D64DRIVER_H
#define D64DRIVER_H

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#endif
#include "cbmdefines.h"
#include <SPI.h>
#include <SD.h>
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;



#define NOT_READY  0					// driver not ready (host file not read or accepted).
#define IMAGE_OK  1		// The open file in fat driver is accepted as a valid image (of the specific file system type).
#define FILE_OPEN  2		// A file is open right now
#define FILE_EOF  4		// The open file is ended
#define DIR_OPEN  8		// A directory entry is active
#define DIR_EOF  16		// Last directory entry has been retrieved

class ISendLine
{
public:
	virtual void send(short lineNo, const char *text,bool isLast) = 0;
};


class D64 
{
public:
	enum FileType
	{
		DEL         = 0,
		SEQ         = 1,
		PRG         = 2,
		USR         = 3,
		REL         = 4,
		NumD64FileTypes,
		FILE_TYPE_MASK   = 0x07,
		FILE_LOCKED = 0x40,
		FILE_CLOSED = 0x80
	};
	
	
	


	// Disk image types - values must match G-P partition type byte
	enum ImageType
	{
		NONE = 0,
		DNP = 1,
		D41 = 2,
		D71 = 3,
		D81 = 4,
		NumD64ImageTypes,
		HAS_ERRORINFO = (1 << 7),
		IMAGE_TYPE_MASK = 0x7f
	};

	// Host file system D64-image can be opened from constructor, if specified.
	D64(const char *fileName=0);
	virtual ~D64();
	
	// The host file system D64 image is opened or re-opened with the method below.
	// If cannot be opened or not a D64 image, it returns false, true otherwise.
	bool mount(const char *fileName);
	void unmount();
  
	// Open a file in the image by filename: Returns true if successful
	bool fopen(const char *fileName);
	// return the name of the last opened file (may not be same as fopen in case it resulted in something else, like when using wildcards).
	char *openedFileName() const;
	// return the file size of the last opened file.
	ushort openedFileSize() const;
	// Get character from open file:
	char getc(void);
	// Returns true if last character was retrieved:
	bool isEOF(void) const;
	// Close current file
	bool close(void);
	// Blocks free information
	ushort blocksFree(void);

	bool sendListing(ISendLine& cb);
  byte getStatus(){ return m_status;}

	class ImageHeader
	{
		public:
		// Note: This is a POD class, may NOT contain any virtual methods or additional data than the DirType struct.

		ImageHeader();
		~ImageHeader();

		// Offsets in a D64 directory entry, also needed for raw dirs
#define DIR_OFS_FILE_TYPE       2
#define DIR_OFS_TRACK           3
#define DIR_OFS_SECTOR          4
#define DIR_OFS_FILE_NAME       5
#define DIR_OFS_SIZE_LOW        0x1e
#define DIR_OFS_SIZE_HI         0x1f
	};


	class DirEntry
	{
		public:
		// Note: This is a POD class, may NOT contain any virtual methods or additional data than the DirType struct.

		DirEntry();
		~DirEntry();
		char *name() const;
		FileType type() const;
		ushort sizeBytes() const;
		ushort numBlocks() const;
		uchar track() const;
		uchar sector() const;
    
		// uchar reserved1[2];  // track/sector of next direntry.
		// Note: only the very first direntry of a sector has this 'reserved' field.
		uchar m_type; // D64FileType
		uchar m_track;
		uchar m_sector;
		uchar m_name[17];
		uchar m_sideTrack;    // For REL files
		uchar m_sideSector;   // For REL files
		uchar m_recordLength; // For REL files
		uchar m_unused[6];    // Except for GEOS disks
		uchar m_blocksLo;     // 16-bit file size in blocks, multiply by
		uchar m_blocksHi;     // D64_BLOCK_DATA to get bytes
	}; // total of 30 bytes
	// special commands.
	CBM::IOErrorMessage newDisk(const char *name, const char *id);

private:
    
	uchar hostReadByte(uint length = 1);
	bool hostSeek(long pos, bool relative = false);
    bool hostReadBuffer(long pos);
	uint hostSize() const
	{
		return m_hostSize;
	}

	ushort xxxsectorsPerTrack(uchar track);
	void seekBlock(uchar track, uchar sector);
	bool seekFirstDir(void);
	bool getDirEntry(DirEntry& dir);
	bool getDirEntryByName(DirEntry& dir, const char *name);
	void seekToDiskName(void);
	
	// The real host file system D64 file:

	File m_hostFile;

	

	// D64 driver state variables:
	// The current d64 file position described as track/sector/offset
	uchar m_currentTrack;
	uchar m_currentSector;
	uchar m_currentOffset;

	// The current block's link to next block
	uchar m_currentLinkTrack;
	uchar m_currentLinkSector;
	DirEntry m_currDirEntry;
	
    char m_d64Path[255];    
    long m_readBufStart;
    long m_readOffset;
    long m_readBufSize;
    int m_hostSize;
	byte m_status;
};

#endif


