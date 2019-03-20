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
//#include <cstdlib>
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

//-----------------------------LAB 1 BEGIN------------------------------------

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


//----------------------------------LAB 1 END--------------------------------------


//----------------------------------LAB 2 BEGIN-----------------------------------------------

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
// ThreadTest5
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
ThreadTest5()
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

//---------------------------------------LAB 2 END ------------------------------------------


//---------------------------------LAB 3 BEGIN-------------------------------------------------
//----------------------------------------------------------------------
// Producer
//  Produce products and wait for customer
// Consumer
//  Wait for product and remove them
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// ThreadTest7
//  use lock and condition to present producer/consuer problem
//----------------------------------------------------------------------



static int productnum=0;
static int maxproduct=0;
Condition * cond;
Lock * lock;

void Producer(int which)
{
    DEBUG('t', "Entering Producer");

    for(int i=0;i<8;++i)
    {
        int temp= 1 + Random() + (TimerTicks * 2);
        if(temp%2)  currentThread->Yield();
        lock->Acquire();
        while(productnum>=maxproduct)
        {
            printf("Full! Producer (%d)\n", which);
            cond->Wait(lock);
            printf("Producer (%d) continues\n", which);
        }
        productnum++;
        printf("Producer %d produces one product, productnum=%d\n", which, productnum);
        if(productnum==1)
            cond->Broadcast(lock);
        lock->Release();
    }
}

void Consumer(int which)
{
    DEBUG('t', "Entering Consumer");

    for(int i=0;i<6;++i)
    {
        int temp= 1 + Random() + (TimerTicks * 2);
        if(temp%2)  currentThread->Yield();
        lock->Acquire();
        while(productnum<=0)
        {
            printf("Empty! Consumer (%d)\n", which);
            cond->Wait(lock);
            printf("Consumer(%d) continues\n", which);
        }
        productnum--;
        printf("Consumer %d consumes one product, productnum=%d\n", which, productnum);
        if(productnum==maxproduct-1)
            cond->Broadcast(lock);
        lock->Release();
    }
}

void ThreadTest7()
{
    DEBUG("t", "Entering ThreadTest7");
    cond=new Condition("condition");
    lock=new Lock("lock");
    maxproduct=4;
    Thread *p[2], *c[2];
    for(int i=0;i<2;++i)
    {
        p[i]=new Thread("producer", 3);
        p[i]->Fork(Producer, (void*)i);
    }
    for(int i=0;i<2;++i)
    {
        c[i]=new Thread("consumer", 3);
        c[i]->Fork(Consumer, (void*)i);
    } 
}




//----------------------------------------------------------------------
// ThreadTest8
//  use semaphore to present producer/consuer problem
//----------------------------------------------------------------------

Semaphore *full, *empty;
Semaphore *mutex;
void Producer_sem(int which)
{
    DEBUG("t", "Entering Producer_sem");

    for(int i=0;i<8;++i)
    {
    //    int temp= 1 + Random() + (TimerTicks * 2);
    //    if(temp%2)  currentThread->Yield();
        empty->P();
        mutex->P();
        productnum++;
        printf("Producer %d produces a product, productnum=%d\n", which, productnum);
        mutex->V();
        full->V();
    }
}

void Consumer_sem(int which)
{
    DEBUG("t", "Entering Consumer_sem");

    for(int i=0;i<6;++i)
    {
    //    int temp= 1 + Random() + (TimerTicks * 2);
     //   if(temp%2)  currentThread->Yield();
        full->P();
        mutex->P();
        productnum--;
        printf("Consumer %d consumes a product, productnum=%d\n", which, productnum);
        mutex->V();
        empty->V();
    }
}

void ThreadTest8()
{
    full=new Semaphore("full", 0);
    empty=new Semaphore("empty", 4);
    mutex=new Semaphore("mutex", 1);
    Thread *p[2], *c[2];
    for(int i=0;i<2;++i)
    {
        p[i]=new Thread("producer", 3);
        p[i]->Fork(Producer_sem, (void*)i);
    }
    for(int i=0;i<2;++i)
    {
        c[i]=new Thread("consumer", 3);
        c[i]->Fork(Consumer_sem, (void*)i);
    }
}




//----------------------------------------------------------------------
// ThreadTest9
//  use nachos to realize a barrier
//----------------------------------------------------------------------

//Lock * lock;
//Condition * condition;
int barrier=0;
int countnum=0;

void BarrierTest(int n)
{
    lock->Acquire();
    countnum++;
    printf("Thread %s gets the lock\n", currentThread->getName());
    if(countnum==barrier)
    {
        printf("countnum: %d, Broadcast from thread %s\n", countnum, currentThread->getName());
        cond->Broadcast(lock);
        lock->Release();
    }
    else
    {
        printf("countnum: %d, thread %s wait\n", countnum, currentThread->getName());
        cond->Wait(lock);
        lock->Release();
    }
    printf("thread %s continues to run\n", currentThread->getName());

}

ThreadTest9()
{
    barrier=3;
    lock = new Lock("barrierLock");
    cond=new Condition("barrierCondition");
    Thread * t1 = new Thread("t1", 3);;
    Thread * t2 = new Thread("t2", 3);
    Thread * t3 = new Thread("t3", 3);
//    Thread * t4 = new Thread("t4", 4);
    t1->Fork(BarrierTest, 1);
    t2->Fork(BarrierTest, 2);
    t3->Fork(BarrierTest, 3);
//    t4->Fork(BarrierTest, 4);

}



//----------------------------------------------------------------------
// ThreadTest10
//  use nachos to realize reader-writer problem
//----------------------------------------------------------------------

//int file=1;
int readerCount=0;
Lock * rlock, *wlock;

void Read(int n)
{
 //    int temp= 1 + Random() + (TimerTicks * 2);
 //   if(temp%2)  currentThread->Yield();
    for(int i=0;i<3;++i)
    {
                                   interrupt->OneTick();
        rlock->Acquire();          interrupt->OneTick();
        readerCount++;        interrupt->OneTick();
        if(readerCount==1)      //  interrupt->OneTick();
        {
            wlock->Acquire();        interrupt->OneTick();
        }
        rlock->Release();        interrupt->OneTick();

        printf("Reader Thread %d comes in\n", n);        interrupt->OneTick();
        printf("There are %d readers\n", readerCount);
        printf("Reader Thread %d is reading\n", n);        interrupt->OneTick();
        printf("Reader Thread %d leaves\n", n);        interrupt->OneTick();

        rlock->Acquire();        interrupt->OneTick();
        readerCount--;        interrupt->OneTick();
        if(readerCount==0)
        {
            wlock->Release();        interrupt->OneTick();
        }
        rlock->Release();        interrupt->OneTick();
    }
}

void Write(int n)
{
 //   int temp= 1 + Random() + (TimerTicks * 2);
 //   if(temp%2)  currentThread->Yield();
    for(int i=0;i<3;++i)
    {
        wlock->Acquire();interrupt->OneTick();
        printf("Writer Thread %d comes in\n", n);interrupt->OneTick();
        printf("Writer Thread %d is writing\n", n);interrupt->OneTick();
        printf("Writer Thread %d leaves\n", n);interrupt->OneTick();
        wlock->Release();interrupt->OneTick();
    }
}
void ThreadTest10()
{
    rlock=new Lock("rlock");
    wlock=new Lock("wlock");
    Thread * w[2],*r[3]; 
    for(int i=0;i<2;++i)
    {
        w[i] = new Thread("writer", 3);
    }
    for(int i=0;i<3;++i)
    {
        r[i]=new Thread("reader", 3);
    }
    r[0]->Fork(Read, 0);
    w[0]->Fork(Write, 0);
    r[1]->Fork(Read, 1);
    w[1]->Fork(Write, 1);
    r[2]->Fork(Read, 2);
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
	ThreadTest5();
	break;
    case 6:
	ThreadTest6();
	break;
    case 7:
    ThreadTest7();
    break;
    case 8:
    ThreadTest8();
    break;
    case 9:
    ThreadTest9();
    break;
    case 10:
    ThreadTest10();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}

