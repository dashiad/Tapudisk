#include <SD.h>
#include <SPI.h>
#include "d64driver.h"
#include <math.h>

char readBuf[1024];

namespace {

void debugInt(char *str,unsigned long i)
{
  char buf[80];
  sprintf(buf,"%s %lu",str,i);
  Serial.println(buf);
}
void debugChar(char *str,char i)
{
  char buf[80];
  sprintf(buf,"%s %d",str,(unsigned int)i);
  Serial.println(buf);
}
#define D64_BLOCK_SIZE 256  // Actual block size
#define D64_BLOCK_DATA 254  // Data capacity of block

#define D64_FIRSTDIR_TRACK  18
#define D64_FIRSTDIR_SECTOR 1

#define D64_BAM_TRACK  18
#define D64_BAM_SECTOR 0

#define D64_BAM_DISKNAME_OFFSET 0x90

#define D64_IMAGE_SIZE 174848

typedef struct {
		uchar disk_name[16]; // disk name padded with A0
		uchar disk_id[5];    // disk id and dos type
} D64DiskInfo;

// Sectors p. track table
// sectors_track[3] is the number of sectors in track 4 (!)
//
const uchar sectorsPerTrack[40] = {
		21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,21,
		19,19,19,19,19,19,19,
		18,18,18,18,18,18,
		17,17,17,17,17,17,17,17,17,17
};

const char strFileTypes[6][4] = { "DEL", "SEQ", "PRG", "USR", "REL", "???" };



} // anonymous


D64::D64(const char * fileName)
		: m_status(NOT_READY),m_currentTrack(0), m_currentSector(0), m_currentOffset(0),
				m_currentLinkTrack(0), m_currentLinkSector(0)
{
  
    
    if(fileName)
      mount(fileName);
} // dtor


D64::~D64()
{
		unmount();
} // dtor


bool D64::mount(const char *fileName)
{		
    strcpy(m_d64Path,fileName);
		m_hostFile = SD.open(m_d64Path,FILE_READ);
    char buf[20];    
    if(!m_hostFile)
      Serial.println("ERROR AL ABRIR!");

        m_readBufStart=0;
        m_readBufSize=0;
        m_readOffset=0;
        if(m_hostFile!=false) {
            m_hostSize=m_hostFile.size();
      			m_status = IMAGE_OK;
        }
		return m_hostFile == false?false:true;

} // mountHostImage


void D64::unmount()
{
    if(m_hostFile!=false) 
    {
		m_hostFile.close();
		m_status = NOT_READY;
    }
} // unmountHostImage


// This function sets the filepointer to third byte in a block.
//
// It also reads in link to next block, which is what the two first bytes
// contains
//
void D64::seekBlock(uchar track, uchar sector)
{
		unsigned long absSector;
		unsigned long absOffset;
    debugChar("SEEKING TRACK ",track);
       debugChar("SEEKING SECTOR ",sector);
		uchar i;
		// Change 1 based track notion to 0 based
		track--;

		//   // Sanity check track value
		//   if (track > 39) {
		//     m_status = 0;
		//     return;
		//   }
		//
		//   // Sanity check sector value
		//   if (sector > pgm_read_byte_near(&(sectors_track[track]))) {
		//     m_status = 0;
		//     return;
		//   }

		// Calculate absolute sector number     
		absSector = sector;
		for(i = 0; i < track; i++)
				absSector += sectorsPerTrack[i];

		// Calculate absolute file offset
		absOffset = absSector * 256;
    debugInt("ABS SECTOR:",absSector);
    debugInt("ABS OFFSET:",absOffset);
		// Seek to that position if possible
    debugInt("hostSize:",m_hostSize);
    debugInt("File size:",m_hostFile.size());
		if(absOffset < m_hostSize) {
				hostSeek(absOffset);

				// Read in link to next block                
                
				m_currentLinkTrack = hostReadByte();
    
				m_currentLinkSector = hostReadByte();
    
				// We are done, update vars
				m_currentTrack = track;
				m_currentSector = sector;
				m_currentOffset = 2;
		}
		else // Track or sector value must have been invalid! Bad image
   {        
      Serial.println("OFFSET DEMASIADO GRANDE!?");
				m_status = NOT_READY;
   }
} // seekBlock


bool D64::isEOF(void) const
{
		return not(m_status & IMAGE_OK)||not(m_status & FILE_OPEN)
						or (m_status & FILE_EOF);
} // isEOF


// This function reads a character and updates file position to next
//
char D64::getc(void)
{
		uchar ret = 0;
		// Check status
		if(not isEOF()) {
				ret = hostReadByte();
				if(255 == m_currentOffset) {
						// We need to go to a new block, end of file?
						if(m_currentLinkTrack != 0) {
								// Seek the next block:
								seekBlock(m_currentLinkTrack, m_currentLinkSector);
						}
						else
								m_status |= FILE_EOF;

				}
				else
						m_currentOffset++;
		}
		return ret;
} // getc



bool D64::close(void)
{
		m_status &= IMAGE_OK;  // Clear all flags except disk ok

		return true;
} // fclose


bool D64::seekFirstDir(void)
{
		if(m_status & IMAGE_OK) {
				// Seek to first dir entry
				seekBlock(D64_FIRSTDIR_TRACK, D64_FIRSTDIR_SECTOR);

				// Set correct status
				m_status = IMAGE_OK | DIR_OPEN;

				return true;
		}
		return false;
} // seekFirstDir


bool D64::getDirEntry(DirEntry& dir)
{
		uchar i, j;
		Serial.println("Cargando Dir Entry");
    debugInt("offset::::",m_hostFile.position());

		// Check if correct status
		if(not ((m_status & IMAGE_OK)&&(m_status & DIR_OPEN)
										and not(m_status & DIR_EOF)))
				return false;

    dir.m_type=hostReadByte();
    char buf[10];
    m_currentOffset++;
    dir.m_track=hostReadByte();
    
    m_currentOffset++;
    dir.m_sector=hostReadByte();
    
    m_currentOffset++;
    for(i=0;i<16;i++)
      dir.m_name[i]=hostReadByte();
    dir.m_name[i]=0;
    
    
    m_currentOffset+=16;
    dir.m_sideTrack=hostReadByte();
    m_currentOffset++;
    dir.m_sideSector=hostReadByte();   // For REL files
    m_currentOffset++;
    dir.m_recordLength=hostReadByte();
    m_currentOffset++;
    for(int i=0;i<6;i++)
      dir.m_unused[i]=hostReadByte();    // Except for GEOS disks
    m_currentOffset+=6;
    dir.m_blocksLo=hostReadByte();     // 16-bit file size in blocks, multiply by
    m_currentOffset++;
    dir.m_blocksHi=hostReadByte();
    m_currentOffset++;

    
		

		// Have we crossed a block boundry?
		if(0 == m_currentOffset) {
				// We need to go to a new block, end of directory chain?
				if(m_currentLinkTrack != 0) {
						// Seek the next block:
						seekBlock(m_currentLinkTrack, m_currentLinkSector);
				}
				else
						m_status |= DIR_EOF;

		}
		else {
				// No boundry crossing, skip past two initial bytes of next dir
				i = hostReadByte();
				j = hostReadByte();
				m_currentOffset += 2;

				if(0 == i&&0xFF == j) {
						// No more dirs!
						m_status |= DIR_EOF;
				}
		}
    
		return true;
} // getDirEntry


bool D64::getDirEntryByName(D64::DirEntry &dir, const char *name)
{
		// Now for the list entries
		seekFirstDir();
		while(getDirEntry(dir)) {
				if(name == dir.name()) // found it?
						return true;
		}
		// no match.
		return false;
} // getDirEntry

bool D64::hostReadBuffer(long pos)
{
    
    m_readBufStart=pos;

    m_readBufSize=m_hostFile.read(readBuf,sizeof(readBuf));        
    m_readOffset=0;
	return true;
}

uchar D64::hostReadByte(uint length)
{
    
    if(m_readOffset==m_readBufSize) 
        hostReadBuffer(m_readOffset + m_readBufStart);

    if(m_readOffset==m_hostSize) 
    {
        m_status = FILE_EOF;
        return 0;
    }
    
    uchar c = readBuf[m_readOffset];
    m_readOffset++;
    return c;

} // hostReadByte


bool D64::hostSeek(long pos, bool relative)
{
    
    if(!relative) {    
        debugInt("SEEKING ABSOLUTE POS:",pos);
        if(pos < m_readBufStart || (m_readBufStart + m_readBufSize)< pos) {
            m_readBufStart=pos;
            m_readBufSize=0;
            m_readOffset=0;
            
            m_hostFile.seek(pos);
        }
        else
            m_readOffset=(pos-m_readBufStart);
    }
    else
    {		
      debugInt("SEEKING RELATIVE POS:",pos);
        m_readOffset+=pos;
        
        if(m_readOffset < m_readBufStart || m_readOffset > (m_readBufStart + m_readBufSize)) {
            m_readBufStart+=pos;
            m_readBufSize=0;
            m_readOffset=0;
            debugInt("Yendo a :",m_readBufStart);
            m_hostFile.seek(m_readBufStart);
        }         
    }
    
  		
	return true;
} // hostSeek


// At the position seeked comes:
//   16 chars of disk name (padded with A0)
//   2 chars of A0
//   5 chars of disk type
//
// Get these bytes directly from FAT by readHostByte();
void D64::seekToDiskName(void)
{
		if(m_status & IMAGE_OK) {
				// Seek BAM block
				seekBlock(D64_BAM_TRACK, D64_BAM_SECTOR);
        
				// Seek to disk name (-2 because seek_block already skips two bytes)
        debugInt("El offset al que va es ",D64_BAM_DISKNAME_OFFSET);
				hostSeek(D64_BAM_DISKNAME_OFFSET - 2, true);

				// Status now is no file open as such
				m_status = IMAGE_OK;
		}
} // seekToDiskName


ushort D64::blocksFree(void)
{
		// Not implemented yet
		return 0;
}

// Opens a file. Filename * will open first file with PRG status
//
bool D64::fopen(const char * fileName)
{
		bool found = false;
		uchar len;
		uchar i;

		len = strlen(fileName);
		if(len > sizeof(m_currDirEntry.m_name))
				len = sizeof(m_currDirEntry.m_name);

		seekFirstDir();

		while(not found&&getDirEntry(m_currDirEntry)) {

				// Acceptable filetype?
				i = m_currDirEntry.m_type & FILE_TYPE_MASK;
				if(SEQ == i||PRG == i) {

						// Compare filename respecting * and ? wildcards
						found = true;
						for(i = 0; i < len&&found; i++) {
								if('?' == fileName[i]) {
										// This character is ignored
								}
								else if('*' == fileName[i]) {
										// No need to check more chars
										break;
								}
								else
										found = fileName[i] == m_currDirEntry.m_name[i];
						}

						// If searched to end of filename, dir.file_name must end here also
						if(found&&(i == len))
								if(len < 16)
										found = m_currDirEntry.m_name[i] == 0xA0;

				}
		}

		if(found) {
				// File found. Jump to block and set correct state
				seekBlock(m_currDirEntry.track(), m_currDirEntry.sector());
				m_status = IMAGE_OK | FILE_OPEN;
				
		}		
		return found;
} // fopen


bool D64::sendListing(ISendLine& cb)
{
    debugInt("Iniciando listing con status ",m_status);
		if(not (m_status & IMAGE_OK)) {
				// We are not happy with the d64 file
       Serial.println("ERROR!");
       debugChar("status:",m_status);
				cb.send(0, "ERROR: D64",true);
				return true;
		}

		// Send line with disc name and stuff, 25 chars
		seekToDiskName();

		char line[25];
        line[0]=0x12;
        line[1]=0x22; // Invert face, "
            
		for(uchar i = 2; i < 25; i++) {
				uchar c = hostReadByte();        

				if(0xA0 == c) // Convert padding A0 to spaces
						c = ' ';

				if(18 == i)   // Ending "
						c = '"';

				line[i] = c;
		}
		cb.send(0, line,false);

		// Now for the list entries
		seekFirstDir();

		DirEntry dir;
		while(getDirEntry(dir)) {
        
				// Determine if dir entry is valid:
				if(dir.m_track != 0) {            
						// A direntry always takes 32 bytes total = 27 chars
						// Send filename until A0 or 16 chars
						char name[19];
						int i;
                        for(i=0;i<18;i++) 
                            name[i]=' ';
                        name[i]=0;
            name[0]='"';            						
						for(i = 1; i <= sizeof(dir.m_name); ++i) {
								uchar c = dir.m_name[i-1];
								if(0xA0 == c)
                                {
										break;  // Filename is no longer
                                }
								name[i] = c;
						}
						// Ending name with dbl quotes
						name[i] = '"';
                        name[i+1]=0;

						// Write filetype
						uchar fileType = dir.m_type & FILE_TYPE_MASK;
						if(fileType > NumD64FileTypes)
								fileType = NumD64FileTypes; // Limit to Unknown type (???) when out of range.

						// Prepare buffer
                        char buf[80];

                        sprintf(buf,"    %s %s%c%c",name,strFileTypes[fileType],
                                    ((dir.m_type & FILE_LOCKED) ? '<' : ' '), // Perhaps write locked symbol
                                    (not (dir.m_type & FILE_CLOSED) ? '*' : ' ')	// Perhaps write splat symbol
                        );
                        
                        char *po=buf;                       
						// Line number is filesize in blocks:
						ushort fileSize = dir.m_blocksLo + (dir.m_blocksHi << 8);
                        char fileSizeBuf[10];
                        sprintf(fileSizeBuf,"%d",fileSize);
                        // Se han metido espacios iniciales en buf.Ahora hay que eliminar espacios, segun
                        // el numero de caracteres que componga el filesize.
                        po+=strlen(fileSizeBuf);
						
						cb.send(fileSize, po,false);
				}
		}

		// Send line with 0 blocks free
        char blkFree[80];
        strcpy(blkFree,"BLOCKS FREE.");
        char *p=blkFree+strlen(blkFree);
        for(int j=0;j<13;j++,p++) 
            *p=' ';
        *p=0;		
		cb.send(0, blkFree,true);

		return true;
} // sendListing

D64::DirEntry::DirEntry()
{
} // ctor


D64::DirEntry::~DirEntry()
{
} // dtor


CBM::IOErrorMessage D64::newDisk(const char * name, const char * id)
{
//	return FileDriverBase::newDisk(name, id);
	return CBM::ErrOK;
} // newDisk


char *D64::DirEntry::name() const
{
	return NULL;
} // getName


D64::FileType D64::DirEntry::type() const
{
		return static_cast<FileType>(m_type);
} // type


ushort D64::DirEntry::numBlocks() const
{
		return (m_blocksHi << 8) | m_blocksLo;
} // getNumBlocks


ushort D64::DirEntry::sizeBytes() const
{
		return numBlocks() * D64_BLOCK_DATA;
} // getSizeBytes


uchar D64::DirEntry::track() const
{
		return m_track;
} // getTrack


uchar D64::DirEntry::sector() const
{
		return m_sector;
} // getSector

/**
 * sectorsPerTrack - number of sectors on given track
 * @track: Track number
 *
 * Returns the number of sectors on the given track
 * of a 1541/71/81 disk. Invalid track numbers will return invalid results.
 */
ushort D64::xxxsectorsPerTrack(uchar track)
{
		//Q_UNUSED(track);
		//	switch(m_header.imageType & IMAGE_TYPE_MASK)
		//	{
		//		case D81:
		//			return 40;

		//		case DNP:
		//			return 256;

		//		case D41:
		//		case D71:
		//		default:
		//			if(track > 35)
		//				track -= 35;
		//			if(track < 18)
		//				return 21;
		//			if(track < 25)
		//				return 19;
		//			if(track < 31)
		//				return 18;
		//			return 17;
		//	}
		return 0;
} // sectorsPerTrack


