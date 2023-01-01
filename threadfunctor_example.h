#pragma once
#include "threadmanager.h"

#include <condition_variable>
#include <mutex>

namespace thorup {

class threadfunctor {
private:
  uint32_t m_sleep_time_us{0};
  std::mutex *mp_mutex;
  std::condition_variable *mp_cv;
  bool *mp_ready;

public:
  threadfunctor(uint32_t sleep_time_us, std::mutex *p_mutex, std::condition_variable *p_cv,
                bool *p_ready);
  ThreadStatus operator()();
};
} // namespace thorup