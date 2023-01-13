#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <rogue/EventHub.h>

namespace {

class DummyEventA : public rogue::Event {
public:
  explicit DummyEventA(int Value) : Value(Value) {}
  int Value = 42;
};

inline bool operator==(const DummyEventA &Lhs,
                       const DummyEventA &Rhs) noexcept {
  return Lhs.Value == Rhs.Value;
}

class DummyEventB : public rogue::Event {
public:
  explicit DummyEventB(std::string Msg) : Msg(Msg) {}
  std::string Msg;
};

inline bool operator==(const DummyEventB &Lhs,
                       const DummyEventB &Rhs) noexcept {
  return Lhs.Msg == Rhs.Msg;
}

class DummyEventC : public rogue::Event {
public:
  explicit DummyEventC(std::string Msg) : Msg(Msg) {}
  std::string Msg;
};

class EventListener {
public:
  virtual void onDummyEventA(const DummyEventA &) = 0;
  virtual void onDummyEventB(const DummyEventB &) = 0;
};

class EventListenerMock : public EventListener {
public:
  MOCK_METHOD(void, onDummyEventA, (const DummyEventA &A), (final));
  MOCK_METHOD(void, onDummyEventB, (const DummyEventB &B), (final));
};

class DummyEntity : public rogue::EventHubConnector {
public:
  void setEventHub(rogue::EventHub *Hub) {
    EventHubConnector::setEventHub(Hub);
    subscribe(*this, &DummyEntity::onDummyEventA);
  }

  void doSth(std::string Msg) { publish(DummyEventB(Msg)); }

  void onDummyEventA(const DummyEventA &A) {
    doSth(std::to_string(A.Value * A.Value));
  }
};

TEST(EventHub, SubscribePublish) {
  rogue::EventHub EH;
  EventListenerMock Listener;
  EH.subscribe(Listener, &EventListenerMock::onDummyEventA);
  EH.subscribe(Listener, &EventListenerMock::onDummyEventB);

  EXPECT_CALL(Listener, onDummyEventA(DummyEventA(43)));
  EXPECT_CALL(Listener, onDummyEventB(DummyEventB("asdf")));

  EH.publish(DummyEventA(43));
  EH.publish(DummyEventB("asdf"));
  EH.publish(DummyEventC("asdf"));
}

TEST(EventHub, Unsubscribe) {
  rogue::EventHub EH;
  EventListenerMock Listener;
  EH.subscribe(Listener, &EventListenerMock::onDummyEventA);
  EH.subscribe(Listener, &EventListenerMock::onDummyEventB);
  EH.unsubscribe(Listener);

  EXPECT_CALL(Listener, onDummyEventA(DummyEventA(43))).Times(0);
  EXPECT_CALL(Listener, onDummyEventB(DummyEventB("asdf"))).Times(0);

  EH.publish(DummyEventA(43));
  EH.publish(DummyEventB("asdf"));
}

TEST(EventHub, EventHubConnectorUnconnected) {
  rogue::EventHub EH;
  DummyEntity DE;
  EventListenerMock Listener;
  EH.subscribe(Listener, &EventListenerMock::onDummyEventB);

  // Expect nothing to happen
  DE.doSth("asdf");
  EXPECT_CALL(Listener, onDummyEventB(testing::_)).Times(0);
}

TEST(EventHub, EventHubConnectorConnected) {
  rogue::EventHub EH;

  DummyEntity DE;
  DE.setEventHub(&EH);

  EventListenerMock Listener;
  EH.subscribe(Listener, &EventListenerMock::onDummyEventB);

  EXPECT_CALL(Listener, onDummyEventB(DummyEventB("asdf"))).Times(1);
  EXPECT_CALL(Listener, onDummyEventB(DummyEventB("144"))).Times(1);

  DE.doSth("asdf");
  DE.publish(DummyEventA(12));
}

TEST(EventHub, EventHubConnectorUnsubscribe) {
  rogue::EventHub EH;

  DummyEntity DE;
  DE.setEventHub(&EH);
  DE.setEventHub(nullptr);

  EventListenerMock Listener;
  EH.subscribe(Listener, &EventListenerMock::onDummyEventB);

  EXPECT_CALL(Listener, onDummyEventB(DummyEventB("asdf"))).Times(0);

  DE.doSth("asdf");
}

} // namespace
