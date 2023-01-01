#include "threadfunctor_example.h"
#include <chrono>

#include <iostream>

using std::cout;
using std::endl;

namespace thorup {

threadfunctor::threadfunctor(uint32_t sleep_time_us, std::mutex *p_mutex,
                             std::condition_variable *p_cv, bool *p_ready)
    : m_sleep_time_us(sleep_time_us), mp_mutex(p_mutex), mp_cv(p_cv), mp_ready(p_ready) {}

ThreadStatus threadfunctor::operator()() {
  cout << "Waiting on conditional Variable" << endl;

  std::unique_lock lk(*mp_mutex);
  bool *p_ready = mp_ready;
  mp_cv->wait(lk, [p_ready]() { return *p_ready; });

  cout << "In Thread sleeping for " << m_sleep_time_us << " (us)" << endl;
  lk.unlock();

  std::this_thread::sleep_for(std::chrono::microseconds(m_sleep_time_us));

  cout << "Exiting Thread " << endl;

  return thorup::ThreadStatus::SUCCESS;
}

} // namespace thorup
