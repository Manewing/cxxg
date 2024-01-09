#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>
#include <rogue/ItemDatabase.h>
#include <rogue/ItemEffect.h>
#include <string>

void dumpLootTableRewards(const rogue::ItemDatabase &ItemDb,
                          const std::string &LootTableName, unsigned Rolls) {
  const auto &LootTable = ItemDb.getLootTable(LootTableName);

  unsigned Total = 0;
  std::map<int, unsigned> ItemIdCounts;
  std::map<int, unsigned> ItemIdOccurrences;
  for (std::size_t I = 0; I < Rolls; I++) {
    auto LootRewards = LootTable->generateLoot();

    // Count occurrences over all loot rewards, rewards may be duplicates
    std::set<int> SeenIds;
    for (auto &LootReward : LootRewards) {
      ItemIdCounts[LootReward.ItId] += LootReward.Count;
      if (SeenIds.insert(LootReward.ItId).second) {
        ItemIdOccurrences[LootReward.ItId]++;
      }
      Total += LootReward.Count;
    }
  }

  std::vector<std::pair<unsigned, int>> SortedCounts;
  for (auto &Pair : ItemIdCounts) {
    SortedCounts.emplace_back(Pair.second, Pair.first);
  }
  std::sort(SortedCounts.begin(), SortedCounts.end(), std::greater<>());

  std::cout << "{\n'item_rewards': [" << std::endl;
  const char *Pred = "  ";
  for (auto &[Count, ItId] : SortedCounts) {
    auto Occurrences = ItemIdOccurrences.at(ItId);
    auto PercentagePerDrop = static_cast<double>(Occurrences) / Rolls * 100.0;
    auto CountPerDrop = static_cast<double>(Count) / Occurrences;
    auto AverageCountPerDrop = static_cast<double>(Count) / Rolls;
    std::cout << std::left << std::fixed << Pred
              << "{ 'count': " << std::setw(6) << Count
              << ", 'name':" << std::setw(40)
              << ("'" + ItemDb.getItemProto(ItId).Name + "'")
              << ", 'pp_drop': " << std::setw(5) << std::setprecision(2)
              << PercentagePerDrop << " ,'oc':" << std::setw(6) << Occurrences
              << ", 'cp_drop':" << std::setw(4) << std::setprecision(2)
              << CountPerDrop << ", 'ac_drop':" << std::setw(4)
              << std::setprecision(2) << AverageCountPerDrop << std::endl;
    Pred = "},";
  }
  std::cout << "}]," << std::endl;

  std::cout << "'total': " << Total << ", " << std::endl
            << "'rolls': " << Rolls << std::endl
            << "}" << std::endl;
}

int handleLootTable(const rogue::ItemDatabase &ItemDb, int Argc, char *Argv[]) {
  if (Argc != 4 && Argc != 5) {
    std::cerr << "usage: " << Argv[0]
              << " <item_db_config> --loot-table <loot_table_name> *<rolls>"
              << std::endl;
    return 2;
  }

  std::string LootTableName = Argv[3];

  unsigned Rolls = 100;
  if (Argc == 5) {
    Rolls = std::stoi(Argv[4]);
  }

  dumpLootTableRewards(ItemDb, LootTableName, Rolls);

  return 0;
}

void dumpItemCreations(const rogue::ItemDatabase &ItemDb,
                       const std::string &ItemName, unsigned Rolls) {
  const auto ItemId = ItemDb.getItemId(ItemName);

  const std::string LineSep(80, '-');
  for (std::size_t I = 0; I < Rolls; I++) {
    auto Item = ItemDb.createItem(ItemId);

    std::cout << LineSep << std::endl
              << "[" << I << "]: Id: " << Item.getId() << " " << Item.getName()
              << " | " << Item.getQualifierName() << std::endl
              << LineSep << std::endl
              << Item.getDescription() << std::endl
              << LineSep << std::endl
              << "Type: " << Item.getType() << std::endl
              << "Capabilities: " << Item.getCapabilityFlags().flagString()
              << std::endl
              << LineSep << std::endl
              << "Effects:" << std::endl;
    for (auto &EffInfo : Item.getAllEffects()) {
      std::cout << " - Attrs: " << EffInfo.Attributes << std::endl
                << "   Effect: " << EffInfo.Effect->getName() << std::endl;
    }
    std::cout << std::endl;
  }
}

int handleDumpItem(const rogue::ItemDatabase &ItemDb, int Argc, char *Argv[]) {
  if (Argc != 4 && Argc != 5) {
    std::cerr << "usage: <item_db_config> --dump-item <item> *<rolls>"
              << std::endl;
    return 3;
  }

  std::string ItemName = Argv[3];

  unsigned Rolls = 1;
  if (Argc == 5) {
    Rolls = std::stoi(Argv[4]);
  }

  dumpItemCreations(ItemDb, ItemName, Rolls);

  return 0;
}

int handleDumpLootTables(const rogue::ItemDatabase &ItemDb, int Argc,
                         char *[]) {
  if (Argc != 3) {
    std::cerr << "usage: <item_db_config> --dump-tables" << std::endl;
    return 4;
  }

  for (auto &[Name, LootTable] : ItemDb.getLootTables()) {
    std::cout << "LootTable=" << std::left << std::setw(40)
              << ("'" + Name + "'") << " Rolls=" << std::setw(2)
              << LootTable->getRolls() << " Slots=" << std::setw(2)
              << LootTable->getSlots().size() << std::endl;
  }

  return 0;
}

void dumpItems(const rogue::ItemDatabase &ItemDb) {
  for (auto &[Id, Proto] : ItemDb.getItemProtos()) {
    std::cout << Id << ": " << Proto.Name << " [" << Proto.Type << "] "
              << std::endl;
  }
}

void dumpUsage(const char *PrgName) {
  std::cerr
      << "usage: " << PrgName << " <item_db_config> *<--options>" << std::endl
      << std::endl
      << "options:" << std::endl
      << "  --dump-tables                           (Dumps all loot tables)"
      << std::endl
      << "  --loot-table <loot_table_name> *<rolls> (Generates items for loot "
         "table and dumps statistics)"
      << std::endl
      << "  --dump-item <item> *<rolls>             (Creates the specified "
         "items for the given number of rolls and dumps it)"
      << std::endl
      << std::endl
      << std::endl
      << "If no options are specified, names of all items are dumped."
      << std::endl;
}

int wrapped_main(int Argc, char *Argv[]) {
  if (Argc < 2) {
    dumpUsage(Argv[0]);
    return 1;
  }
  std::srand(std::time(nullptr));

  std::filesystem::path ItemDbConfig = Argv[1];
  if (!std::filesystem::exists(ItemDbConfig)) {
    std::cerr << "error: item db config file does not exist: " << ItemDbConfig
              << std::endl;
    return 1;
  }
  auto ItemDb = rogue::ItemDatabase::load(ItemDbConfig);

  std::string Option = Argc >= 3 ? Argv[2] : "";
  if (Option.empty()) {
    dumpItems(ItemDb);
    return 0;
  }

  if (Option == "--help" || Option == "-h") {
    dumpUsage(Argv[0]);
    return 0;
  }

  if (Option == "--loot-table") {
    return handleLootTable(ItemDb, Argc, Argv);
  }

  if (Option == "--dump-item") {
    return handleDumpItem(ItemDb, Argc, Argv);
  }

  if (Option == "--dump-tables") {
    return handleDumpLootTables(ItemDb, Argc, Argv);
  }

  std::cerr << "error: unknown option: " << Option << std::endl;

  return 1;
}

int main(int Argc, char *Argv[]) {
  try {
    return wrapped_main(Argc, Argv);
  } catch (std::exception &E) {
    std::cerr << "error: " << E.what() << std::endl;
    return 1;
  }
}