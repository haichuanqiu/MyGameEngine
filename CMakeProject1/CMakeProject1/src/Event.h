#pragma once

#include <cstdint>
#include <functional>
#include <utility>
#include <vector>
#include <algorithm>

template<typename... Args>
class Event
{
public:

     using Callback = std::function<void(Args...)>;
     using SubscriptionID = uint64_t;


private:

     struct Subscription
     {
          SubscriptionID id;
          Callback callback;
     };


public:

     // ============================================================
     // Subscribe
     // ============================================================

     SubscriptionID Subscribe(Callback callback)
     {
          const SubscriptionID id =
               m_NextSubscriptionID++;

          m_Callbacks.push_back(
               {
                   id,
                   std::move(callback)
               }
          );

          return id;
     }


     // ============================================================
     // Unsubscribe
     // ============================================================

     void Unsubscribe(SubscriptionID id)
     {
          if (id == 0)
               return;

          std::erase_if(
               m_Callbacks,
               [id](const Subscription& subscription)
               {
                    return subscription.id == id;
               }
          );
     }


     // ============================================================
     // Unsubscribe All
     // ============================================================

     void UnsubscribeAll()
     {
          m_Callbacks.clear();
     }


     // ============================================================
     // Invoke
     // ============================================================

     void Invoke(Args... args)
     {
          for (auto& subscription : m_Callbacks)
          {
               if (subscription.callback)
               {
                    subscription.callback(args...);
               }
          }
     }


     // ============================================================
     // Empty
     // ============================================================

     bool Empty() const
     {
          return m_Callbacks.empty();
     }


     // ============================================================
     // Count
     // ============================================================

     size_t Count() const
     {
          return m_Callbacks.size();
     }


private:

     std::vector<Subscription> m_Callbacks;

     SubscriptionID m_NextSubscriptionID = 1;
};