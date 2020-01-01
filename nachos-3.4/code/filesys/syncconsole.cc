#include "copyright.h"
#include "syncconsole.h"

static void ConsoleReadAvail(int arg) { 
    SyncConsole* console = (SyncConsole *)arg;
    console->ReadAvail(); 
}
static void ConsoleWriteDone(int arg) { 
    SyncConsole* console = (SyncConsole *)arg;
    console->WriteDone(); 
}


SyncConsole::SyncConsole(char *readFile, char *writeFile)
{
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    lock = new Lock("console lock");
    
    console = new Console(readFile, writeFile, ConsoleReadAvail, ConsoleWriteDone, (char *)this);
}

//----------------------------------------------------------------------
// Console::~Console
// 	Clean up console emulation
//----------------------------------------------------------------------

SyncConsole::~SyncConsole()
{
    delete readAvail;
    delete writeDone;
    delete console;
    delete lock;
}


char
SyncConsole::GetChar()
{
   char ch;
   lock->Acquire();
   readAvail->P();
   ch = console->GetChar();
   lock->Release();
   
   return ch;
}

void
SyncConsole::PutChar(char ch)
{
    lock->Acquire();
    console->PutChar(ch);
    writeDone->P();
    lock->Release(); 
}

void 
SyncConsole::ReadAvail() { 
    readAvail->V(); 
}

void 
SyncConsole::WriteDone() { 
    writeDone->V(); 
}
