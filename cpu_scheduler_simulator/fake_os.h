#include "fake_process.h"
#include "linked_list.h"
#pragma once
#define MAX_CPU 5


typedef struct {
  ListItem list;
  int pid;
  ListHead events;
  int cpu;
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

  int cpu_assignments[MAX_CPU];
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);
