// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"



struct Assist{
    AddrSpace *addr;
    int func;
};



char name[10];
Assist *as = new Assist;

void ChangePC() {
    machine->WriteRegister(PrevPCReg, machine->registers[PCReg]);
    machine->WriteRegister(PCReg, machine->registers[NextPCReg]);
    machine->WriteRegister(NextPCReg, machine->registers[NextPCReg] + sizeof(int));
}

void PageTableMissHandler() {
    int vpn = (unsigned)machine->registers[BadVAddrReg] / PageSize;
    //printf("Page fault occurred in Thread %d at Virtual Page %d\n", currentThread->GetTid(), vpn);
    machine->search(vpn);
    
}

void TLBMissHandler() {
    /*
    int vpn = machine->registers[BadVAddrReg] / PageSize;
    int mark = -1;
    for (int i = 0; i < TLBSize; ++i) {
    	if (!machine->tlb[i].valid) {
    	    mark = i;
    	    break;
    	}
    }
    
    if (mark == -1) {
    	if (machine->FIFO) {   // use FIFO
    	    mark = TLBSize - 1;
    	    for (int i = 0; i < TLBSize - 1; ++i) {
    	    	machine->tlb[i] = machine->tlb[i + 1];
    	    }
    	
    	} else {    // use LRU
    	    for (int i = 0; i < TLBSize; ++i) {
    	    	if (machine->tlbTime[i] == TLBSize) {
    	    	    mark = i;
    	    	    break;
    	    	}
    	    }
    	}
    }
    
    if (!machine->FIFO) {  
    	machine->tlbTime[mark] = 1;
    	for (int i = 0; i < TLBSize; ++i) {
    	    if (i == mark || !machine->tlb[i].valid) continue;
   	    machine->tlbTime[i]++;
    	}
    }
    
    machine->tlb[mark].virtualPage = vpn;
    machine->tlb[mark].physicalPage = machine->pageTable[vpn].physicalPage;
    machine->tlb[mark].valid = TRUE;
    machine->tlb[mark].use = FALSE;
    machine->tlb[mark].dirty = FALSE;
    machine->tlb[mark].readOnly = FALSE;
    */
}


void ExecFunc(char *filename) {
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    
    space = new AddrSpace(executable);    
    currentThread->space = space;

    delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}

void ForkFunc(int _as) {   
    Assist *as = (Assist *)_as;
    
    currentThread->space = as->addr;

    //currentThread->space->InitRegisters();		// set the initial register values
    currentThread->space->RestoreState();		// load page table register
    
    machine->WriteRegister(PCReg, as->func);
    machine->WriteRegister(NextPCReg, as->func + sizeof(int));
    
    

    currentThread->SaveUserState();

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
	
	// ADD FOR TLB HIT RATE
	//double hit_rate = 1.0 * machine->tlbHit / (machine->tlbHit + machine->tlbMiss);
    	//printf("TLB HIT %d times, TLB miss %d times, hit rate is %lf\n", machine->tlbHit, machine->tlbMiss, hit_rate);
 
   	interrupt->Halt();
    } else if ((which == SyscallException) && (type == SC_Create)) {
    	int address = machine->ReadRegister(4);
    	int i = 0, temp;
    	do {
    	    machine->ReadMem(address + i, 1, &temp);
    	    name[i] = temp;
    	} while (name[i++] != 0);
    	
    	#ifdef FILESYS_NEEDED
    	fileSystem->Create(name, 128);
    	#endif
    	
    	printf("file created\n");
    	ChangePC();
    } else if ((which == SyscallException) && (type == SC_Open)) {  	
    	int address = machine->ReadRegister(4);
    	int i = 0, temp;
    	do {
    	    machine->ReadMem(address + i, 1, &temp);
    	    name[i] = temp;
    	} while (name[i++] != 0);
    	
    	#ifdef FILESYS_NEEDED
    	OpenFile *openFile = fileSystem->Open(name);
    	#endif
    	
    	printf("file opened\n");
    	
    	machine->WriteRegister(2, (int)openFile);
    	ChangePC();
    	
    } else if ((which == SyscallException) && (type == SC_Close)) {
    
    	#ifdef FILESYS_NEEDED
    	OpenFile *file = (OpenFile *)machine->ReadRegister(4);
    	delete file;
    	#endif
    	
    	printf("file closed\n");
    	ChangePC();
    	
    } else if ((which == SyscallException) && (type == SC_Write)) {
    	int address = machine->ReadRegister(4);
    	int size = machine->ReadRegister(5);
    	char *buffer = new char[size];
    	printf("size %d\n", size);
    	for (int i = 0; i < size; i++) {
    	    int temp;
    	    machine->ReadMem(address + i, 1, &temp);
    	    buffer[i] = char(temp);  
    	}
    	
    	#ifdef FILESYS_NEEDED
    	OpenFile *file = (OpenFile *)machine->ReadRegister(6);
	file->Write(buffer, size);
	#endif
	
	printf("write file\n");
	ChangePC();
    } else if ((which == SyscallException) && (type == SC_Read)) {
    	int address = machine->ReadRegister(4);
    	int size = machine->ReadRegister(5);
    	char *buffer = new char[size];
    	int length;
    	int temp;
    	
    	#ifdef FILESYS_NEEDED
    	OpenFile *file = (OpenFile *)machine->ReadRegister(6);
    	length = file->Read(buffer, size);
    	#endif
    	
    	for (int i = 0; i < length; ++i) {
    	    machine->WriteMem(address + i, 1, (int)buffer[i]);
    	}
    	
    	buffer[length] = 0;
    	printf("%s", buffer);
    	printf("read file\n");
    	machine->WriteRegister(2, length);
    	ChangePC();
    } else if (which == PageFaultException) {
    	#ifdef USE_TLB 
    	    TLBMissHandler();
    	#else
    	    PageTableMissHandler();
    	#endif
    	
    } else if ((which == SyscallException) && (type == SC_Exit)) {
    	printf("Thread %d exits.\n", currentThread->GetTid());
	machine->clear();
	currentThread->Finish();	
	
    } else if ((which == SyscallException) && (type == SC_Exec)) {
    	int address = machine->ReadRegister(4);
    	int i = 0, temp;
    	do {
    	    machine->ReadMem(address + i, 1, &temp);
    	    name[i] = temp;
    	} while (name[i++] != 0);
    	
    	Thread *t = new Thread("exec", 8);
    	t->Fork(ExecFunc, (void *)name); 
    	machine->WriteRegister(2, t->GetTid());
    	ChangePC();
    
    } else if ((which == SyscallException) && (type == SC_Fork)) {
    	int func = machine->ReadRegister(4); 
    	Thread *t = new Thread("fork", 8);
    	as->func = func;
    	as->addr = new AddrSpace(t->GetTid(), currentThread->GetTid(), currentThread->space->GetPage());
    	//t->SaveUserState();
    	t->Fork(ForkFunc, (int)as);
    	ChangePC();
    
    } else if ((which == SyscallException) && (type == SC_Yield)) {
	printf("thread yield\n");
    	currentThread->Yield();
    	ChangePC();
    } else if ((which == SyscallException) && (type == SC_Join)) {
    	int tid = machine->ReadRegister(4);
    	while (tidArray[tid] != 0)
    	    currentThread->Yield();
    	ChangePC();
    	
    
    } else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}
