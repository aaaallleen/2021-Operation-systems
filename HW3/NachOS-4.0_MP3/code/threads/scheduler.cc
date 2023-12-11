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

int Scheduler::priority(Thread *a, Thread *b){
    int a_priority = a->getpriority();
    int b_priority = b->getpriority();
    return ((a_priority == b_priority) ? 0 : ((a_priority > b_priority) ? -1 : 1));
}
int Scheduler::sjf(Thread *a, Thread *b){
    double timeA = a->CPUguesssedburst;
   	double timeB = b->CPUguesssedburst;
    return ((timeA == timeB)?priority(a, b):((timeA > timeB) ? -1 : 1 ));
}
bool Scheduler::IsPreemptive()
{
    if (!L1_empty()) {
        Thread *first = L1readyList->RemoveFront();
        L1readyList->Insert(first);
        Thread *now = kernel->currentThread;
        // cout << '\n' << first->CPUguesssedburst << "\n" << now->CPUguesssedburst << '\n';
        return first->CPUguesssedburst < now->CPUguesssedburst;
    } 
    else 
        return false;
}
//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------
Scheduler::Scheduler()
{   
    L1readyList = new SortedList<Thread *> (sjf);
    L2readyList = new SortedList<Thread *> (priority);
    L3readyList = new List<Thread *>;
    toBeDestroyed = NULL;
}

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{   
    delete L1readyList;
    delete L2readyList;
    delete L3readyList;
}

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------
void Scheduler::Aging(){
    SortedList<Thread *> *newL1 = new SortedList<Thread *> (sjf);
    SortedList<Thread *> *newL2 = new SortedList<Thread *> (priority);
    List<Thread *> *newL3 = new List<Thread *>;
    while(!L1readyList->IsEmpty()){
        Thread *thread = L1readyList->RemoveFront();
        thread -> Aging();
        newL1->Insert(thread);
    }
    while(!L2readyList->IsEmpty()){
        Thread *thread = L2readyList->RemoveFront();
        thread -> Aging();
        if(thread->getpriority()>=100){
            DEBUG(dbgMP3, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is removed from queue L[2]");
            DEBUG(dbgMP3, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[1]");
            newL1->Insert(thread);
        }
        else
            newL2->Insert(thread);
    }
    while(!L3readyList->IsEmpty()){
        Thread *thread = L3readyList->RemoveFront();
        thread -> Aging();
        if(thread->getpriority()>=50){
            DEBUG(dbgMP3, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is removed from queue L[3]");
            DEBUG(dbgMP3, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[2]");
            newL2->Insert(thread);
        }
        else
            newL3->Append(thread);
    }
    delete L1readyList;
    delete L2readyList;
    delete L3readyList;
    L1readyList = newL1;
    L2readyList = newL2;
    L3readyList = newL3;
}
void Scheduler::PutBack(Thread *thread){
    if(thread->getpriority() >99){
        cout << "Put Back Thread[" << thread->getID()<<"] it's priority is " << thread->getpriority() << endl;
        L1readyList->Insert(thread);
    }
    else if(thread->getpriority() > 49){
        L2readyList->Insert(thread);
    }
    else{
        L3readyList->Append(thread);
    }
}
void Scheduler::ReadyToRun(Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
    //cout << "Putting thread on ready list: " << thread->getName() << endl ;
    thread->setStatus(READY);
    // readyList->Append(thread);
    
    int priority = thread -> getpriority();
    if(priority > 99){
        L1readyList->Insert(thread);
        DEBUG(dbgMP3, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[1]");
        // Thread *oldThread = kernel->currentThread;
        // oldThread->setburst( kernel->stats->totalTicks - oldThread->s_tick );
        // int burst = thread->getburst();
        // double oldburst = 0.5*oldThread->getburst() + 0.5*(kernel->stats->totalTicks - oldThread->s_tick);
        // // thread->setburst(oldburst);
        // // DEBUG(dbgMP3, "[D] Tick [" << kernel->stats->totalTicks << "]: Thread [" 
        // //     << oldThread->getID() << "] update approximate burst time, from: [" << burst << "], add [" 
        // //     << oldburst-burst << "], to [" 
        // //     << oldburst << "]");
        // double remainingBurstTime = burst - (kernel->stats->totalTicks - thread->s_tick);
        // if(oldburst > remainingBurstTime){
        //     kernel -> currentThread -> Yield();
        // }
    }
    else if (priority > 49){
        L2readyList->Insert(thread);
        DEBUG(dbgMP3, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[2]");
    }
    else {
        L3readyList->Append(thread);
        DEBUG(dbgMP3, "[A] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is inserted into queue L[3]");
    }
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    Thread *thread = NULL;
    if(!L1readyList->IsEmpty()){
        thread = L1readyList->RemoveFront();
        DEBUG(dbgMP3, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is removed from queue L[1]");
    }
    else if (!L2readyList->IsEmpty()){
        thread = L2readyList->RemoveFront();
        DEBUG(dbgMP3, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is removed from queue L[2]");
    }
    else if(!L3readyList->IsEmpty()){
        thread = L3readyList->RemoveFront();
        DEBUG(dbgMP3, "[B] Tick [" << kernel->stats->totalTicks << "]: Thread [" << thread->getID() << "] is removed from queue L[3]");
    }
    return thread;
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

void Scheduler::Run(Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;

    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing)
    { // mark that we need to delete current thread
        ASSERT(toBeDestroyed == NULL);
        toBeDestroyed = oldThread;
    }

    if (oldThread->space != NULL)
    {                               // if this thread is a user program,
        oldThread->SaveUserState(); // save the user's CPU registers
        oldThread->space->SaveState();
    }

    oldThread->CheckOverflow(); // check if the old thread
                                // had an undetected stack overflow

    kernel->currentThread = nextThread; // switch to the next thread
    nextThread->setStatus(RUNNING);     // nextThread is now running

    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());

    // This is a machine-dependent assembly language routine defined
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    nextThread->s_tick = kernel->stats->totalTicks;
    DEBUG(dbgMP3, "[E] Tick [" << kernel->stats->totalTicks << "]: Thread [" << nextThread->getID() << "] is now selected for execution, thread [" << oldThread->getID() << "] is replaced, and it has executed [" << (kernel->stats->totalTicks - oldThread->s_tick) << "] ticks");
    SWITCH(oldThread, nextThread);

    // we're back, running oldThread

    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed(); // check if thread we were running
                          // before this one has finished
                          // and needs to be cleaned up

    if (oldThread->space != NULL)
    {                                  // if there is an address space
        oldThread->RestoreUserState(); // to restore, do it.
        oldThread->space->RestoreState();
    }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL)
    {
        delete toBeDestroyed;
        toBeDestroyed = NULL;
    }
}

//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void Scheduler::Print()
{
    cout << "Ready list contents:\n";
    // readyList->Apply(ThreadPrint);
}
