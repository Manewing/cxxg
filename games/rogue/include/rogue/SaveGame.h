#ifndef ROGUE_SAVE_GAME_H
#define ROGUE_SAVE_GAME_H

#include <filesystem>
#include <string>

namespace rogue {

struct SaveGameInfo {
public:
  static constexpr const char *JsonExt = ".json";
  static constexpr const char *BinExt = ".bin";

public:
  /// Returns the save game info for the given path
  static SaveGameInfo fromPath(const std::filesystem::path &Path, bool JSON);

  /// Creates a path to the save game directory for the given save game name
  /// \param SaveGameName Name of the save game
  /// \param JSON True if the save game is a JSON file
  static std::filesystem::path getSaveGamePath(const std::string &SaveGameName,
                                               bool JSON);

  /// Returns the save game info for the given slot
  /// \param SlotIdx Slot index
  /// \param JSON True if the save game is a JSON file
  static SaveGameInfo fromSlot(unsigned SlotIdx, bool JSON);

  /// Path to the save game
  std::filesystem::path Path;

  /// True if the save game is a JSON file
  bool JSON = false;

  /// Returns true if the save game exists
  bool exists() const;

  /// Returns the date of the save game as a string, e.g. "2024-01-29 22:40"
  /// \throws std::runtime_error if the date cannot be determined
  std::string getDateStr() const;
};

} // namespace rogue

#endif // #ifndef ROGUE_SAVE_GAME_H