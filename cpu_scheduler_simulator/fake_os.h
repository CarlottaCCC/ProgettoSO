#include "fake_process.h"
#include "linked_list.h"
#pragma once



typedef struct {
  ListItem list;
  int pid;
  ListHead events;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

typedef struct FakeOS{
  //FakePCB* running;
  ListHead running;

  ListHead ready;
  ListHead waiting;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;
  
  ListHead processes;
  int num_cpu;

  int count;
  int prev_quantum;
  
  int cpu_assignments[];
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);
