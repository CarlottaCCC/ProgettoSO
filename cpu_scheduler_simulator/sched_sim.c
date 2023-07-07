#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

FakeOS os;

typedef struct {
  int quantum;
  int prev_quantum;
  float alpha;
} SchedSJFArgs;

//Funzione che trova il PCB con il CPU burst minimo nella ready queue
ListItem* MinBurst(FakeOS* os) {
  if (! os->ready.first)
    return 0;

  int min = 10000;
  ListItem* min_item;
  int duration;
  ListItem* node = os->ready.first;

  while(node) {

    FakePCB* pcb =  (FakePCB*) node;
    ProcessEvent* e = (ProcessEvent*)pcb->events.first;
    //duration = e->p_quantum;
    duration = e->duration;

    if (duration < min) {
      min = duration;
      min_item = node;
    }

    node = node->next;
    }

    return min_item;
}


void schedSJF(FakeOS* os, void* args_){
  SchedSJFArgs* args=(SchedSJFArgs*)args_;
  float alpha = args->alpha;
  int q_current = args->quantum;
  int prev_quantum = args->prev_quantum;
  int q_new;

  // look for the first process in ready
  // if none, return
  if (! os->ready.first)
    return;

  //trovo l'elemento con il min burst
  ListItem* min_item = MinBurst(os);
  FakePCB* pcb = (FakePCB*) List_detach(&os->ready, min_item);


  //FakePCB* pcb=(FakePCB*) List_popFront(&os->ready);
  os->running=pcb;
  
  assert(pcb->events.first);
  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
  assert(e->type==CPU);

  // look at the first event
  // if duration>quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  if (e->duration>args->quantum) {
    ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev=qe->list.next=0;
    qe->type=CPU;
    qe->duration=args->quantum;
    e->duration-=args->quantum;
    List_pushFront(&pcb->events, (ListItem*)qe);
  }

  //aggiornameno quantum
  args->prev_quantum = q_current;
  q_new = alpha * (q_current) + (1-alpha)*(prev_quantum);
  args->quantum = q_new;
  args_ = (void*) args;
  
};

int main(int argc, char** argv) {
  FakeOS_init(&os);
  SchedSJFArgs srr_args;
  srr_args.quantum=5;
  srr_args.prev_quantum = 0;
  srr_args.alpha = 0.5;
  os.schedule_args=&srr_args;
  os.schedule_fn=schedSJF;
  
  for (int i=1; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading [%s], pid: %d, events:%d",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }

  printf("num processes in queue %d\n", os.processes.size);
  while(os.running
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
}
