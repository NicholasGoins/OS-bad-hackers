#include "devices/timer.h"
#include <debug.h>
#include <inttypes.h>
#include <round.h>
#include <stdio.h>
#include "devices/pit.h"
#include "threads/interrupt.h"
#include "threads/synch.h"
#include "threads/thread.h" 
/* See [8254] for hardware details of the 8254 timer chip. */

#if TIMER_FREQ < 19
#error 8254 timer requires TIMER_FREQ >= 19
#endif
#if TIMER_FREQ > 1000
#error TIMER_FREQ <= 1000 recommended
#endif

/* Number of timer ticks since OS booted. */
static int64_t ticks;
struct thread *blocked_q[10] = {0,0,0,0,0,0,0,0,0,0};
struct thread *dummy_thread;
#define THREAD_PTR_NULL (struct thread *)0


//insert a new thrad in the blocked check for every order chronogiaclly orderder

void my_list_insert (struct thread *list[], struct thread *t) {
	for (int i = 0; i < 10; i++) {
		if (list[i] == THREAD_PTR_NULL) {
			printf("list insert...");
			list[i] = t;
			break;
		}
	}
}

void my_list_remove(struct thread * list[], struct thread *t) {
	for (int i = 0; i < 10; i++) {
		if (list[i]->tid == t->tid) {
			printf("list removing....");
			list[i] = THREAD_PTR_NULL;
			break;
		}
	}
}

struct thread * my_list_pop (struct thread * list[])
{
	static int count = 0;
	while (count < 10) {
		if (list[count] != THREAD_PTR_NULL) {
			return list[count++];
		}
		count++;
	}
	count = 0;
	return THREAD_PTR_NULL;
}

/* Number of loops per timer tick.
   Initialized by timer_calibrate(). */
static unsigned loops_per_tick;
static intr_handler_func timer_interrupt;
static bool too_many_loops (unsigned loops);
static void busy_wait (int64_t loops);
static void real_time_sleep (int64_t num, int32_t denom);
static void real_time_delay (int64_t num, int32_t denom);

/* Sets up the timer to interrupt TIMER_FREQ times per second,
   and registers the corresponding interrupt. */
void
timer_init (void) 
{
                    
	 blocked_q[0] = thread_current();
  pit_configure_channel (0, 2, TIMER_FREQ);
  intr_register_ext (0x20, timer_interrupt, "8254 Timer");
 // blocked_queue[0] = dummy_thread;
 // list_insert(blocked_queue[0],&dummy_thread);
}

/* Calibrates loops_per_tick, used to implement brief delays. */
void
timer_calibrate (void) 
{
  unsigned high_bit, test_bit;

  ASSERT (intr_get_level () == INTR_ON);
  printf ("Calibrating timer...  ");

  /* Approximate loops_per_tick as the largest power-of-two
     still less than one timer tick. */
  loops_per_tick = 1u << 10;
  while (!too_many_loops (loops_per_tick << 1)) 
    {
      loops_per_tick <<= 1;
      ASSERT (loops_per_tick != 0);
    }

  /* Refine the next 8 bits of loops_per_tick. */
  high_bit = loops_per_tick;
  for (test_bit = high_bit >> 1; test_bit != high_bit >> 10; test_bit >>= 1)
    if (!too_many_loops (high_bit | test_bit))
      loops_per_tick |= test_bit;

  printf ("%'"PRIu64" loops/s.\n", (uint64_t) loops_per_tick * TIMER_FREQ);
}

/* Returns the number of timer ticks since the OS booted. */
int64_t
timer_ticks (void) 
{
  enum intr_level old_level = intr_disable ();
  int64_t t = ticks;
  intr_set_level (old_level);
  return t;
}

/* Returns the number of timer ticks elapsed since THEN, which
   should be a value once returned by timer_ticks(). */
int64_t
timer_elapsed (int64_t then) 
{
  return timer_ticks () - then;
}

/* Sleeps for approximately TICKS timer ticks.  Interrupts must
   be turned on.  thread_yield does noe specify what it volun give up*/
void
timer_sleep (int64_t ticks) 
{
  int64_t start = timer_ticks ();

  ASSERT (intr_get_level () == INTR_ON);
  // check wherher an event has happened  constantly checking whether some event has happened or not 
  // need to force the thread to give up its time using thread_block()
  // need to use some form  blocking 
  while (timer_elapsed (start) < ticks) 
    thread_yield ();
}

// mytimer_sleep
my_timer_sleep (int64_t ticks)
{
	int64_t start = timer_ticks();
struct thread *curr_thread = thread_current();
curr_thread->wakeup_ticks = start+ticks;
curr_thread->waiting_for = TIME_EVENT;

//my_list_insert(blocked_q, curr_thread);

printf("my_timer_sleep(): Current ticks is %llu\n", start);
//printf("my_timer_sleep(): Thread set to wake at: %llu\n", blocked_queue[0]->wakeup_ticks);
ASSERT(intr_get_level() == INTR_ON);

enum intr_level old_level = intr_disable();
//printf("inserting into the list");
//list_wakeup_ticks_insert(&blocked_list, &curr_thread->elem);
my_list_insert(blocked_q, curr_thread);
//thread_block();
//printf("thread blocked, let's goo!!!!");
sema_init(&curr_thread->timerevent_sema, 0);
sema_down(&curr_thread->timerevent_sema);
intr_set_level(old_level);	
          
	 //printf("inside before block");   
	  //int64_t start = timer_ticks ();
           //struct thread *t = thread_current();

      	   //t->wakeup_ticks = start + ticks; // set up some of the fields

	   //t->waiting_for = TIME_EVENT;  // what the thread is waiting for 

	  //ASSERT (intr_get_level () == INTR_ON);
   /// insert into list	   
	      // check wherher an event has happened  constantly checking whether some event has happened or not
	       // need to force the thread to give up its time using thread_block()
	       // need to use some form  blocking
          //blocked_q[0]= t; // holds pionters to all threads 
          //enum intr_level old_level = intr_disable();// disable the interrupt and return the current stae of the interrupt
	  //thread_block();
	  //intr_set_level (old_level);
	  
}
//
void
timer_msleep (int64_t ms) 
{
  real_time_sleep (ms, 1000);
}

/* Sleeps for approximately US microseconds.  Interrupts must be
   turned on. */
void
timer_usleep (int64_t us) 
{
  real_time_sleep (us, 1000 * 1000);
}

/* Sleeps for approximately NS nanoseconds.  Interrupts must be
   turned on. */
void
timer_nsleep (int64_t ns) 
{
  real_time_sleep (ns, 1000 * 1000 * 1000);
}

/* Busy-waits for approximately MS milliseconds.  Interrupts need
   not be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_msleep()
   instead if interrupts are enabled. */
void
timer_mdelay (int64_t ms) 
{
  real_time_delay (ms, 1000);
}

/* Sleeps for approximately US microseconds.  Interrupts need not
   be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_usleep()
   instead if interrupts are enabled. */
void
timer_udelay (int64_t us) 
{
  real_time_delay (us, 1000 * 1000);
}

/* Sleeps execution for approximately NS nanoseconds.  Interrupts
   need not be turned on.

   Busy waiting wastes CPU cycles, and busy waiting with
   interrupts off for the interval between timer ticks or longer
   will cause timer ticks to be lost.  Thus, use timer_nsleep()
   instead if interrupts are enabled.*/
void
timer_ndelay (int64_t ns) 
{
  real_time_delay (ns, 1000 * 1000 * 1000);
}

/* Prints timer statistics. */
void
timer_print_stats (void) 
{
  printf ("Timer: %"PRId64" ticks\n", timer_ticks ());
}

/* Timer interrupt handler. */
static void
timer_interrupt (struct intr_frame *args UNUSED)
{
	// my work
	struct thread * next_thread;
  ticks++;
  thread_tick ();
 next_thread = my_list_pop(blocked_q);
 
 if (next_thread != THREAD_PTR_NULL) {
	 if (next_thread->waiting_for == TIME_EVENT) {
		if (next_thread->status == THREAD_BLOCKED) {
			printf("Thread to be awaken at %llu\n", next_thread->wakeup_ticks);
			if (ticks >= next_thread->wakeup_ticks) {
				printf("timer_interrup(): Thread ready to be unblocked\n");
				sema_up(&next_thread->wakeup_ticks);
				//thread_unblock(next_thread);
				my_list_remove(blocked_q, next_thread);
			}
		} 
	 }
 }
 //if(blocked_q[0]->waiting_for == TIME_EVENT ){
   
                   
 // if( blocked_q[0] -> status == THREAD_BLOCKED){
 // 		if(ticks >= blocked_q[0]->wakeup_ticks){
  		
 //                printf("timer inteerupt(): Thread ready t be unblocked\n");
  //               thread_unblock(blocked_q[0]);
//		 }
//	}
 // }
  
 }


/* Returns true if LOOPS iterations waits for more than one timer
   tick, otherwise false. */
static bool
too_many_loops (unsigned loops) 
{
  /* Wait for a timer tick. */
  int64_t start = ticks;
  while (ticks == start)
    barrier ();

  /* Run LOOPS loops. */
  start = ticks;
  busy_wait (loops);

  /* If the tick count changed, we iterated too long. */
  barrier ();
  return start != ticks;
}

/* Iterates through a simple loop LOOPS times, for implementing
   brief delays.

   Marked NO_INLINE because code alignment can significantly
   affect timings, so that if this function was inlined
   differently in different places the results would be difficult
   to predict. */
static void NO_INLINE
busy_wait (int64_t loops) 
{
  while (loops-- > 0)
    barrier ();
}

/* Sleep for approximately NUM/DENOM seconds. */
static void
real_time_sleep (int64_t num, int32_t denom) 
{
  /* Convert NUM/DENOM seconds into timer ticks, rounding down.
          
        (NUM / DENOM) s          
     ---------------------- = NUM * TIMER_FREQ / DENOM ticks. 
     1 s / TIMER_FREQ ticks
  */
  int64_t ticks = num * TIMER_FREQ / denom;

  ASSERT (intr_get_level () == INTR_ON);
  if (ticks > 0)
    {
      /* We're waiting for at least one full timer tick.  Use
         timer_sleep() because it will yield the CPU to other
         processes. */                
      timer_sleep (ticks); 
    }
  else 
    {
      /* Otherwise, use a busy-wait loop for more accurate
         sub-tick timing. */
      real_time_delay (num, denom); 
    }
}

/* Busy-wait for approximately NUM/DENOM seconds. */
static void
real_time_delay (int64_t num, int32_t denom)
{
  /* Scale the numerator and denominator down by 1000 to avoid
     the possibility of overflow. */
  ASSERT (denom % 1000 == 0);
  busy_wait (loops_per_tick * num / 1000 * TIMER_FREQ / (denom / 1000)); 
}
