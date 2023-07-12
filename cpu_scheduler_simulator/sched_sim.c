#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"


FakeOS os;

typedef struct {
  int quantum;
  float alpha;
  int num_cpu;
  
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
  int num_cpu = args->num_cpu;


  // look for the first process in ready
  // if none, return
  if (! os->ready.first)
    return;
  
  for (int i=0; i < num_cpu; i++) {
    if (! os->ready.first) break;

    if (os->cpu_assignments[i] == 0) {

      //trovo l'elemento con il min burst e lo assegno ad una cpu disponibile

      ListItem* min_item = MinBurst(os);
      FakePCB* pcb = (FakePCB*) List_detach(&os->ready, min_item);

        os->cpu_assignments[i] = pcb->pid;
        pcb->cpu = i;
        //quindi c'è una lista di running
        //os->running=pcb;
        List_pushFront(&os->running, (ListItem*)pcb);

        assert(pcb->events.first);
        ProcessEvent* e = (ProcessEvent*)pcb->events.first;
        assert(e->type==CPU);

        //quantum prediction
        args->quantum = (alpha)*(e->duration) + (1 - alpha)*(args->quantum);
        args_ = args;
        printf("CURRENT QUANTUM: %d\n", args->quantum);


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
    }

  }
  
  

  
};

int main(int argc, char** argv) {
  FakeOS_init(&os);
  SchedSJFArgs ssjf_args;
  ssjf_args.quantum=5;
  ssjf_args.alpha = 0.5;
  ssjf_args.num_cpu = 2;
  os.schedule_args=&ssjf_args;
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
  while(os.running.first
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
}
