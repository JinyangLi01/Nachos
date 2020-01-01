// filehdr.cc 
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the 
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the 
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect 
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector, 
//
//      Unlike in a real system, we do not keep track of file permissions, 
//	ownership, last modification date, etc., in the file header. 
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "system.h"
#include "filehdr.h"
#include <time.h>
#include <string.h>

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the bit map of free disk sectors
//----------------------------------------------------------------------

bool
FileHeader::Allocate(BitMap *freeMap, int fileSize)
{ 
    numBytes = fileSize;
    numSectors  = divRoundUp(fileSize, SectorSize);
    
    int need = 0;
    if (numSectors < NumDirect)
    	need = numSectors;
    else 
    	need = numSectors + 1;
    
    if (freeMap->NumClear() < need)
	return FALSE;		// not enough space
    
    if (numSectors < NumDirect) {
    	for (int i = 0; i < numSectors; ++i)
	    dataSectors[i] = freeMap->Find();
    } else {
    	for (int i = 0; i < NumDirect; ++i)
    	    dataSectors[i] = freeMap->Find();
    	int *temp = new int[SectorSize / sizeof(int)];
    	for (int i = NumDirect - 1; i < numSectors; ++i)
    	    temp[i - NumDirect + 1] = freeMap->Find();
    	
    	synchDisk->WriteSector(dataSectors[NumDirect - 1], (char *)temp); 
    	 
    }
    
    return TRUE;
   
   /* 
    if (freeMap->NumClear() < numSectors)
	return FALSE;		// not enough space

    for (int i = 0; i < numSectors; i++)
	dataSectors[i] = freeMap->Find();
    return TRUE;
    */
}

//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------

void 
FileHeader::Deallocate(BitMap *freeMap)
{
    printf("Here is FileHeader::Deallocate()\n");
    if (numSectors < NumDirect) {
    	for (int i = 0; i < numSectors; i++) {
	    ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	    freeMap->Clear((int) dataSectors[i]);
    	}
    } else {
    	int *temp = new int[SectorSize / sizeof(int)];
    	synchDisk->ReadSector(dataSectors[NumDirect - 1], (char *)temp);
    	for (int i = NumDirect - 1; i < numSectors; ++i) {
	    ASSERT(freeMap->Test((int) temp[i - NumDirect + 1]));  // ought to be marked!
	    freeMap->Clear((int) temp[i - NumDirect + 1]);
    	}    	
    	for (int i = 0; i < NumDirect; i++) {
	    ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	    freeMap->Clear((int) dataSectors[i]);
    	} 
    }
    /*	
    for (int i = 0; i < numSectors; i++) {
	ASSERT(freeMap->Test((int) dataSectors[i]));  // ought to be marked!
	freeMap->Clear((int) dataSectors[i]);
    }
    */
}

//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk. 
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void
FileHeader::FetchFrom(int sector)
{

    printf("Here is FileHeader::FetchFrom, sector = %d\n", sector);
    synchDisk->ReadSector(sector, (char *)this);
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk. 
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void
FileHeader::WriteBack(int sector)
{
    synchDisk->WriteSector(sector, (char *)this); 
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

int
FileHeader::ByteToSector(int offset)
{
    printf("Here is FileHeader::ByteToSector\n");
    int sector = offset / SectorSize;
    if (sector < NumDirect - 1)
    	return(dataSectors[sector]);
    else {
    	int *temp = new int[SectorSize / sizeof(int)];
    	synchDisk->ReadSector(dataSectors[NumDirect - 1], (char *)temp);
    	return temp[sector - NumDirect + 1];
    }
}

//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int
FileHeader::FileLength()
{
    return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void
FileHeader::Print()
{
    printf("Here is FileHeader::Print()\n");
    int i, j, k;
    char *data = new char[SectorSize];
    
    
    int *temp = new int[SectorSize / sizeof(int)];
    synchDisk->ReadSector(dataSectors[NumDirect - 1], (char *)temp);
    
    printf("FileHeader contents:\nFile size: %d\nSector Num:%d\n", numBytes, numSectors);
    
    printf("File Type: %s\nFile Created Time: %s\n", type, createTime);
    printf("Last Visited Time: %s\nLast Modified Time: %s\n", lastVisitTime, lastModifyTime);

    printf("File blocks:\n", numBytes);
    if (numSectors < NumDirect) {
    	for (i = 0; i < numSectors; i++)
	    printf("%d ", dataSectors[i]);
    } else {
    	for (i = 0; i < NumDirect - 1; i++)
	    printf("%d ", dataSectors[i]);
	for (i = NumDirect - 1; i < numSectors; i++)
	    printf("%d ", temp[i - NumDirect + 1]);
    }
	
    printf("\nFile contents:\n");
    
    for (i = k = 0; i < numSectors; i++) {
    	if (i < NumDirect - 1)
	    synchDisk->ReadSector(dataSectors[i], data);
        else 
	    synchDisk->ReadSector(temp[i - NumDirect + 1], data);
        	
        for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++) {
	    if ('\040' <= data[j] && data[j] <= '\176')   // isprint(data[j])
		printf("%c", data[j]);
            else
		printf("\\%x", (unsigned char)data[j]);
	}
        printf("\n"); 
    }
    delete [] data;
}


void 
FileHeader::setType(char *name) {
    int i, j = 0;
    int len = strlen(name);
    for (i = 0; i < len; ++i) 
    	if (name[i] == '.')
	    break;
    for (i = i + 1; i < len && j < 4; ++i, ++j)
        type[j] = name[i];
    type[j] = 0;
}

void
FileHeader::setTime(int ty) {
    time_t timep;
    time(&timep);
    if (ty == 0) {
    	strncpy(createTime, asctime(gmtime(&timep)), 25);
    	createTime[24] = 0;
    } else if (ty == 1) {
    	strncpy(lastVisitTime, asctime(gmtime(&timep)), 25);
    	lastVisitTime[24] = 0;
    } else {
    	strncpy(lastModifyTime, asctime(gmtime(&timep)), 25);
    	lastModifyTime[24] = 0;
    }
}


bool 
FileHeader::Extend(BitMap *freeMap, int length) {
    printf("Here is FileHeader::Extend\n");
    int len = numBytes + length, sector = divRoundUp(len, SectorSize);
    int need;  
    
    if (sector == numSectors) {
    	numBytes = len;
    	return TRUE;
    }
    
    if (sector > 40)
    	return FALSE;
    	
    if (sector < NumDirect && numSectors < NumDirect || 
    	(sector >= NumDirect && numSectors >= NumDirect)) {
    	need = sector - numSectors;
    } else {
    	need = sector - numSectors + 1;
    }
    
    if (freeMap->NumClear() < need)
	return FALSE;		// not enough space
	
    printf("extend %d sectors.\n", sector - numSectors);
    
    if (sector < NumDirect) {
    	for (int i = numSectors; i < sector; ++i)
	    dataSectors[i] = freeMap->Find();
    } else if (numSectors < NumDirect && sector >= NumDirect) {
    	int *temp = new int[SectorSize / sizeof(int)];
   
    	for (int i = numSectors; i < NumDirect; ++i)
	    dataSectors[i] = freeMap->Find();
	for (int i = NumDirect - 1; i < sector; ++i) 
    	    temp[i - NumDirect + 1] = freeMap->Find();
    	
    	synchDisk->WriteSector(dataSectors[NumDirect - 1], (char *)temp); 
    } else {
    	int *temp = new int[SectorSize / sizeof(int)];
    	
    	synchDisk->ReadSector(dataSectors[NumDirect - 1], (char *)temp); 
    	
    	for (int i = numSectors; i < sector; ++i) 
    		temp[i - NumDirect + 1] = freeMap->Find();
    		
    		
    	synchDisk->WriteSector(dataSectors[NumDirect - 1], (char *)temp); 
    }
    
    numBytes = len;
    numSectors = sector;
    return TRUE;
    
}
