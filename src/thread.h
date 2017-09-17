/*

 This library is copyrighted(C) 2016 by WeeSan Lee <weesan@weesan.com>, 
 with all rights reserved.  

 This program is free software;  please feel free to redistribute it 
 and/or modify it under the terms of the GNU General Public License 
 (version 2) as published by the Free Software Foundation.  It is 
 distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A 
 PARTICULAR PURPOSE.  See the GNU General Public License for more details.

 */

#ifndef THREAD_H
#define THREAD_H

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <list>
#include <queue>

using namespace std;

/*
 * The basic unit of a thread.
 *
 * To create a thread, a new class must be derived from this Thread
 * class.  The derived class must also override the run() method which
 * does the actual work.  For example:
 *
 * class Foo : public Thread {
 * private:
 *    void run(void) {
 *        cout << "hello world!" << endl;
 *    }
 * };
 */
class Thread {
    // Need to declare the callback as a friend first.
    friend void *__threadCallBackFunc(void *pvArgv);

    // Then define the callback as a static to avoid compilation error.
    static void *__threadCallBackFunc(void *pvArgv) {
        Thread *pcThread = (Thread *)pvArgv;

        // Should the object returns OK, proc() will be executed
        if (pcThread->ok()) {
            pcThread->run();
        }

        // If the thread is not joinable, delete the object before
        // returning/ending the thread
        if (!pcThread->joinable()) {
            delete pcThread;
        }

        return NULL;
    }
    
public:
    /*
     * There are two types of threads:
     *
     *   1) Joinable
     *   2) Detached
     *
     * For joinable (managed) threads, the main thread needs to "join"
     * the threads when they finish to clean up the resources.  This
     * is also a good way for the main thread to "wait" for the
     * threads to finish; otherwise, the whole program would be
     * terminated immediately.
     *
     * For detached (unmanaged) threads, the main thread does not need
     * to join the threads when they finish.  Each detached thread is
     * able to deallocate itself.  This is a good way for the main
     * thread to do other tasks such as processing GUI events.  This
     * is the default thread type for class Thread.
     */
    enum Type {
	Joinable = PTHREAD_CREATE_JOINABLE,
	Detached = PTHREAD_CREATE_DETACHED,
    };
    
private:
    Type _type;
    pthread_t _threadID;
    
protected:
    // Must be overridden by the derived class.
    virtual void run(void) = 0;
    // Always assume to be ok here.
    virtual bool ok(void) const {
	return true;
    }

public:
    Thread(Type type = Detached) : _type(type), _threadID(0) {
        // Set the thread attribute: joinable or detached.
        pthread_attr_t tThreadAttr;
        pthread_attr_init(&tThreadAttr);
        pthread_attr_setdetachstate(&tThreadAttr, _type);

        // Create the thread.
        int rc = pthread_create(&_threadID, 
                                &tThreadAttr, 
                                __threadCallBackFunc, 
                                this);

        // According to man page, the following function call 
        // does nothing in LinuxThreads implementation
        pthread_attr_destroy(&tThreadAttr);
    }
    virtual ~Thread(void) {
	if (_type == Joinable) {
	    pthread_detach(tid());
	    pthread_cancel(tid());
	}
    }
    pthread_t tid(void) const {
	return _threadID;
    }
    bool joinable(void) const {
        return _type == Joinable;
    }
    static pthread_t self(void) {
        return pthread_self();
    }
};

/*
 * Threads class is used to keep track of a collection of joinable
 * threads.  Each joinable thread is added to the threads.  When a
 * wait() method is called to wait for the joinable threads to finish.
 * For example:
 *
 * class Bar : public Thread {
 * public:
 *     Bar(void) : Thread(Joinable) {
 *     }
 * };
 * Threads threads;
 * threads.add(new Bar);
 * threads.add(new Bar);
 * threads.wait();
 */
class Threads : public list<Thread *> {
private:
    void join(void) {
	// If the thread is joinable, join it; otherwise, ignore.
	for (iterator itr = begin(); itr != end(); ++itr) {
	    if (!(*itr)->joinable()) {
		continue;
	    }
	    pthread_join((*itr)->tid(), NULL);
	    delete *itr;
	}
    }

public:
    // We only keep track of joinable threads so that we can join them
    // before terminated.
    void add(Thread *th) {
        if (th->joinable()) {
            list<Thread *>::push_back(th);
        }
    }
    // Wait for threads to finish.
    void wait(int sec = 0) {
        if (sec) {
            sleep(sec);
        }

	// Join the joinable threads.
	join();

        // Remove the list.
        clear();
    }
};

/*
 * A mutex is a mechanism to protect a shared resource.  Before
 * acessing a shared resource, a thread needs to lock the mutex and
 * unlock the mutex when the thread finishes with the shared resource.
 */
class Mutex {
private:
    pthread_mutex_t _mutex;

public:
    Mutex(void) {
	(void)pthread_mutex_init(&_mutex, NULL);
    }
    ~Mutex(void) {
	(void)pthread_mutex_destroy(&_mutex);
    }
    void lock(void) {
	(void)pthread_mutex_lock(&_mutex);
    }
    void unlock(void) {
	(void)pthread_mutex_unlock(&_mutex);
    }
    bool trylock(void) {
	if (pthread_mutex_trylock(&_mutex) == EBUSY) 
	    return false;
	else
	    return true;
    }
    pthread_mutex_t *operator()(void) {
	return &_mutex;
    }
};

/*
 * Thread pool implementation.
 */

/*
 * A condition variable blocks until it is signaled.  It works
 * together with a mutex.  The mutex needs to be locked first before
 * calling the wait() method, in which, it will unlock the mutex
 * atomically before waiting for the signal.  Before the wait() method
 * returns, it would lock the mutex again.
 *
 * A signal will be sent to an arbitrary thread which has been blocked
 * before.  A broadcast signal will be sent to all threads which have
 * been blocked.
 *
 * A condition variable will be used to implement a thread pool, in
 * which, a pre-defined number of threads are created beforehand.
 * Eash new task will be added to the thread pool and served by the
 * thread in the thread pool.  When a task is finished, the task
 * object will be freed, and the thread will be put back to the thread
 * pool waiting to serve the next task.
 */
class Cond {
private:
    pthread_cond_t _cond;

public:
    Cond(void) {
	(void)pthread_cond_init(&_cond, NULL);
    }
    ~Cond(void) {
	(void)pthread_cond_destroy(&_cond);
    }
    void wait(Mutex &mutex) {
	int rc = pthread_cond_wait(&_cond, mutex());
    }
    void signal(void) {
	int rc = pthread_cond_signal(&_cond);
    }
    void broadcast(void) {
	(void)pthread_cond_broadcast(&_cond);
    }
};

/*
 * Task class is a basic work unit to be served by the threads in the
 * pool.  Highest prority task will be served first.
 */
class Task {
    // Make ThreadPool a friend so that it can access Task::run() method.
    friend class ThreadPool;

private:
    int _priority;

protected:
    // Must be overridden by the derived class.
    virtual void run(void) = 0;

public:
    Task(int priority = 0) : _priority(priority) {
        // Do nothing.
    }
    virtual ~Task(void) {
        // Do nothing.
    }
    // Priority of the task.
    int priority(void) const {
	return _priority;
    }
    // Instruct priority_queue to sort the tasks in descending order.
    bool operator<(const Task &t) const {
	return priority() > t.priority();
    }
};

/*
 * ThreadPool class implements thread pool.  A fixed number of threads
 * are pre-created in the pool.  A number of tasks will be added to
 * the queue for service.  Each thread from the pool will fetch a task
 * from the queue and run it.  When the task is finished, the task
 * object will be destroyed.  The thread, on the other hand, will be
 * returned to the pool and ready to serve the next task.
 */
class ThreadPool : public Threads {
private:
    priority_queue<Task *> _tasks;
    Mutex _mutex;
    Cond _cond;
    bool _done;

private:
    // Worker threads will be pre-created in the thread pool.
    class WorkerThread : public Thread {
    private:
        ThreadPool *_tp;

    private:
        // Override the run() method to fetch tasks from the queue.
        void run(void) {
            if (_tp) {
                _tp->runTask();
            }
        }

    public:
        // WorkerTheads are joinable threads.
        WorkerThread(ThreadPool *tp) : Thread(Joinable), _tp(tp) {
        }
    };

private:
    // A callback method for the worker thread to call.
    void runTask(void) {
        // Each worker thread keeps working until it's signaled otherwise.
        while (true) {
            // Need to lock the mutex first before calling the cond wait.
            _mutex.lock();

            // Wait for the tasks to be served.
            while (!_done && _tasks.empty()) {
                _cond.wait(_mutex);
            }

            // If it's done and no more tasks, exit the loop.
            if (_done && _tasks.empty()) {
                _mutex.unlock();
                break;
            }

            // Retrieve the top priority task if any.
            Task *task = _tasks.top();
            _tasks.pop();

            // Unlock the mutex before running the task.
            _mutex.unlock();

            // Execute the task if any.
            if (task) {
                task->run();
                // After a task is finished, destroy it.
                delete task;
            }
        }
    }

public:
    ThreadPool(int n) : _done(false) {
        // Create a pre-defined number of worker threads.
        for (int i = 0; i < n; i++) {
            add(new WorkerThread(this));
        }    
    }
    ~ThreadPool(void) {
        // Signal all threads to finish.
        done();
        // Destroy all the remaining tasks.
        _mutex.lock();
        while (!_tasks.empty()) {
            Task *task = _tasks.top();
            _tasks.pop();
            delete task;
        }
        _mutex.unlock();
    }
    // Add a new task to be served.
    void addTask(Task *task) {
        // When a done signal has been given, we are not accepting any more jobs.
        // Rather, we serve until the last task is finished, and then exit.
        // aka. graceful shutdown.
        if (_done) {
            return;
        }
	_mutex.lock();
	_tasks.push(task);
	_mutex.unlock();
	_cond.signal();
    }
    // Signal all the worker threads to exit.
    void done(void) {
	_mutex.lock();
        _done = true;
	_mutex.unlock();
        _cond.broadcast();
    }
};

#endif // THREAD_H
