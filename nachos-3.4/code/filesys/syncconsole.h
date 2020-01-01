

#include "copyright.h"
#include "console.h"
#include "synch.h"

class SyncConsole {
  public:
    SyncConsole(char *readFile, char *writeFile);
				
    ~SyncConsole();		
    
    void PutChar(char ch);

    char GetChar();
				
				
    void ReadAvail();
    void WriteDone();


  private:
    Console *console;
    Semaphore *readAvail, *writeDone;
    Lock *lock;
};
