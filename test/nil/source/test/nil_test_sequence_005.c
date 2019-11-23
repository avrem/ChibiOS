/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "hal.h"
#include "nil_test_root.h"

/**
 * @file    nil_test_sequence_005.c
 * @brief   Test Sequence 005 code.
 *
 * @page nil_test_sequence_005 [5] Event Sources and Event Flags
 *
 * File: @ref nil_test_sequence_005.c
 *
 * <h2>Description</h2>
 * This module implements the test sequence for the Events subsystem.
 *
 * <h2>Conditions</h2>
 * This sequence is only executed if the following preprocessor condition
 * evaluates to true:
 * - CH_CFG_USE_EVENTS
 * .
 *
 * <h2>Test Cases</h2>
 * - @subpage nil_test_005_001
 * - @subpage nil_test_005_002
 * - @subpage nil_test_005_003
 * - @subpage nil_test_005_004
 * - @subpage nil_test_005_005
 * - @subpage nil_test_005_006
 * - @subpage nil_test_005_007
 * .
 */

#if (CH_CFG_USE_EVENTS) || defined(__DOXYGEN__)

/****************************************************************************
 * Shared code.
 ****************************************************************************/

static EVENTSOURCE_DECL(es1);
static EVENTSOURCE_DECL(es2);

static void h1(eventid_t id) {(void)id;test_emit_token('A');}
static void h2(eventid_t id) {(void)id;test_emit_token('B');}
static void h3(eventid_t id) {(void)id;test_emit_token('C');}
static ROMCONST evhandler_t evhndl[] = {h1, h2, h3};

/*
 * Direct events thread.
 */
static THD_FUNCTION(evtthd1, p) {

  chThdSleepMilliseconds(50);
  chEvtSignal((thread_t *)p, 1);
}

/*
 * Broadcaster thread.
 */
static THD_FUNCTION(evtthd2, p) {

  (void)p;
  chEvtBroadcast(&es1);
  chThdSleepMilliseconds(50);
  chEvtBroadcast(&es2);
}

/****************************************************************************
 * Test cases.
 ****************************************************************************/

/**
 * @page nil_test_005_001 [5.1] Events registration
 *
 * <h2>Description</h2>
 * Two event listeners are registered on an event source and then
 * unregistered in the same order.<br> The test expects that the even
 * source has listeners after the registrations and after the first
 * unregistration, then, after the second unegistration, the test
 * expects no more listeners.
 *
 * <h2>Test Steps</h2>
 * - [5.1.1] An Event Source is initialized.
 * - [5.1.2] Two Event Listeners are registered on the Event Source,
 *   the Event Source is tested to have listeners.
 * - [5.1.3] An Event Listener is unregistered, the Event Source must
 *   still have listeners.
 * - [5.1.4] An Event Listener is unregistered, the Event Source must
 *   not have listeners.
 * .
 */

static void nil_test_005_001_execute(void) {
  event_listener_t el1, el2;

  /* [5.1.1] An Event Source is initialized.*/
  test_set_step(1);
  {
    chEvtObjectInit(&es1);
  }
  test_end_step(1);

  /* [5.1.2] Two Event Listeners are registered on the Event Source,
     the Event Source is tested to have listeners.*/
  test_set_step(2);
  {
    chEvtRegisterMask(&es1, &el1, 1);
    chEvtRegisterMask(&es1, &el2, 2);
    test_assert_lock(chEvtIsListeningI(&es1), "no listener");
  }
  test_end_step(2);

  /* [5.1.3] An Event Listener is unregistered, the Event Source must
     still have listeners.*/
  test_set_step(3);
  {
    chEvtUnregister(&es1, &el1);
    test_assert_lock(chEvtIsListeningI(&es1), "no listener");
  }
  test_end_step(3);

  /* [5.1.4] An Event Listener is unregistered, the Event Source must
     not have listeners.*/
  test_set_step(4);
  {
    chEvtUnregister(&es1, &el2);
    test_assert_lock(!chEvtIsListeningI(&es1), "stuck listener");
  }
  test_end_step(4);
}

static const testcase_t nil_test_005_001 = {
  "Events registration",
  NULL,
  NULL,
  nil_test_005_001_execute
};

/**
 * @page nil_test_005_002 [5.2] Event Flags dispatching
 *
 * <h2>Description</h2>
 * The test dispatches three event flags and verifies that the
 * associated event handlers are invoked in LSb-first order.
 *
 * <h2>Test Steps</h2>
 * - [5.2.1] Three evenf flag bits are raised then chEvtDispatch() is
 *   invoked, the sequence of handlers calls is tested.
 * .
 */

static void nil_test_005_002_setup(void) {
  chEvtGetAndClearEvents(ALL_EVENTS);
}

static void nil_test_005_002_execute(void) {

  /* [5.2.1] Three evenf flag bits are raised then chEvtDispatch() is
     invoked, the sequence of handlers calls is tested.*/
  test_set_step(1);
  {
    chEvtDispatch(evhndl, 7);
    test_assert_sequence("ABC", "invalid sequence");
  }
  test_end_step(1);
}

static const testcase_t nil_test_005_002 = {
  "Event Flags dispatching",
  nil_test_005_002_setup,
  NULL,
  nil_test_005_002_execute
};

/**
 * @page nil_test_005_003 [5.3] Events Flags wait using chEvtWaitOne()
 *
 * <h2>Description</h2>
 * Functionality of chEvtWaitOne() is tested under various scenarios.
 *
 * <h2>Test Steps</h2>
 * - [5.3.1] Setting three event flags.
 * - [5.3.2] Calling chEvtWaitOne() three times, each time a single
 *   flag must be returned in order of priority.
 * - [5.3.3] Getting current time and starting a signaler thread, the
 *   thread will set an event flag after 50mS.
 * - [5.3.4] Calling chEvtWaitOne() then verifying that the event has
 *   been received after 50mS and that the event flags mask has been
 *   emptied.
 * .
 */

static void nil_test_005_003_setup(void) {
  chEvtGetAndClearEvents(ALL_EVENTS);
}

static void nil_test_005_003_execute(void) {
  eventmask_t m;
  systime_t target_time;
  thread_t *tp;

  /* [5.3.1] Setting three event flags.*/
  test_set_step(1);
  {
    chEvtAddEvents(7);
  }
  test_end_step(1);

  /* [5.3.2] Calling chEvtWaitOne() three times, each time a single
     flag must be returned in order of priority.*/
  test_set_step(2);
  {
    m = chEvtWaitOne(ALL_EVENTS);
    test_assert(m == 1, "single event error");
    m = chEvtWaitOne(ALL_EVENTS);
    test_assert(m == 2, "single event error");
    m = chEvtWaitOne(ALL_EVENTS);
    test_assert(m == 4, "single event error");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
  }
  test_end_step(2);

  /* [5.3.3] Getting current time and starting a signaler thread, the
     thread will set an event flag after 50mS.*/
  test_set_step(3);
  {
    target_time = chTimeAddX(test_wait_tick(), TIME_MS2I(50));
    thread_config_t tc = {
      chThdGetPriorityX() + 1,
      "event1",
      wa_common,
      THD_WORKING_AREA_END(wa_common),
      evtthd1,
      chThdGetSelfX()
    };
    tp = chThdCreate(&tc);
  }
  test_end_step(3);

  /* [5.3.4] Calling chEvtWaitOne() then verifying that the event has
     been received after 50mS and that the event flags mask has been
     emptied.*/
  test_set_step(4);
  {
    m = chEvtWaitOne(ALL_EVENTS);
    test_assert_time_window(target_time,
                            chTimeAddX(target_time, ALLOWED_DELAY),
                            "out of time window");
    test_assert(m == 1, "event flag error");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
    chThdWait(tp);
  }
  test_end_step(4);
}

static const testcase_t nil_test_005_003 = {
  "Events Flags wait using chEvtWaitOne()",
  nil_test_005_003_setup,
  NULL,
  nil_test_005_003_execute
};

/**
 * @page nil_test_005_004 [5.4] Events Flags wait using chEvtWaitAny()
 *
 * <h2>Description</h2>
 * Functionality of chEvtWaitAny() is tested under various scenarios.
 *
 * <h2>Test Steps</h2>
 * - [5.4.1] Setting two, non contiguous, event flags.
 * - [5.4.2] Calling chEvtWaitAny() one time, the two flags must be
 *   returned.
 * - [5.4.3] Getting current time and starting a signaler thread, the
 *   thread will set an event flag after 50mS.
 * - [5.4.4] Calling chEvtWaitAny() then verifying that the event has
 *   been received after 50mS and that the event flags mask has been
 *   emptied.
 * .
 */

static void nil_test_005_004_setup(void) {
  chEvtGetAndClearEvents(ALL_EVENTS);
}

static void nil_test_005_004_execute(void) {
  eventmask_t m;
  systime_t target_time;
  thread_t *tp;

  /* [5.4.1] Setting two, non contiguous, event flags.*/
  test_set_step(1);
  {
    chEvtAddEvents(5);
  }
  test_end_step(1);

  /* [5.4.2] Calling chEvtWaitAny() one time, the two flags must be
     returned.*/
  test_set_step(2);
  {
    m = chEvtWaitAny(ALL_EVENTS);
    test_assert(m == 5, "unexpected pending bit");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
  }
  test_end_step(2);

  /* [5.4.3] Getting current time and starting a signaler thread, the
     thread will set an event flag after 50mS.*/
  test_set_step(3);
  {
    target_time = chTimeAddX(test_wait_tick(), TIME_MS2I(50));
    thread_config_t tc = {
      chThdGetPriorityX() + 1,
      "event1",
      wa_common,
      THD_WORKING_AREA_END(wa_common),
      evtthd1,
      chThdGetSelfX()
    };
    tp = chThdCreate(&tc);
  }
  test_end_step(3);

  /* [5.4.4] Calling chEvtWaitAny() then verifying that the event has
     been received after 50mS and that the event flags mask has been
     emptied.*/
  test_set_step(4);
  {
    m = chEvtWaitAny(ALL_EVENTS);
    test_assert_time_window(target_time,
                            chTimeAddX(target_time, ALLOWED_DELAY),
                            "out of time window");
    test_assert(m == 1, "event flag error");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
    chThdWait(tp);
  }
  test_end_step(4);
}

static const testcase_t nil_test_005_004 = {
  "Events Flags wait using chEvtWaitAny()",
  nil_test_005_004_setup,
  NULL,
  nil_test_005_004_execute
};

/**
 * @page nil_test_005_005 [5.5] Events Flags wait using chEvtWaitAll()
 *
 * <h2>Description</h2>
 * Functionality of chEvtWaitAll() is tested under various scenarios.
 *
 * <h2>Test Steps</h2>
 * - [5.5.1] Setting two, non contiguous, event flags.
 * - [5.5.2] Calling chEvtWaitAll() one time, the two flags must be
 *   returned.
 * - [5.5.3] Setting one event flag.
 * - [5.5.4] Getting current time and starting a signaler thread, the
 *   thread will set another event flag after 50mS.
 * - [5.5.5] Calling chEvtWaitAll() then verifying that both event
 *   flags have been received after 50mS and that the event flags mask
 *   has been emptied.
 * .
 */

static void nil_test_005_005_setup(void) {
  chEvtGetAndClearEvents(ALL_EVENTS);
}

static void nil_test_005_005_execute(void) {
  eventmask_t m;
  systime_t target_time;
  thread_t *tp;

  /* [5.5.1] Setting two, non contiguous, event flags.*/
  test_set_step(1);
  {
    chEvtAddEvents(5);
  }
  test_end_step(1);

  /* [5.5.2] Calling chEvtWaitAll() one time, the two flags must be
     returned.*/
  test_set_step(2);
  {
    m = chEvtWaitAll(5);
    test_assert(m == 5, "unexpected pending bit");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
  }
  test_end_step(2);

  /* [5.5.3] Setting one event flag.*/
  test_set_step(3);
  {
    chEvtAddEvents(4);
  }
  test_end_step(3);

  /* [5.5.4] Getting current time and starting a signaler thread, the
     thread will set another event flag after 50mS.*/
  test_set_step(4);
  {
    target_time = chTimeAddX(test_wait_tick(), TIME_MS2I(50));
    thread_config_t tc = {
      chThdGetPriorityX() + 1,
      "event1",
      wa_common,
      THD_WORKING_AREA_END(wa_common),
      evtthd1,
      chThdGetSelfX()
    };
    tp = chThdCreate(&tc);
  }
  test_end_step(4);

  /* [5.5.5] Calling chEvtWaitAll() then verifying that both event
     flags have been received after 50mS and that the event flags mask
     has been emptied.*/
  test_set_step(5);
  {
    m = chEvtWaitAll(5);
    test_assert_time_window(target_time,
                            chTimeAddX(target_time, ALLOWED_DELAY),
                            "out of time window");
    test_assert(m == 5, "event flags error");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
    chThdWait(tp);
  }
  test_end_step(5);
}

static const testcase_t nil_test_005_005 = {
  "Events Flags wait using chEvtWaitAll()",
  nil_test_005_005_setup,
  NULL,
  nil_test_005_005_execute
};

/**
 * @page nil_test_005_006 [5.6] Events Flags wait timeouts
 *
 * <h2>Description</h2>
 * Timeout functionality is tested for chEvtWaitOneTimeout(),
 * chEvtWaitAnyTimeout() and chEvtWaitAllTimeout().
 *
 * <h2>Test Steps</h2>
 * - [5.6.1] The functions are invoked first with TIME_IMMEDIATE
 *   timeout, the timeout condition is tested.
 * - [5.6.2] The functions are invoked first with a 50mS timeout, the
 *   timeout condition is tested.
 * .
 */

static void nil_test_005_006_setup(void) {
  chEvtGetAndClearEvents(ALL_EVENTS);
}

static void nil_test_005_006_execute(void) {
  eventmask_t m;

  /* [5.6.1] The functions are invoked first with TIME_IMMEDIATE
     timeout, the timeout condition is tested.*/
  test_set_step(1);
  {
    m = chEvtWaitOneTimeout(ALL_EVENTS, TIME_IMMEDIATE);
    test_assert(m == 0, "spurious event");
    m = chEvtWaitAnyTimeout(ALL_EVENTS, TIME_IMMEDIATE);
    test_assert(m == 0, "spurious event");
    m = chEvtWaitAllTimeout(ALL_EVENTS, TIME_IMMEDIATE);
    test_assert(m == 0, "spurious event");
  }
  test_end_step(1);

  /* [5.6.2] The functions are invoked first with a 50mS timeout, the
     timeout condition is tested.*/
  test_set_step(2);
  {
    m = chEvtWaitOneTimeout(ALL_EVENTS, TIME_MS2I(50));
    test_assert(m == 0, "spurious event");
    m = chEvtWaitAnyTimeout(ALL_EVENTS, TIME_MS2I(50));
    test_assert(m == 0, "spurious event");
    m = chEvtWaitAllTimeout(ALL_EVENTS, TIME_MS2I(50));
    test_assert(m == 0, "spurious event");
  }
  test_end_step(2);
}

static const testcase_t nil_test_005_006 = {
  "Events Flags wait timeouts",
  nil_test_005_006_setup,
  NULL,
  nil_test_005_006_execute
};

/**
 * @page nil_test_005_007 [5.7] Broadcasting using chEvtBroadcast()
 *
 * <h2>Description</h2>
 * Functionality of chEvtBroadcast() is tested.
 *
 * <h2>Test Steps</h2>
 * - [5.7.1] Registering on two event sources associating them with
 *   flags 1 and 4.
 * - [5.7.2] Getting current time and starting a broadcaster thread,
 *   the thread broadcast the first Event Source immediately and the
 *   other after 50mS.
 * - [5.7.3] Calling chEvtWaitAll() then verifying that both event
 *   flags have been received after 50mS and that the event flags mask
 *   has been emptied.
 * - [5.7.4] Unregistering from the Event Sources.
 * .
 */

static void nil_test_005_007_setup(void) {
  chEvtGetAndClearEvents(ALL_EVENTS);
  chEvtObjectInit(&es1);
  chEvtObjectInit(&es2);
}

static void nil_test_005_007_execute(void) {
  eventmask_t m;
  event_listener_t el1, el2;
  systime_t target_time;
  thread_t *tp;

  /* [5.7.1] Registering on two event sources associating them with
     flags 1 and 4.*/
  test_set_step(1);
  {
    chEvtRegisterMask(&es1, &el1, 1);
    chEvtRegisterMask(&es2, &el2, 4);
  }
  test_end_step(1);

  /* [5.7.2] Getting current time and starting a broadcaster thread,
     the thread broadcast the first Event Source immediately and the
     other after 50mS.*/
  test_set_step(2);
  {
    target_time = chTimeAddX(test_wait_tick(), TIME_MS2I(50));
    thread_config_t tc = {
      chThdGetPriorityX() + 1,
      "event2",
      wa_common,
      THD_WORKING_AREA_END(wa_common),
      evtthd2,
      NULL
    };
    tp = chThdCreate(&tc);
  }
  test_end_step(2);

  /* [5.7.3] Calling chEvtWaitAll() then verifying that both event
     flags have been received after 50mS and that the event flags mask
     has been emptied.*/
  test_set_step(3);
  {
    m = chEvtWaitAll(5);
    test_assert_time_window(target_time,
                            chTimeAddX(target_time, ALLOWED_DELAY),
                            "out of time window");
    m = chEvtGetAndClearEvents(ALL_EVENTS);
    test_assert(m == 0, "stuck event");
    chThdWait(tp);
  }
  test_end_step(3);

  /* [5.7.4] Unregistering from the Event Sources.*/
  test_set_step(4);
  {
    chEvtUnregister(&es1, &el1);
    chEvtUnregister(&es2, &el2);
    test_assert(!chEvtIsListeningI(&es1), "stuck listener");
    test_assert(!chEvtIsListeningI(&es2), "stuck listener");
  }
  test_end_step(4);
}

static const testcase_t nil_test_005_007 = {
  "Broadcasting using chEvtBroadcast()",
  nil_test_005_007_setup,
  NULL,
  nil_test_005_007_execute
};

/****************************************************************************
 * Exported data.
 ****************************************************************************/

/**
 * @brief   Array of test cases.
 */
const testcase_t * const nil_test_sequence_005_array[] = {
  &nil_test_005_001,
  &nil_test_005_002,
  &nil_test_005_003,
  &nil_test_005_004,
  &nil_test_005_005,
  &nil_test_005_006,
  &nil_test_005_007,
  NULL
};

/**
 * @brief   Event Sources and Event Flags.
 */
const testsequence_t nil_test_sequence_005 = {
  "Event Sources and Event Flags",
  nil_test_sequence_005_array
};

#endif /* CH_CFG_USE_EVENTS */
