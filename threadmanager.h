#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <map>
#include <string>
#include <vector>

namespace thorup {

enum class ThreadStatus { NOT_DEFINED = 0, ACTIVE, READY, SUCCESS, FAILED };

const std::vector<std::string> thread_status_str = {
    "NOT_DEFINED", "ACTIVE", "READY", "SUCCESS", "FAILED"};

inline std::string thread_status_to_string(ThreadStatus status) {
  return thread_status_str[static_cast<uint32_t>(status)];
}

inline ThreadStatus future_status_to_thead_status(std::future_status fstatus) {
  ThreadStatus tstatus{ThreadStatus::NOT_DEFINED};
  switch (fstatus) {
  case std::future_status::deferred:
    tstatus = ThreadStatus::NOT_DEFINED;
    break;
  case std::future_status::ready:
    tstatus = ThreadStatus::READY;
    break;
  case std::future_status::timeout:
    tstatus = ThreadStatus::ACTIVE;
    break;
  default:
    tstatus = ThreadStatus::NOT_DEFINED;
  }
  return tstatus;
}

class threadmanager {
private:
  std::map<std::string, std::future<ThreadStatus>> thread_handle_map;
  ThreadStatus get_thread_status(std::future<ThreadStatus> &f);

public:
  threadmanager() = default;

  template <typename T>
  void start_thread(std::string name, T val,
                    ThreadStatus (*func)(std::string, T));

  ThreadStatus wait_on_thread(std::string name);
  ThreadStatus get_thread_status(std::string name);
  ThreadStatus get_thread_result(std::string name);
  std::vector<std::string> get_active_thread_names();
  uint32_t num_active_threads();
  void prune_thread_handle_map();
};

template <typename T>
void threadmanager::start_thread(std::string name, T val,
                                 ThreadStatus (*func)(std::string, T)) {
  if (auto map_itr = thread_handle_map.find(name);
      map_itr == thread_handle_map.end()) {
    std::future<ThreadStatus> f =
        std::async(std::launch::async, func, name, val);
    thread_handle_map.insert({name, std::move(f)});
  } else {
    std::cout << "Thread " << name << " already started" << std::endl;
  }
}
} // namespace thorup
