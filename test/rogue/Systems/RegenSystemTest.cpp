
TEST(StatsSystemTest, StatsSystemRegenEffects) {
  const rogue::StatPoints StatPoints = {1, 2, 3, 4};

  entt::registry Reg;
  rogue::StatsSystem StatsSystem(Reg);

  auto Entity = Reg.create();
  Reg.emplace<rogue::StatsComp>(Entity, /*Base=*/StatPoints,
                                /*Bonus=*/StatPoints);

  auto &Health = Reg.emplace<rogue::HealthComp>(Entity);
  Health.MaxValue = 100;
  Health.Value = 50;

  auto &Mana = Reg.emplace<rogue::ManaComp>(Entity);
  Mana.MaxValue = 100;
  Mana.Value = 50;

  auto &Agility = Reg.emplace<rogue::AgilityComp>(Entity);
  Agility.Agility = 50;

  StatsSystem.update();
  EXPECT_EQ(Health.Value, 50);
  EXPECT_EQ(Mana.Value, 50);
  EXPECT_EQ(Agility.Agility, 50);

  Reg.emplace<rogue::StatsBuffComp>(Entity, /*Bonus=*/StatPoints);
  StatsSystem.update();
}