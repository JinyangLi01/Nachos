// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;
Lock *lock = new Lock("test");

extern void ts();

List *slot;
int len;
Lock *mlock;
Condition *condc, *condp;
Semaphore *full, *mutex, *empty;

Lock *barrier_lock;
Condition *barrier_cond;
int thread_num;

Semaphore *r_mutex, *w_lock;
int rc = 0;
int buffer;

void 
Reader(int tid) {
    int item;
    for (int i = 0; i < 50; ++i) {
    	r_mutex->P();
    	rc++;
    	if (rc == 1)
    	    w_lock->P();
    	r_mutex->V();
    	item = buffer;
	printf("Reader with tid %d starts reading\n", tid);
	interrupt->SetLevel(IntOn);
        interrupt->SetLevel(IntOff);
	printf("Reader with tid %d reads %d from buffer\n", tid, item);
	interrupt->SetLevel(IntOn);
        interrupt->SetLevel(IntOff);
	printf("Reader with tid %d finishes reading\n", tid);
	
	interrupt->SetLevel(IntOn);
        interrupt->SetLevel(IntOff);
	r_mutex->P();
    	rc--;
    	if (rc == 0)
    	    w_lock->V();
    	r_mutex->V();
    }
}

void 
Writer(int tid) {
   int item;
   for (int i = 0; i < 100; ++i) {
   	item = i;
   	w_lock->P();
   	buffer = item;
	printf("Writer with tid %d starts writing\n", tid);
	printf("Writer with tid %d writes %d to buffer\n", tid, item);
	printf("Writer with tid %d finishes writing\n", tid);
	w_lock->V();
   }
}

void 
Producer1(int tid) {
    for (int i = 0; i < 50; ++i) {
	int item = i; // simple producer
	full->P();
	mutex->P();
	slot->Append((void *)item);
	printf("producer with tid %d produces %d\n", tid, item);
	mutex->V();
	empty->V();
    }
}

void 
Consumer1(int tid) {
    for (int i = 0; i < 50; ++i) {
	int item;
	empty->P();
	mutex->P();
	item = slot->Remove(); // simple consumer
	printf("consumer with tid %d consumes %d\n", tid, item);
	mutex->V();
	full->V();
    }
}


void 
Producer2(int tid) {
    for (int i = 0; i < 50; ++i) {
    	int item = i;
    	mlock->Acquire();
    	while (len == 5) condp->Wait(mlock);
    	slot->Append((void *)item);
    	len++;
	printf("producer with tid %d produces %d\n", tid, item);
    	condc->Signal(mlock);
    	mlock->Release();
    }

}

void 
Consumer2(int tid) {
    for (int i = 0; i < 50; ++i) {
    	int item;
    	mlock->Acquire();
    	while (len == 0) condc->Wait(mlock);
    	item = slot->Remove();
    	len--;
	printf("consumer with tid %d consumes %d\n", tid, item);
    	condp->Signal(mlock);
    	mlock->Release();
    }
}

void 
Barrier(int tid) {
    for (int num = 0; num < 10; num++) {
    	//ts();
	printf("*** thread tid %d looped %d times\n", tid, num);
	
	interrupt->SetLevel(IntOn);
        interrupt->SetLevel(IntOff);
    }
    barrier_lock->Acquire();
    if (thread_num == 1) {
   	printf("*** final thread tid %d reached barrier\n", tid, num);
        barrier_cond->Broadcast(barrier_lock);
    }
    else {
    	thread_num--;
   	printf("*** thread tid %d reached barrier\n", tid, num);
    	barrier_cond->Wait(barrier_lock);
    }
    barrier_lock->Release();
    printf("*** thread tid %d finished\n", tid, num);
}

void 
LockTest(int tid)
{
    int num;
    
    lock->Acquire();
    for (num = 0; num < 20; num++) {
    	//ts();
	printf("*** thread tid %d with priority %d looped %d times\n", tid, currentThread->getPriority(), num);
	interrupt->SetLevel(IntOn);
        interrupt->SetLevel(IntOff);
        //if (tid == 0)
          //  currentThread->Yield();
    }
    lock->Release();
}

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int tid)
{
    int num;
    
    for (num = 0; num < 5; num++) {
    	//ts();
	printf("*** thread tid %d with priority %d looped %d times\n", tid, currentThread->getPriority(), num);
	//interrupt->SetLevel(IntOn);
        //interrupt->SetLevel(IntOff);
        //if (tid == 0)
          //  currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    for (int i = 0; i < testnum; ++i) {
    	Thread *t = new Thread("forked thread", 8);   
    
    	t->Fork(SimpleThread, (void*)t->GetTid());
    }
    SimpleThread(currentThread->GetTid());
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void SemTest1() {
    slot = new List;
    full = new Semaphore("full", 5);
    mutex = new Semaphore("mutex", 1);
    empty = new Semaphore("empty", 0);
    
    Thread *p1 = new Thread("producer", 8); 
    Thread *t1 = new Thread("consumer", 8); 
    Thread *t2 = new Thread("consumer", 8);
     
    p1->Fork(Producer1, (void*)p1->GetTid());
    
    t1->Fork(Consumer1, (void*)t1->GetTid());    
    t2->Fork(Consumer1, (void*)t2->GetTid());
}

void SemTest2() {
    slot = new List;
    len = 0;
    mlock = new Lock("mutex");
    condp = new Condition("pcondition");
    condc = new Condition("ccondition");
    
    Thread *p1 = new Thread("producer", 8); 
    Thread *t1 = new Thread("consumer", 8); 
    Thread *t2 = new Thread("consumer", 8);
    
    
    t1->Fork(Consumer2, (void*)t1->GetTid()); 
     
    p1->Fork(Producer2, (void*)p1->GetTid());   
    t2->Fork(Consumer2, (void*)t2->GetTid());
}

void BarrierTest() {
    thread_num = 3;
    barrier_lock = new Lock("barrier");
    barrier_cond = new Condition("barrier");
    
    Thread *p1 = new Thread("b1", 8); 
    Thread *p2 = new Thread("b2", 8); 
    Thread *p3 = new Thread("b3", 8);
     
    p1->Fork(Barrier, (void*)p1->GetTid());
    p2->Fork(Barrier, (void*)p2->GetTid());    
    p3->Fork(Barrier, (void*)p3->GetTid());
	
}

void RCTest() {
    rc = 0;
    buffer = 0;
    w_lock = new Semaphore("write_lock", 1);
    r_mutex = new Semaphore("mutex_lock", 1);
    
    Thread *p1 = new Thread("b1", 8); 
    Thread *p2 = new Thread("b2", 8); 
    Thread *p3 = new Thread("b3", 8);
     
    p1->Fork(Writer, (void*)p1->GetTid());
    p2->Fork(Reader, (void*)p2->GetTid());    
    p3->Fork(Reader, (void*)p3->GetTid());
}



void
ThreadTest()
{
    //BarrierTest();
    //ThreadTest1();
    
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;

    default:
	printf("No test specified.\n");
	break;
    }
}
