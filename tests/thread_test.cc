#include <stdio.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "thread.h"

#define TOTAL_THREADS  10
#define THREAD_SLEEP   4

using namespace ::testing;

/*
 * A Google testing class.
 */
class ThreadTest : public Test {
};

/*
 * A joinable thread.
 */
class JoinableThread : public Thread {
private:
    int _i;

private:
    void run(void) {
        sleep(random() % THREAD_SLEEP);
    }

public:
    JoinableThread(int i) : Thread(Joinable), _i(i) {
        printf("%3d: %s\n", _i, __PRETTY_FUNCTION__);
    }
    ~JoinableThread(void) {
        printf("%3d: %s\n", _i, __PRETTY_FUNCTION__);
    }
};

/*
 * A detached thread.
 */
class DetachedThread : public Thread {
private:
    int _i;

private:
    void run(void) {
        sleep(random() % THREAD_SLEEP);
    }

public:
    DetachedThread(int i) : _i(i) {
        printf("%3d: %s\n", _i, __PRETTY_FUNCTION__);
    }
    ~DetachedThread(void) {
        printf("%3d: %s\n", _i, __PRETTY_FUNCTION__);
    }
}; 

/*
 * Test unmanaged threads, aka. detached threads.
 */

TEST_F(ThreadTest, UnmanagedThread) {
    // Allocate whole bunch of unmanaged threads.
    for (int i = 0; i < TOTAL_THREADS; i++) {
        new DetachedThread(i);
    }

    // Application is free to do whatever such as processing GUI events.
    sleep(THREAD_SLEEP + 1);
}

/*
 * Test class Threads.
 */

TEST_F(ThreadTest, EmptyThread)
{
    Threads threads;
    EXPECT_EQ(0, threads.size());
    // Since there is no thread, wait() should return immediately.
    threads.wait();
    EXPECT_EQ(0, threads.size());
}

/*
 * Only joinable threads, aka. managed threads, will be added to the
 * class Threads.
 */
TEST_F(ThreadTest, JoinableThreads)
{
    Threads threads;
    for (int i = 0; i < TOTAL_THREADS; i++) {
        threads.add(new JoinableThread(i));
    }
    EXPECT_EQ(TOTAL_THREADS, threads.size());
    // wait() will wait and join all the joinable threads before
    // returning.  ie. wait() will block.
    threads.wait();
    EXPECT_EQ(0, threads.size());
}

/*
 * Unmanaged/Detached threads will not be added to the class Threads.
 */
TEST_F(ThreadTest, DetachedThreads)
{
    Threads threads;
    for (int i = 0; i < TOTAL_THREADS; i++) {
        threads.add(new DetachedThread(i));
    }
    // We only keep track of joinable threads.
    EXPECT_EQ(0, threads.size());
    // Since detached threads do not need to be joined, so we wait for
    // (THREAD_SLEEP + 1) secs for all the threads to finish.
    printf("Waiting for %d secs ...\n", THREAD_SLEEP);
    threads.wait(THREAD_SLEEP);
    EXPECT_EQ(0, threads.size());
}
