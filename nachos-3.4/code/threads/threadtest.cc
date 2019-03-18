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
#include "thread.h"
#include "switch.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
printf("Thread Tid %d BEGIN\n", currentThread->GetTid());
    int num;
    
    for (num = 0; num <= 50; num++) {
        if(num % 10 ==0)
	printf("*** thread %s, Tid %d,  Priority %d, time-slice %d, looped %d times\n", currentThread->getName(), currentThread->GetTid(), currentThread->GetPriority(), currentThread->GetTimeSlice(), num);
       // currentThread->Yield(); 
	

	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	(void) interrupt->SetLevel(oldLevel);

	if(num ==0 && currentThread->GetPriority()!=0)
	{
	    Thread *t = new Thread("Thread", currentThread->GetPriority()-1);
	    t->Fork(SimpleThread, t->GetTid());
	}
    }
printf("Thread Tid %d END\n", currentThread->GetTid());
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

    Thread *t1 = new Thread("forked thread");
    Thread *t2 = new Thread("forked thread");
    Thread *t3 = new Thread("forked thread");
    Thread *t4 = new Thread("forked thread");


    t1->Fork(SimpleThread, t1->GetTid());
    t2->Fork(SimpleThread, t2->GetTid());
    t3->Fork(SimpleThread, t3->GetTid());
    t4->Fork(SimpleThread, t4->GetTid());

    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest2
// 	There can only be at most 128 threads, MaxThread=128
//	use this function to create 129 threads and print error information when createing the 129th thread, and stop
//----------------------------------------------------------------------



void
ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest1");
    for(int i=0;i<=128;i++)
    {
	Thread *t = new Thread("forked thread");
	printf("Thread name %s, tid %d, uid %d\n", t->getName(), t->GetTid(), t->GetUid());
    }

    SimpleThread(0);
}


//----------------------------------------------------------------------
// ThreadTest3
// 	print thread status
//	
//----------------------------------------------------------------------


void
ThreadTest3()
{
//printf("threadtest3\n");
    DEBUG('t', "Entering ThreadTest1");

    Thread *t1 = new Thread("forked thread");
    Thread *t2 = new Thread("forked thread");
    Thread *t3 = new Thread("forked thread");
    Thread *t4 = new Thread("forked thread");



    t1->Fork(ReadyQueuePrint, t1->GetTid());
    t2->Fork(ReadyQueuePrint, t2->GetTid());
    t3->Fork(ReadyQueuePrint, t3->GetTid());
    t4->Fork(ReadyQueuePrint, t4->GetTid());



}


void
ReadyQueuePrint()
{
    printf("ReadyQueuePrint\n");
    IntStatus OldLevel = interrupt->SetLevel(IntOff);//关中断

//print the information of current thread
    printf("name %s, Tid %d, Uid, %d, status %s\n", currentThread->getName(), currentThread->GetTid(), currentThread->GetUid(), ThreadStatusChar[currentThread->GetStatus()]);

//print information of threads in the ready queue
    List * lst = new List();
    lst = scheduler->getReadyList();

    if(!lst->IsEmpty())
    {
	lst->Mapcar(MyThreadStatusPrint);
    }
    currentThread->Yield();
    interrupt->SetLevel(OldLevel);

}

void MyThreadStatusPrint(int a)
{
    Thread *t = (Thread*) a;
    printf("thread %s, uid %d, tid %d, status %s \n", t->getName(), t->GetUid(), t->GetTid(), ThreadStatusChar[t->GetStatus()]);

}



//----------------------------------------------------------------------
// ThreadTest4
// 	print thread priority
//----------------------------------------------------------------------


void
ThreadTest4()
{
    Thread *t1 = new Thread("forked thread1", 4);
    Thread *t2 = new Thread("forked thread2", 3);
    Thread *t3 = new Thread("forked thread3", 2);
    Thread *t4 = new Thread("forked thread4", 1);


    t1->Fork(SimpleThread, t1->GetTid());
    t2->Fork(SimpleThread, t2->GetTid());
    t3->Fork(SimpleThread, t3->GetTid());
    t4->Fork(SimpleThread, t4->GetTid());

    SimpleThread(0);
}

//----------------------------------------------------------------------
// PriorityPreempt
//     create a thread within a thread to test preempt 
//     
//----------------------------------------------------------------------



void p2(int which)
{
    int num;
    for(num=0;num<5;num++)
    {
	printf("*** thread  %s, Priority %d, looped %d times\n", currentThread->getName(), currentThread->GetPriority(), num);
	if(num==0)
	{
	    Thread *t3 = new Thread("Thread3", 4);
	    t3->Fork(SimpleThread, 1);
	}
    }
}


void p1(int which)
{
    int num;
    for(num=0;num<5;num++)
    {
	printf("*** thread  %s, Priority %d, looped %d times\n", currentThread->getName(), currentThread->GetPriority(), num);
	if(num==0)
	{
	    Thread *t2 = new Thread("Thread2", 1);
	    t2->Fork(p2, 1);
	}
    }
}


void
PriorityPreempt()
{
    Thread *t1 = new Thread("Thread1", 8);

    t1->Fork(p1, 1);

}


//----------------------------------------------------------------------
// ThreadTest6
//       test time slice
//     
//----------------------------------------------------------------------


void ThreadTest6()
{
    printf("Initial Time Slice %d\n", InitialTimeSlice);
    Thread * t1 = new Thread("Thread", 3);
    printf("New Thread %d Created\n", t1->GetTid());
    t1->Fork(SimpleThread, t1->GetTid());


}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
 //   printf("ThreadTest, testnum %d\n", testnum);
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    case 2:
	ThreadTest2();
	break;
    case 3:
	ThreadTest3();
	break;
    case 4:
	ThreadTest4();
	break;
    case 5:
	PriorityPreempt();
	break;
    case 6:
	ThreadTest6();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

