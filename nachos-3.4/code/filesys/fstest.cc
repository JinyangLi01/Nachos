// fstest.cc 
//	Simple test routines for the file system.  
//
//	We implement:
//	   Copy -- copy a file from UNIX to Nachos
//	   Print -- cat the contents of a Nachos file 
//	   Perftest -- a stress test for the Nachos file system
//		read and write a really large file in tiny chunks
//		(won't work on baseline system!)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "utility.h"
#include "filesys.h"
#include "system.h"
#include "thread.h"
#include "disk.h"
#include "stats.h"

#define TransferSize 	10 	// make it small, just to be difficult

void 
ListDirectory(char *name) {
    fileSystem->ListDir(name);
}

void 
MakeDirectory(char *name) {
    if (!fileSystem->Create(name, -1)) {	 
	printf("Couldn't create directory %s\n", name);
	return;
    }
}

//----------------------------------------------------------------------
// Copy
// 	Copy the contents of the UNIX file "from" to the Nachos file "to"
//----------------------------------------------------------------------

void
Copy(char *from, char *to)
{
    FILE *fp;
    OpenFile* openFile;
    int amountRead, fileLength;
    char *buffer;

// Open UNIX file
    if ((fp = fopen(from, "r")) == NULL) {	 
	printf("Copy: couldn't open input file %s\n", from);
	return;
    }

// Figure out length of UNIX file
    fseek(fp, 0, 2);		
    fileLength = ftell(fp);
    fseek(fp, 0, 0);

// Create a Nachos file of the same length
    DEBUG('f', "Copying file %s, size %d, to file %s\n", from, fileLength, to);
    if (!fileSystem->Create(to, fileLength)) {	 // Create Nachos file
	printf("Copy: couldn't create output file %s\n", to);
	fclose(fp);
	return;
    }
    
    openFile = fileSystem->Open(to);
    ASSERT(openFile != NULL);
    
// Copy the data in TransferSize chunks
    buffer = new char[TransferSize];
    while ((amountRead = fread(buffer, sizeof(char), TransferSize, fp)) > 0)
	openFile->Write(buffer, amountRead);	
    delete [] buffer;

// Close the UNIX and the Nachos files
    delete openFile;
    fclose(fp);
}

//----------------------------------------------------------------------
// Print
// 	Print the contents of the Nachos file "name".
//----------------------------------------------------------------------

void
Print(char *name)
{
    OpenFile *openFile;    
    int i, amountRead;
    char *buffer;

    if ((openFile = fileSystem->Open(name)) == NULL) {
	printf("Print: unable to open file %s\n", name);
	return;
    }
    
    buffer = new char[TransferSize];
    while ((amountRead = openFile->Read(buffer, TransferSize)) > 0)
	for (i = 0; i < amountRead; i++)
	    printf("%c", buffer[i]);
    delete [] buffer;

    delete openFile;		// close the Nachos file
    return;
}

//----------------------------------------------------------------------
// PerformanceTest
// 	Stress the Nachos file system by creating a large file, writing
//	it out a bit at a time, reading it back a bit at a time, and then
//	deleting the file.
//
//	Implemented as three separate routines:
//	  FileWrite -- write the file
//	  FileRead -- read the file
//	  PerformanceTest -- overall control, and print out performance #'s
//----------------------------------------------------------------------

#define FileName 	"TestFile"
#define Contents 	"1234567890"
#define ContentSize 	strlen(Contents)
#define FileSize 	((int)(ContentSize * 5000))

static void 
FileWrite()
{
    OpenFile *openFile;    
    int i, numBytes;

    printf("Sequential write of %d byte file, in %d byte chunks\n", 
	FileSize, ContentSize);
    if (!fileSystem->Create(FileName, 0)) {
      printf("Perf test: can't create %s\n", FileName);
      //return;
    }
    openFile = fileSystem->Open(FileName);
    if (openFile == NULL) {
	printf("Perf test: unable to open %s\n", FileName);
	return;
    }
    for (i = 0; i < FileSize; i += ContentSize) {
        numBytes = openFile->Write(Contents, ContentSize);
        printf("write %d bytes\n", i);
	if (numBytes < 10) {
	    printf("Perf test: unable to write %s\n", FileName);
	    delete openFile;
	    return;
	}
    }
    delete openFile;	// close file
}

static void 
FileRead()
{
    OpenFile *openFile;    
    char *buffer = new char[ContentSize];
    int i, numBytes;

    printf("Sequential read of %d byte file, in %d byte chunks\n", 
	FileSize, ContentSize);

    if ((openFile = fileSystem->Open(FileName)) == NULL) {
	printf("Perf test: unable to open file %s\n", FileName);
	delete [] buffer;
	return;
    }
    for (i = 0; i < FileSize; i += ContentSize) {
        numBytes = openFile->Read(buffer, ContentSize);
        printf("read %d bytes\n", i);
	if ((numBytes < 10) || strncmp(buffer, Contents, ContentSize)) {
	    printf("Perf test: unable to read %s\n", FileName);
	    
	    delete openFile;
	    delete [] buffer;
	    
    	    fileSystem->Remove(FileName);
	    
	    return;
	}
    }
    delete [] buffer;
    delete openFile;	// close file
    
    fileSystem->Remove(FileName);
}

void
PerformanceTest()
{
    printf("Starting file system performance test:\n");
    stats->Print();
    FileWrite();
    FileRead();
    if (!fileSystem->Remove(FileName)) {
      printf("Perf test: unable to remove %s\n", FileName);
      return;
    }
    stats->Print();
}

void 
SyncTest() {
    FileWrite();
    Thread *r1 = new Thread("reader1", 8);
    Thread *r2 = new Thread("reader2", 8);
    r1->Fork(FileRead, (void *)r1->GetTid());
    r2->Fork(FileRead, (void *)r2->GetTid());
    FileWrite();
    fileSystem->Remove(FileName);
}

void
PipeReadTest() {
    char data[50];
    int length = fileSystem->ReadPipe(data);
}

void 
PipeWriteTest() {
    char data[50];
    scanf("%s", data);
    int len = strlen(data);
    int length = fileSystem->WritePipe(data, len);
    printf("write: %s, length: %d\n", data, length); 
}


void PipeTest2()
{
    printf("Thread 1 communicates with Thread 0\n");
    printf("Thread 1 reads data from the pipe\n");
    char data[SectorSize + 1];
    int length = fileSystem->ReadFromPipe(data, 0, 
        currentThread->GetTid());
    data[length]=0;
    printf("output: %s\n", data);
}


void PipeTest()
{
    //printf("Here is PipeTest()\n");
    printf("Thread 0 communicates with Thread 1\n");
    printf("Thread 0 writes data into the pipe\n");
    char input_str[20]="hello";
    printf("input: hello\n");
    Thread* th = new Thread("th2");
    fileSystem->WriteIntoPipe(input_str, strlen(input_str), 
        currentThread->GetTid(), th->GetTid());
    th->Fork(PipeTest2, th->GetTid());
    currentThread->Yield();
}


void MessageTest2()
{
    printf("Thread 1 communicates with Thread 0\n");
    printf("Thread 1 reads message from the buffer\n");
    char *data = Receive(currentThread->GetTid());
    ASSERT(data!=NULL);
    printf("output: %s\n", data);
}

void MessageTest()
{
    printf("Thread 0 communicates with Thread 1\n");
    printf("Thread 0 writes message into the buffer\n");
    char input_str[20]="hello";
    printf("input: hello\n");
    Thread* th = new Thread("th2");
    ASSERT(Send(input_str, th->GetTid()) == true);
    th->Fork(MessageTest2, th->GetTid());
    currentThread->Yield();
}
