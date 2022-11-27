#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include "Event.h"

using std::cout;
using std::endl;
using std::string;

// Globals
std::mutex g_mutex;
std::condition_variable g_cv;
bool g_ready;

LB::ThreadStatus dummy_func(string name, uint64_t sleep_time_us) {

  cout << "Waiting on conditional Variable" << endl;

  std::unique_lock lk(g_mutex);
  g_cv.wait(lk, [] { return g_ready; });

  cout << "In Thread " << name << " sleeping for " << sleep_time_us
       << " (us)" << endl;
  lk.unlock();

  std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us));

  cout << "Exiting Thread " << name << endl;

  return LB::ThreadStatus::SUCCESS;
}

int main() {

  LB::Event lb_event;
  std::vector<std::pair<string, uint32_t>> func_vec = {
      {"func_1", 300}, {"func_2", 200}, {"func_3", 100}};

  for (auto &[name, secs] : func_vec) {
    lb_event.start_thread(name, secs, dummy_func);
  }

  auto num_threads = lb_event.num_active_threads();
  auto thread_names = lb_event.get_active_thread_names();
  cout << "Number of active threads: " << num_threads << endl;
  for (auto name : thread_names) {
    cout << name << " ";
  }
  cout << endl;

  uint32_t wait_count{0};
  while (num_threads > 0) {
    wait_count++;
    if (num_threads != lb_event.num_active_threads()) {
      num_threads = lb_event.num_active_threads();

      auto thead_names = lb_event.get_active_thread_names();
      cout << "Active Threads ";
      for (auto name : thead_names) {
        cout << name << " ";
      }
      cout << endl;

      for (auto itr = func_vec.begin(); itr != func_vec.end(); itr++) {
        auto thread_name = itr->first;
        auto status = lb_event.get_thread_status(thread_name);
        if (status != LB::ThreadStatus::ACTIVE &&
            status != LB::ThreadStatus::NOT_DEFINED) {
          auto result = lb_event.get_thread_result(thread_name);
          cout << thread_name
               << " complete. Result = " << LB::thread_status_to_string(result)
               << endl;
        }
      }
    }

    // send data to the worker thread
    {
      std::lock_guard lk(g_mutex);
      g_ready = true;
    }
    g_cv.notify_all();

    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
  return 0;
}