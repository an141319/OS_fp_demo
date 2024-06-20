// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------



//<TODO>
// Declare sorting rule of SortedList for L1 & L2 ReadyQueue
// Hint: Funtion Type should be "static int"
static int cmp_L1(Thread *a, Thread *b) {
    if (a->getRemainingBurstTime() == b->getRemainingBurstTime()) {
        return a->getID() < b->getID() ? -1 : 1;
    }
    return a->getRemainingBurstTime() < b->getRemainingBurstTime() ? -1 : 1;
}
static int cmp_L2(Thread *a, Thread *b) {
    return a->getID() < b->getID() ? -1 : 1;
}
//<TODO>

Scheduler::Scheduler()
{
	//schedulerType = type;
     //readyList = new List<Thread *>; 
    //<TODO>
    // Initialize L1, L2, L3 ReadyQueue
    L1ReadyQueue = new SortedList<Thread *> (cmp_L1);
    L2ReadyQueue = new SortedList<Thread *> (cmp_L2);
    L3ReadyQueue = new List<Thread *>;
    //<TODO>
	toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    //<TODO>
    // Remove L1, L2, L3 ReadyQueue
    delete L1ReadyQueue;
    delete L2ReadyQueue;
    delete L3ReadyQueue;
    //<TODO>
     //delete readyList; 
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
    //thread->setStatus(READY);
    Statistics* stats = kernel->stats;
    //<TODO>
    // According to priority of Thread, put them into corresponding ReadyQueue.
    // After inserting Thread into ReadyQueue, don't forget to reset some values.
    // Hint: L1 ReadyQueue is preemptive SRTN(Shortest Remaining Time Next).
    // When putting a new thread into L1 ReadyQueue, you need to check whether preemption or not.
    int queue_level=0;
    if (thread->getPriority() < 50) {
	queue_level=3;
	//DEBUG(dbgMLFQ, "Before L3ReadyQueue->Append(thread)");

	//DEBUG(dbgMLFQ, "Thread " << thread->getID() << " is being put into L3ReadyQueue");
	//ListIterator<Thread *> *it3 = new ListIterator<Thread *>(L3ReadyQueue);
	//for (; !it3->IsDone(); it3->Next()) {
	//	Thread *thread = it3->Item();
	//	DEBUG(dbgMLFQ, "Thread " << thread->getID() << " is in L3ReadyQueue");
	//}	
	L3ReadyQueue->Append(thread);
	//DEBUG(dbgMLFQ, "After L3ReadyQueue->Append(thread)");
    }
    else if (thread->getPriority() < 100) {
	queue_level=2;
	//DEBUG(dbgMLFQ, "Before L2ReadyQueue->Append(thread)");
	L2ReadyQueue->Insert(thread);
	//DEBUG(dbgMLFQ, "After L2ReadyQueue->Append(thread)");
    }
    else if (thread->getPriority() < 150) {
	queue_level=1;
	//DEBUG(dbgMLFQ, "Before L1ReadyQueue->Append(thread)");
	L1ReadyQueue->Insert(thread);
	//DEBUG(dbgMLFQ, "After L1ReadyQueue->Append(thread)");

	Thread* curThread = kernel->currentThread;
	if (curThread->getID()>0) {
	    if (curThread->getPriority() < 100) {
	    //DEBUG(dbgMLFQ, "Before Interrupt")
	        kernel->interrupt->YieldOnReturn();
	    }
	    else if (cmp_L1(thread, curThread) < 0) {
	        kernel->interrupt->YieldOnReturn();
	    }
        }
    }
    thread->setStatus(READY);
    // thread->setWaitTime(0);
    DEBUG(dbgMLFQ, "[InsertToQueue] Tick[" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[" << queue_level << "]");
    //<TODO>
    //readyList->Append(thread);
}

bool
Scheduler::PreemptiveCompare() 
{ 
	if (kernel->currentThread->getPriority() >= 100){ 
	    if (!L1ReadyQueue->IsEmpty()){ 
                Thread* first = L1ReadyQueue->Front(); 
	        double first_remain = first->getRemainingBurstTime() - first->getRunTime();
	        double now_remain = kernel->currentThread->getRemainingBurstTime() - kernel->currentThread->getRunTime();
	        if (first_remain < now_remain) return true; 
            } 
	} 
	else if (kernel->currentThread->getPriority() < 100 && kernel->currentThread->getPriority() >= 50){ 
	    if (!L1ReadyQueue->IsEmpty()){ 
	        return true; 
	    } 
	} 
	return false; 
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    /*if (readyList->IsEmpty()) {
    return NULL;
    } else {
        return readyList->RemoveFront();
    }*/

    //<TODO>
    // a.k.a. Find Next (Thread in ReadyQueue) to Run
    if (!L1ReadyQueue->IsEmpty()) {
        DEBUG(dbgMLFQ, "[RemoveFromQueue] Tick[" << kernel->stats->totalTicks << "]: Thread [" 	<< L1ReadyQueue->Front()->getID() << "] is removed from queue L[1]");        
    	return L1ReadyQueue->RemoveFront();
    }
    if (!L2ReadyQueue->IsEmpty()) {
        DEBUG(dbgMLFQ, "[RemoveFromQueue] Tick[" << kernel->stats->totalTicks << "]: Thread [" 	<< L2ReadyQueue->Front()->getID() << "] is removed from queue L[2]");        
    	return L2ReadyQueue->RemoveFront();
    }
    if (!L3ReadyQueue->IsEmpty()) {
        DEBUG(dbgMLFQ, "[RemoveFromQueue] Tick[" << kernel->stats->totalTicks << "]: Thread [" 	<< L3ReadyQueue->Front()->getID() << "] is removed from queue L[3]");        
    	return L3ReadyQueue->RemoveFront();
    }
    return NULL;
    //<TODO>
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;
 
    //cout << "Current Thread" <<oldThread->getName() << "    Next Thread"<<nextThread->getName()<<endl;
    //DEBUG(dbgMLFQ, "Scheduler::Run");

    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// // finishing == true: need to delete current thread
         ASSERT(toBeDestroyed == NULL);
	     toBeDestroyed = oldThread;
    }
    
    // Reset oldThread's run time
    oldThread->setRunTime(0);
   
#ifdef USER_PROGRAM			// ignore until running user programs 
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	    oldThread->space->SaveState();
    }
#endif
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    DEBUG(dbgMLFQ, "[ContextSwitch] Tick[" << kernel->stats->totalTicks << "]: Thread ["
          << nextThread->getID() << "] is now selected for execution. Thread ["
          << oldThread->getID() << "] is replaced.");
    SWITCH(oldThread, nextThread);

    // we're back, running oldThread
    //DEBUG(dbgMLFQ, "Before ASSERT in scheduler::run");
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    //DEBUG(dbgMLFQ, "After ASSERT in scheduler::run");

    DEBUG(dbgThread, "Now in thread: " << kernel->currentThread->getID());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
#ifdef USER_PROGRAM
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	    oldThread->space->RestoreState();
    }
#endif

    //DEBUG(dbgMLFQ, "Scheduler::Run finished");
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        DEBUG(dbgThread, "toBeDestroyed->getID(): " << toBeDestroyed->getID());
        delete toBeDestroyed;
	    toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Ready list contents:\n";
    // readyList->Apply(ThreadPrint);
    L1ReadyQueue->Apply(ThreadPrint);
    L2ReadyQueue->Apply(ThreadPrint);
    L3ReadyQueue->Apply(ThreadPrint);
}

// <TODO>
// Function 1. Function definition of sorting rule of L1 ReadyQueue
// Function 2. Function definition of sorting rule of L2 ReadyQueue
// Function 3. Scheduler::UpdatePriority()
// Hint:INT 
// 1. ListIterator can help.
// 2. Update WaitTime and priority in Aging situations
// 3. After aging, Thread may insert to different ReadyQueue

void 
Scheduler::UpdatePriority()
{
    ListIterator<Thread *> *it1 = new ListIterator<Thread *>(L1ReadyQueue);
    ListIterator<Thread *> *it2 = new ListIterator<Thread *>(L2ReadyQueue);
    ListIterator<Thread *> *it3 = new ListIterator<Thread *>(L3ReadyQueue);

    for(; !it1->IsDone(); it1->Next()) {
        Thread *thread = it1->Item();
        thread->setWaitTime(thread->getWaitTime() + TimerTicks);

        if(thread->getWaitTime() >= 400) {
            int oldPriority = thread->getPriority();
            thread->setPriority(oldPriority + 10);
            thread->setWaitTime(0);

            L1ReadyQueue->Remove(thread);
    	    ReadyToRun(thread);
	    DEBUG(dbgMLFQ, "[UpdatePriority] Tick [" << kernel->stats->totalTicks << "]: Thread ["
                  << thread->getID() << "] changes its priority from [" << oldPriority << "] to ["
                  << thread->getPriority() << "]");
            //DEBUG(dbgMLFQ, "[Context Switch] Tick[" << kernel->stats->totalTicks << "]: Thread ["
            //      << thread->getID() << "] is now selected for execution.");
            DEBUG(dbgMLFQ, "Thread " << thread->getName() << " moved from a higher level queue to L1ReadyQueue due to priority change.");
        }
    }

    for(; !it2->IsDone(); it2->Next()) {
        Thread *thread = it2->Item();
        thread->setWaitTime(thread->getWaitTime() + TimerTicks);

        if(thread->getWaitTime() >= 400) {
            int oldPriority = thread->getPriority();
            thread->setPriority(oldPriority + 10);
            thread->setWaitTime(0);
	    /*DEBUG(dbgMLFQ, "[UpdatePriority] Tick [" << kernel->stats->totalTicks << "]: Thread ["
                  << thread->getID() << "] changes its priority from [" << oldPriority << "] to ["
                  << thread->getPriority() << "]");*/

            L2ReadyQueue->Remove(thread);
            ReadyToRun(thread);
            DEBUG(dbgMLFQ, "[UpdatePriority] Tick [" << kernel->stats->totalTicks << "]: Thread ["
                  << thread->getID() << "] changes its priority from [" << oldPriority << "] to ["
                  << thread->getPriority() << "]");
            //DEBUG(dbgMLFQ, "[Context Switch] Tick[" << kernel->stats->totalTicks << "]: Thread ["
              //    << thread->getID() << "] is now selected for execution.");
            DEBUG(dbgMLFQ, "Thread " << thread->getName() << " moved from a higher level queue to L2ReadyQueue due to priority change.");
        }
    }

    for(; !it3->IsDone(); it3->Next()) {
        Thread *thread = it3->Item();
        thread->setWaitTime(thread->getWaitTime() + TimerTicks);

        if(thread->getWaitTime() >= 400) {
            int oldPriority = thread->getPriority();
            thread->setPriority(oldPriority + 10);
            thread->setWaitTime(0);
		
            L3ReadyQueue->Remove(thread);
            ReadyToRun(thread);
            DEBUG(dbgMLFQ, "[UpdatePriority] Tick [" << kernel->stats->totalTicks << "]: Thread ["
                  << thread->getID() << "] changes its priority from [" << oldPriority << "] to ["
                  << thread->getPriority() << "]");
            //DEBUG(dbgMLFQ, "[Context Switch] Tick[" << kernel->stats->totalTicks << "]: Thread ["
             //     << thread->getID() << "] is now selected for execution.");
            DEBUG(dbgMLFQ, "Thread " << thread->getName() << " moved from a higher level queue to L3ReadyQueue due to priority change.");
        }
    }

    delete it1;
    delete it2;
    delete it3;
}
// <TODO>
