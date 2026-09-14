
#pragma once

#include <functional>
#include <vector>

template<typename... Args>
class Event
{
public:

     using Callback = std::function<void(Args...)>;

     void Subscribe(Callback callback)
     {
          m_Callbacks.push_back(std::move(callback));
     }

     void Invoke(Args... args)
     {
          for (auto& callback : m_Callbacks)
          {
               callback(args...);
          }
     }

private:

     std::vector<Callback> m_Callbacks;
};