#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include "threadfunctor_example.h"
#include "threadmanager.h"

using std::cout;
using std::endl;
using std::string;

// Globals
std::mutex g_mutex;
std::condition_variable g_cv;
bool g_ready;

thorup::ThreadStatus dummy_func(uint32_t sleep_time_us) {

  cout << "dummy_func"
       << "Waiting on conditional Variable" << endl;

  std::unique_lock lk(g_mutex);
  g_cv.wait(lk, []() { return g_ready; });

  cout << "In Thread sleeping for " << sleep_time_us << " (us)" << endl;
  lk.unlock();

  std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us));

  cout << "Exiting Thread " << endl;

  return thorup::ThreadStatus::SUCCESS;
}
using dummyfunc2 = std::tuple<string, uint32_t, double>;

thorup::ThreadStatus dummy_func_2(dummyfunc2 dummy) {
  auto [func_name, sleep_time_us, dummy_val] = dummy;

  cout << func_name << " Val " << dummy_val << " Waiting on conditional Variable" << endl;

  std::unique_lock lk(g_mutex);
  g_cv.wait(lk, []() { return g_ready; });

  cout << "In Thread sleeping for " << sleep_time_us << " (us)" << endl;
  lk.unlock();

  std::this_thread::sleep_for(std::chrono::microseconds(sleep_time_us));

  cout << "Exiting Thread " << endl;

  return thorup::ThreadStatus::SUCCESS;
}

int main() {

  thorup::threadmanager lb_event;
  std::vector<std::pair<string, uint32_t>> func_vec = {
      {"func_1", 1000000}, {"func_2", 20000}, {"func_3", 100000}};

  for (auto &[name, usecs] : func_vec) {
    thorup::threadfunctor functor(usecs, &g_mutex, &g_cv, &g_ready);
    lb_event.start_thread(name, functor);
  }

  lb_event.start_thread("dummy_func", static_cast<uint32_t>(30000), dummy_func);
  lb_event.start_thread(
      "dummy_func2",
      std::make_tuple(static_cast<string>("dummy_func2"), static_cast<uint32_t>(600000), 97.2),
      dummy_func_2);

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
        if (status != thorup::ThreadStatus::ACTIVE && status != thorup::ThreadStatus::NOT_DEFINED) {
          auto result = lb_event.get_thread_result(thread_name);
          cout << thread_name << " complete. Result = " << thorup::thread_status_to_string(result)
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