#ifndef ROGUE_EVENT_HUB_H
#define ROGUE_EVENT_HUB_H

#include <functional>
#include <map>
#include <typeindex>
#include <vector>

class Event {};

class EventHub {
public:
  using HandlerType = std::function<void(const Event &)>;
  using HandlerList = std::vector<HandlerType>;

public:
  template <class SubscriberType, typename EventType>
  void subscribe(SubscriberType &Subscriber,
                 void (SubscriberType::*CallbackFunc)(const EventType &)) {
    Handlers[typeid(EventType)].push_back(
        [&Subscriber, CallbackFunc](const Event &E) {
          (Subscriber.*CallbackFunc)(static_cast<const EventType &>(E));
        });
  }

  template <typename EventType> void publish(const EventType &E) {
    auto It = Handlers.find(typeid(EventType));
    if (It == Handlers.end()) {
      return;
    }
    for (auto const &Handler : It->second) {
      Handler(E);
    }
  }

private:
  std::map<std::type_index, HandlerList> Handlers;
};

class EventHubConnector {
public:
  virtual void setEventHub(EventHub *Hub) { this->Hub = Hub; }

  template <class SubscriberType, typename EventType>
  void subscribe(SubscriberType &Subscriber,
                 void (SubscriberType::*CallbackFunc)(const EventType &)) {
    if (!Hub) {
      return;
    }
    Hub->subscribe(Subscriber, CallbackFunc);
  }

  template <typename EventType> void publish(const EventType &E) {
    if (!Hub) {
      return;
    }
    Hub->publish(E);
  }

private:
  EventHub *Hub = nullptr;
};

#endif // #ifndef ROGUE_EVENT_HUB_H