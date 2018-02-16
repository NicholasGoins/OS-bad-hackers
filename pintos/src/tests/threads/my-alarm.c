/* Tests timer_sleep(-100).  Only requirement is that it not crash. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

extern struct thread *blocked_queue[20];

void
test_my_alarm (void) 
{
  timer_sleep (1000);
  printf("test_my_alarm(): Thread actually awaken at: %llu\n", timer_ticks());
  timer_sleep (1000);
  printf("test_my_alarm(): Thread actually awaken at: %llu\n", timer_ticks());
  timer_sleep (1000);
  printf("test_my_alarm(): Thread actually awaken at: %llu\n", timer_ticks());
  pass ();
}
