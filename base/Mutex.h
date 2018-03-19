/******************************************************************************************
 * Read And Write Muduo(RAWMD)
 * wzhe, ahuwang@163.com
 ******************************************************************************************/

#pragma once

#include <bast/noncopyable.h>
#include <base/CurrentThread.h>

#include <assert.h>
#include <pthread.h>

namespace rawmd
{

class MutexLock : rawmd::noncopyable
{
public:
    MutexLock()
     : holder_(0)
     {
        int ret = pthread_mutex_init(&mutex_, NULL);
        assert(ret = 0);
        (void) ret;
     }

     ~MutexLock()
     {
        assert(holder_ == 0);
        int ret = pthread_mutex_destory(&mutex_);
        assert(ret == 0);
        (void) ret;
     }

     bool isLockByThisThread()
     {
        return holder_ == CurrentThread::tid();
     }

     void assertLocked()
     {
        assert(isLockByThisThread());
     }

     // internal usage

     void lock()
     {
        pthread_mutex_lock(&mutex_);
        holder_ = CurrentThread::tid();
     }

     void unlock()
     {
        holder_ = 0;
        pthread_mutex_unlock(&mutex_);
     }

     pthread_mutex_t* getPthreadMutex() /* non-const */
     {
        return &mutex_;
     }
private:
    pthread_mutex_t mutex_;
    pid_t holder_;
};

class MutexLockGuard : rawmd::noncopyable
{
public:
    explicit MutexLockGuard(MutexLock& mutex)
        :mutex_(mutex)
    {
        mutex.lock();
    }
    ~MutexLockGuard()
    {
        mutex.unlock();
    }
private:
    MutexLock& mutex_;
};

// Prevent misuse like:
// MutexLockGuard(mutex_);
// A tempory object doesn't hold the lock for long!
#define MutexLockGuard(x) error "Missing guard object name"

}