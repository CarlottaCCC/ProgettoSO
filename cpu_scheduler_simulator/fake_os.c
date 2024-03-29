#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fake_os.h"

void FakeOS_init(FakeOS* os) {
  //os->running=0;
  List_init(&os->running);
  List_init(&os->ready);
  List_init(&os->waiting);
  List_init(&os->processes);
  os->timer=0;
  os->schedule_fn=0;
  //os->count=0;
  os->prev_quantum=0;


}

void FakeOS_createProcess(FakeOS* os, FakeProcess* p) {
  // sanity check
  assert(p->arrival_time==os->timer && "time mismatch in creation");
  // we check that in the list of PCBs there is no
  // pcb having the same pid


  ListItem* aux = os->running.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }
  

  aux=os->ready.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  aux=os->waiting.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  // all fine, no such pcb exists
  FakePCB* new_pcb=(FakePCB*) malloc(sizeof(FakePCB));
  new_pcb->list.next=new_pcb->list.prev=0;
  new_pcb->pid=p->pid;
  new_pcb->events=p->events;

  assert(new_pcb->events.first && "process without events");

  // depending on the type of the first event
  // we put the process either in ready or in waiting
  ProcessEvent* e=(ProcessEvent*)new_pcb->events.first;
  switch(e->type){
  case CPU:
    List_pushBack(&os->ready, (ListItem*) new_pcb);
    break;
  case IO:
    List_pushBack(&os->waiting, (ListItem*) new_pcb);
    break;
  default:
    assert(0 && "illegal resource");
    ;
  }
}




void FakeOS_simStep(FakeOS* os){
  
  printf("************** TIME: %08d **************\n", os->timer);


  //scan process waiting to be started
  //and create all processes starting now
  ListItem* aux=os->processes.first;
  while (aux){
    FakeProcess* proc=(FakeProcess*)aux;
    FakeProcess* new_process=0;
    if (proc->arrival_time==os->timer){
      new_process=proc;
    }
    aux=aux->next;
    if (new_process) {
      printf("\tcreate pid:%d\n", new_process->pid);
      new_process=(FakeProcess*)List_detach(&os->processes, (ListItem*)new_process);
      FakeOS_createProcess(os, new_process);
      free(new_process);
    }
  }

  // scan waiting list, and put in ready all items whose event terminates
  aux=os->waiting.first;
  while(aux) {
    FakePCB* pcb=(FakePCB*)aux;
    aux=aux->next;
    ProcessEvent* e=(ProcessEvent*) pcb->events.first;
    printf("\twaiting pid: %d\n", pcb->pid);
    assert(e->type==IO);
    e->duration--;
    printf("\t\tremaining time:%d\n",e->duration);
    if (e->duration==0){
      printf("\t\tend burst\n");
      List_popFront(&pcb->events);
      free(e);
      List_detach(&os->waiting, (ListItem*)pcb);
      if (! pcb->events.first) {
        // kill process
        printf("\t\tend process\n");
        free(pcb);
      } else {
        //handle next event
        e=(ProcessEvent*) pcb->events.first;
        switch (e->type){
        case CPU:
          printf("\t\tmove to ready\n");
          List_pushBack(&os->ready, (ListItem*) pcb);

          break;
        case IO:
          printf("\t\tmove to waiting\n");
          List_pushBack(&os->waiting, (ListItem*) pcb);
          break;
        }
      }
    }
  }

  

  // decrement the duration of every pcb in running list
  // if event over, destroy event, free cpu
  // and reschedule process
  // if last event, destroy running

  
  ListItem* running_item = os->running.first;
  

  
  if (!os->running.first) {
    printf("\trunning pid: -1\n");
  }

  while (running_item) {

    ListItem* next_running_item = running_item->next;

    FakePCB* running = (FakePCB*) running_item;

    printf("\trunning pid: %d\n", running?running->pid:-1);

    ProcessEvent* e=(ProcessEvent*) running->events.first;
    assert(e->type==CPU);
   
    e->duration--;

    printf("\t\tremaining time:%d\n",e->duration);


      if (e->duration == 0) {

      printf("\t\tend burst\n");
      List_popFront(&running->events);
      //*************************************
      

      free(e);
      //libero la cpu

      List_detach(&os->running,(ListItem*)running);
      os->prev_quantum = e->predicted_burst;

    
      

      if (! running->events.first) {
        printf("\t\tend process\n");


      } else {
        e=(ProcessEvent*) running->events.first;
        switch (e->type){
        case CPU:
          printf("\t\tmove to ready\n");
          List_pushBack(&os->ready, (ListItem*) running);
          break;
        case IO:
          printf("\t\tmove to waiting\n");
          List_pushBack(&os->waiting, (ListItem*) running);
          break;
        }
      }
    }


     running_item = next_running_item;
      




  }


  // call schedule, if defined

  if (os->schedule_fn)
    
    (*os->schedule_fn)(os, os->schedule_args); 
        
  
  
 

  // if running not defined and ready queue not empty
  // put the first in ready to run
  //if (! os->running && os->ready.first) {
    if (os->ready.first) {
      //debugging
      ListItem* item = os->ready.first;
      printf("LISTA READY: ");
      printf("{ ");
       while (item) {
        FakePCB* fake = (FakePCB*) item;

        printf("%d ", fake->pid);
        item = item->next;
      }
      printf(" }\n");
      //ListItem* new_running = List_popFront(&os->ready);
      //List_pushBack(&os->running, new_running);
    
  }

  ++os->timer;

}

void FakeOS_destroy(FakeOS* os) {
}


