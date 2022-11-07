#include <thread>
#include <vector>
#include <queue>
#include <atomic>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadsafeQueue {
  std::queue<T> queue;
  mutable std::mutex mutex;
 
  // Moved out of public interface to prevent races between this and pop().
  bool empty() const {
    return queue.empty();
  }
 
public:
  ThreadsafeQueue() = default;
  virtual ~ThreadsafeQueue() = default;
  ThreadsafeQueue(const ThreadsafeQueue<T> &) = delete ;
  ThreadsafeQueue& operator=(const ThreadsafeQueue<T> &) = delete ;
 
  ThreadsafeQueue(ThreadsafeQueue<T>&& other) {
    std::lock_guard<std::mutex> lock(mutex);
    queue = std::move(other.queue);
  }

  unsigned long size() const {
    std::lock_guard<std::mutex> lock(mutex);
    return queue.size();
  }
 
  std::optional<T> pop() {
    std::lock_guard<std::mutex> lock(mutex);
    if (queue.empty()) return {};

    T tmp = queue.front();
    queue.pop();
    return tmp;
  }
 
  void push(const T &item) {
    std::lock_guard<std::mutex> lock(mutex);
    queue.push(item);
  }
};

class ThreadPool {

  std::atomic_bool done{false};
  ThreadsafeQueue<std::function<void()>> workQueue;
  std::vector<std::thread> threads;
  // join_threads

  void workerThread() {
    while(true) {
      if(done)
	return;
      
      auto task = workQueue.pop();

      if(!task)
	std::this_thread::yield();
      else
	(*task)();
    }
    
  }

public:
  ThreadPool() {
    const auto threadCount = std::thread::hardware_concurrency();

    for(int i = 0; i < threadCount; ++i) {
      try {
	threads.push_back(std::thread(&ThreadPool::workerThread, this));
      }
      catch (...) {
	std::cerr << "COULD NOT CONSTRUCT THREADPOOL\n";
	done = true;
      }
    }
  }
  ~ThreadPool() { done = true; for(auto & t : threads) t.join();}

  template<typename FunctionType>
  auto submit(FunctionType f) { workQueue.push(std::function<void()>(f));  }
  auto awaitAll() {
    while(workQueue.size())
      continue;
    done = true;
    for(auto & t : threads)
       t.join();
  }
};
