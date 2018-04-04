#include <base/BoundedBlockingQueue.h>
#include <base/CountDownLatch.h>
#include <base/Thread.h>

#include <functional>
#include <vector>
#include <memory>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <stdio.h>

using namespace std;
using namespace rawmd;
using std::placeholders::_1;

class Test
{
 public:
  Test(int numThreads)
    : queue_(20),
      latch_(numThreads)
  {
    for (int i = 0; i < numThreads; ++i)
    {
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.push_back(static_cast<unique_ptr<Thread>>(new Thread(
            bind(&Test::threadFunc, this), string(name))));
    }
    for_each(threads_.begin(), threads_.end(), bind(&Thread::start, _1));
  }

  void run(int times)
  {
    printf("waiting for count down latch\n");
    latch_.wait();
    printf("all threads started\n");
    for (int i = 0; i < times; ++i)
    {
      char buf[32];
      snprintf(buf, sizeof buf, "hello %d", i);
      queue_.put(buf);
      printf("tid=%d, put data = %s, size = %zd\n", CurrentThread::tid(), buf, queue_.size());
    }
  }

  void joinAll()
  {
    for (size_t i = 0; i < threads_.size(); ++i)
    {
      queue_.put("stop");
    }

    for_each(threads_.begin(), threads_.end(), bind(&Thread::join, _1));
  }

 private:

  void threadFunc()
  {
    printf("tid=%d, %s started\n",
           CurrentThread::tid(),
           CurrentThread::name());

    latch_.countDown();
    bool running = true;
    while (running)
    {
      std::string d(queue_.take());
      printf("tid=%d, get data = %s, size = %zd\n", CurrentThread::tid(), d.c_str(), queue_.size());
      running = (d != "stop");
    }

    printf("tid=%d, %s stopped\n",
           CurrentThread::tid(),
           CurrentThread::name());
  }

  BoundedBlockingQueue<std::string> queue_;
  CountDownLatch latch_;
  vector<unique_ptr<Thread>> threads_;
};

int main()
{
  printf("pid=%d, tid=%d\n", ::getpid(), CurrentThread::tid());
  Test t(5);
  t.run(100);
  t.joinAll();

  printf("number of created threads %d\n", Thread::numCreated());
}
