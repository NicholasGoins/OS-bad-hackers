/* Tests timer_sleep(-100).  Only requirement is that it not crash. */

#include <stdio.h>
#include "tests/threads/tests.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "devices/timer.h"

void
test_alarm_mega (void) 
{
  int x = 0;
  while(x<71){

  timer_sleep (1);
  pass ();
  x++;
  }
}
