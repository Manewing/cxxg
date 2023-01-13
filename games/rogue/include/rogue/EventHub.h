#ifndef ROGUE_EVENT_HUB_H
#define ROGUE_EVENT_HUB_H

#include <functional>
#include <map>
#include <typeindex>

namespace rogue {

struct Event {};

class EventHub {
public:
  using HandlerType = std::function<void(const Event &)>;
  using HandlerMap = std::map<void *, HandlerType>;

public:
  template <class SubscriberType, typename EventType>
  void subscribe(SubscriberType &Subscriber,
                 void (SubscriberType::*CallbackFunc)(const EventType &)) {
    Subscribers[typeid(EventType)][&Subscriber] =
        [&Subscriber, CallbackFunc](const Event &E) {
          (Subscriber.*CallbackFunc)(static_cast<const EventType &>(E));
        };
  }

  template <class SubscriberType> void unsubscribe(SubscriberType &Subscriber) {
    for (auto &[EvTypeId, Handlers] : Subscribers) {
      Handlers.erase(&Subscriber);
    }
  }

  template <typename EventType> void publish(const EventType &E) {
    auto It = Subscribers.find(typeid(EventType));
    if (It == Subscribers.end()) {
      return;
    }
    for (auto const &[Inst, Handler] : It->second) {
      Handler(E);
    }
  }

private:
  std::map<std::type_index, HandlerMap> Subscribers;
};

class EventHubConnector {
public:
  virtual ~EventHubConnector() = default;

  virtual void setEventHub(EventHub *Hub) {
    if (this->Hub) {
      this->Hub->unsubscribe(*this);
    }
    this->Hub = Hub;
  }

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

}

#endif // #ifndef ROGUE_EVENT_HUB_H