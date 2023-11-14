#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <rogue/Components/Combat.h>
#include <rogue/Components/Items.h>
#include <rogue/Components/Stats.h>
#include <rogue/Event.h>
#include <rogue/EventHub.h>
#include <rogue/Systems/DeathSystem.h>

namespace {

class EventListener {
public:
  virtual void onEntityDiedEvent(const rogue::EntityDiedEvent &) = 0;
};

class EventListenerMock : public EventListener {
public:
  MOCK_METHOD(void, onEntityDiedEvent, (const rogue::EntityDiedEvent &E),
              (final));
};

TEST(DeathSystemTest, CheckEmptyContainersAreRemoved) {
  entt::registry Reg;
  rogue::DeathSystem DS(Reg);

  entt::entity Et = Reg.create();
  Reg.emplace<rogue::InventoryComp>(Et);
  DS.update(rogue::System::UpdateType::NoTick);
  EXPECT_FALSE(Reg.valid(Et));

  Et = Reg.create();
  Reg.emplace<rogue::InventoryComp>(Et);
  auto &HC = Reg.emplace<rogue::HealthComp>(Et);
  ASSERT_FALSE(HC.isDead());
  DS.update(rogue::System::UpdateType::NoTick);
  EXPECT_TRUE(Reg.valid(Et));

  Reg.erase<rogue::HealthComp>(Et);
  DS.update(rogue::System::UpdateType::Tick);
  EXPECT_FALSE(Reg.valid(Et));
}

TEST(DeathSystemTest, ReapDeadEntities) {
  entt::registry Reg;
  rogue::EventHub Hub;
  rogue::DeathSystem DS(Reg);
  DS.setEventHub(&Hub);

  entt::entity Et = Reg.create();
  auto &HC = Reg.emplace<rogue::HealthComp>(Et);
  ASSERT_FALSE(HC.isDead());
  DS.update(rogue::System::UpdateType::NoTick);
  EXPECT_TRUE(Reg.valid(Et));

  EventListenerMock Listener;
  Hub.subscribe(Listener, &EventListenerMock::onEntityDiedEvent);
  EXPECT_CALL(Listener, onEntityDiedEvent(rogue::EntityDiedEvent{{}, Et, &Reg}))
      .Times(1);

  HC.Value = 0;
  DS.update(rogue::System::UpdateType::NoTick);
  EXPECT_FALSE(Reg.valid(Et));
}

TEST(DeathSystemTest, ReapDeadEntitiesUpdateCombatComps) {
  entt::registry Reg;
  rogue::DeathSystem DS(Reg);

  entt::entity Et = Reg.create();
  Reg.emplace<rogue::HealthComp>(Et).Value = 0;
  Reg.emplace<rogue::CombatTargetComp>(Et).Attacker = entt::null;
  DS.update(rogue::System::UpdateType::NoTick);
  EXPECT_FALSE(Reg.valid(Et));

  Et = Reg.create();
  entt::entity AtEt = Reg.create();
  Reg.emplace<rogue::HealthComp>(Et).Value = 0;
  Reg.emplace<rogue::CombatTargetComp>(Et).Attacker = AtEt;
  DS.update(rogue::System::UpdateType::NoTick);
  EXPECT_FALSE(Reg.valid(Et));

  Et = Reg.create();
  Reg.emplace<rogue::HealthComp>(Et).Value = 0;
  Reg.emplace<rogue::CombatTargetComp>(Et).Attacker = AtEt;
  Reg.emplace<rogue::CombatAttackComp>(AtEt).Target = Et;
  DS.update(rogue::System::UpdateType::NoTick);
  EXPECT_FALSE(Reg.valid(Et));
  EXPECT_FALSE(Reg.any_of<rogue::CombatAttackComp>(AtEt));
}

TEST(DeathSystemTest, ReapDeadEntitiesDrops) {
  // TODO
}

} // namespace