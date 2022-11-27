#include <iostream>

#include "threadmanager.h"

using namespace std::chrono;
using std::function;
using std::thread;

namespace lb {

void threadmanager::start_thread(std::string name, uint64_t time_us,
                         function<ThreadStatus(std::string, uint64_t)> func) {
  if (auto map_itr = thread_handle_map.find(name);
      map_itr == thread_handle_map.end()) {
    std::future<ThreadStatus> f =
        std::async(std::launch::async, func, name, time_us);
    thread_handle_map.insert({name, std::move(f)});
  } else {
    std::cout << "Thread " << name << " already started" << std::endl;
  }
}

ThreadStatus threadmanager::wait_on_thread(std::string name) {
  ThreadStatus status{ThreadStatus::NOT_DEFINED};
  if (auto map_itr = thread_handle_map.find(name);
      map_itr != thread_handle_map.end()) {
    status = map_itr->second.get();
  }
  return status;
}

ThreadStatus threadmanager::get_thread_status(std::string name) {
  ThreadStatus status{ThreadStatus::NOT_DEFINED};

  if (auto map_itr = thread_handle_map.find(name);
      map_itr != thread_handle_map.end()) {

    std::future_status fstatus =
        map_itr->second.wait_for(std::chrono::nanoseconds(1));

    status = future_status_to_thead_status(fstatus);
  }
  return status;
}

ThreadStatus threadmanager::get_thread_status(std::future<ThreadStatus> &f) {
  ThreadStatus status{ThreadStatus::NOT_DEFINED};
  {
    std::future_status fstatus = f.wait_for(std::chrono::nanoseconds(1));

    status = future_status_to_thead_status(fstatus);
  }
  return status;
}

ThreadStatus threadmanager::get_thread_result(std::string name) {
  ThreadStatus status{ThreadStatus::NOT_DEFINED};

  if (auto map_itr = thread_handle_map.find(name);
      map_itr != thread_handle_map.end()) {

    status = map_itr->second.get();
    thread_handle_map.erase(name);
  }
  return status;
}

std::vector<std::string> threadmanager::get_active_thread_names() {
  std::vector<std::string> thread_names;
  for (auto map_itr = thread_handle_map.begin();
       map_itr != thread_handle_map.end(); map_itr++) {

    if (get_thread_status(map_itr->second) == ThreadStatus::ACTIVE) {
      thread_names.push_back(map_itr->first);
    }
  }
  return thread_names;
}

uint32_t threadmanager::num_active_threads() {
  uint32_t num_threads{0};
  for (auto map_itr = thread_handle_map.begin();
       map_itr != thread_handle_map.end(); map_itr++) {
    if (get_thread_status(map_itr->second) == ThreadStatus::ACTIVE) {
      num_threads++;
    }
  }
  return num_threads;
}

void threadmanager::prune_thread_handle_map() {
  for (auto itr = thread_handle_map.begin(); itr != thread_handle_map.end();
       itr++) {
    if (get_thread_status(itr->first) != ThreadStatus::ACTIVE) {
      thread_handle_map.erase(itr->first);
    }
  }
}

} // namespace lb