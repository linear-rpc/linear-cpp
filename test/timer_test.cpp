#include "gtest/gtest.h"

#include "test_common.h"

#include <unistd.h>

#include "linear/timer.h"

typedef LinearTest TimerTest;

void fakeOnTimer(void* args) {
}

TEST_F(TimerTest, createDelete) {
  linear::Timer* timer1 = new linear::Timer();
  timer1->Start(fakeOnTimer, 0, NULL);
  timer1->Stop();
  delete timer1;
  linear::Timer* timer2 = new linear::Timer();
  timer2->Start(fakeOnTimer, 0, NULL);
  delete timer2;
}

void deleteOnTimer(void* args) {
  linear::Timer* timer = reinterpret_cast<linear::Timer*>(args);
  delete timer;
}

TEST_F(TimerTest, deleteOnTimer) {
  linear::Timer* timer = new linear::Timer();
  timer->Start(deleteOnTimer, 10, timer);
  usleep(100);
}

void onTimer(void* args) {
  int* count = reinterpret_cast<int*>(args);
  (*count)++;
}

TEST_F(TimerTest, call0sec) {
  int count = 0;
  linear::Timer timer;

  timer.Start(onTimer, 0, &count);
  sleep(1);
  ASSERT_EQ(1, count);
  sleep(1);
  ASSERT_EQ(1, count);
  timer.Stop();
}

TEST_F(TimerTest, heap) {
  int count = 0, timer_msec = 10, wait_msec = 30 * 1000;
  linear::Timer* timer = new linear::Timer();

  timer->Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(1, count);

  linear::Timer timer2; // operator=
  timer2 = *timer;
  timer2.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(2, count);

  linear::Timer timer3(timer2); // copy constructor
  timer3.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(3, count);

  timer->Stop();
  delete timer;
}

TEST_F(TimerTest, stack) {
  int count = 0, timer_msec = 10, wait_msec = 30 * 1000;
  linear::Timer timer;

  timer.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(1, count);

  linear::Timer timer2; // operator=
  timer2 = timer;
  timer2.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(2, count);

  linear::Timer timer3(timer2); // copy constructor
  timer3.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(3, count);

  timer.Stop();
}

TEST_F(TimerTest, loop) {
  int count = 0, timer_msec = 10, wait_msec = 30 * 1000;
  linear::EventLoop loop;
  linear::Timer timer(loop);

  timer.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(1, count);

  linear::Timer timer2; // operator=
  timer2 = timer;
  timer2.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(2, count);

  linear::Timer timer3(timer2); // copy constructor
  timer3.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(3, count);

  timer.Stop();
}

TEST_F(TimerTest, newdeleteLoop) {
  int count = 0, timer_msec = 10, wait_msec = 30 * 1000;
  linear::EventLoop* loop = new linear::EventLoop();
  linear::Timer timer(*loop);

  timer.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(1, count);

  linear::Timer timer2; // operator=
  timer2 = timer;
  timer2.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(2, count);

  delete loop; // must not segv (managed by internal shared_ptr)

  linear::Timer timer3(timer2); // copy constructor
  timer3.Start(onTimer, timer_msec, &count);
  usleep(wait_msec);
  ASSERT_EQ(3, count);

  timer.Stop();
}
