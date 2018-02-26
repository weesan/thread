#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <gtest/gtest.h>
#include "thread.h"

#define THREAD_SLEEP      2

#define TP_TEST(tp_size, total_task)                                          \
    TEST_F(ThreadPoolTest, TP_##tp_size##_Task_##total_task)                  \
    {                                                                         \
        threadPoolWithTasks(tp_size, total_task);                             \
    }

using namespace ::testing;

/*
 * A Google testing class.
 */
class ThreadPoolTest : public Test {
};

/*
 * A sleep task whose main task is simply to sleep a random moment.
 */
class SleepTask : public Task {
private:
    int _sec;

public:
    SleepTask(int priority) :
        Task(priority),
        _sec(random() % THREAD_SLEEP) {
        cout << "(" << flush;
    }
    ~SleepTask(void) {
        cout << ")" << flush;
    }
    bool run(void) {
        cout << "." << flush;
        sleep(_sec);
        return true;
    }
};

/*
 * A helper function for the unit tests.
 */
static void threadPoolWithTasks(int tp_size, int total_task)
{
    cout << "TP size = " << tp_size << ", # of tasks = " << total_task << endl;

    // Setup a thread pool.
    ThreadPool tp(tp_size);
    EXPECT_EQ(tp_size, tp.size());

    if (!total_task) {
        return;
    }

    // Add the tasks.
    for (int i = 0; i < total_task; i++) {
        tp.addTask(new SleepTask(i));
    }

    // Wait for the thread pool to end.
    tp.wait();

    EXPECT_EQ(0, tp.size());
}

/*
 * Unit test cases.
 */

TP_TEST( 0,  0);
TP_TEST( 0, 10);
TP_TEST( 1,  1);
TP_TEST( 1, 10);
TP_TEST( 2, 10);
TP_TEST( 3, 11);
TP_TEST(10,  0);
TP_TEST(10,  1);
TP_TEST(10, 10);
TP_TEST(10, 20);
TP_TEST(16, 500);
