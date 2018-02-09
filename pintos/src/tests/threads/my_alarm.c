/* Tests timer_sleep(-100).  Only requirement is that it not crash. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

void
test_my_alarm (void) 
{
uint64_t curr_ticks;

  curr_ticks = timer_ticks();
  my_timer_sleep (1000);
  printf("Time elapsed %ul\n",timer_elapsed(curr_ticks));
  
}
