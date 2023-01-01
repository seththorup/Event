#include <iostream>

#include "threadmanager.h"

using namespace std::chrono;
using std::function;
using std::thread;

namespace thorup {


// TODO if no param is passed in, assume waiting on all threads
// should loop through the map and wait on everything.
ThreadStatus threadmanager::wait_on_thread(std::string name) {
  ThreadStatus status{ThreadStatus::NOT_DEFINED};
  if (name == "last") {
    name = m_last_thread;
  }
  if (auto map_itr = m_thread_handle_map.find(name);
      map_itr != m_thread_handle_map.end()) {
    status = map_itr->second.get();
  }
  return status;
}

ThreadStatus threadmanager::get_thread_status(std::string name) {
  ThreadStatus status{ThreadStatus::NOT_DEFINED};

  if (auto map_itr = m_thread_handle_map.find(name);
      map_itr != m_thread_handle_map.end()) {

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

  if (auto map_itr = m_thread_handle_map.find(name);
      map_itr != m_thread_handle_map.end()) {

    status = map_itr->second.get();
    m_thread_handle_map.erase(name);
  }
  return status;
}

std::vector<std::string> threadmanager::get_active_thread_names() {
  std::vector<std::string> thread_names;
  for (auto map_itr = m_thread_handle_map.begin();
       map_itr != m_thread_handle_map.end(); map_itr++) {

    if (get_thread_status(map_itr->second) == ThreadStatus::ACTIVE) {
      thread_names.push_back(map_itr->first);
    }
  }
  return thread_names;
}

uint32_t threadmanager::num_active_threads() {
  uint32_t num_threads{0};
  for (auto map_itr = m_thread_handle_map.begin();
       map_itr != m_thread_handle_map.end(); map_itr++) {
    if (get_thread_status(map_itr->second) == ThreadStatus::ACTIVE) {
      num_threads++;
    }
  }
  return num_threads;
}

void threadmanager::prune_thread_handle_map() {
  for (auto itr = m_thread_handle_map.begin(); itr != m_thread_handle_map.end();
       itr++) {
    if (get_thread_status(itr->first) != ThreadStatus::ACTIVE) {
      m_thread_handle_map.erase(itr->first);
    }
  }
}

} // namespace thorup