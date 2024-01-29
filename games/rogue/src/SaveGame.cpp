#include <cxxg/Utils.h>
#include <rogue/SaveGame.h>

namespace rogue {

std::filesystem::path
SaveGameInfo::getSaveGamePath(const std::string &SaveGameName, bool JSON) {
  auto Path = std::filesystem::path(cxxg::utils::getHomeDir());
  Path = Path / ".rogue" / "saves";
  std::filesystem::create_directories(Path);
  Path /= SaveGameName;
  if (JSON) {
    Path.replace_extension(JsonExt);
  } else {
    Path.replace_extension(BinExt);
  }
  return Path;
}

SaveGameInfo SaveGameInfo::fromPath(const std::filesystem::path &Path,
                                    bool JSON) {
  SaveGameInfo SGI;
  SGI.Path = std::filesystem::absolute(Path);
  SGI.JSON = JSON;
  return SGI;
}

SaveGameInfo SaveGameInfo::fromSlot(unsigned Slot, bool JSON) {
  SaveGameInfo SGI;
  return fromPath(getSaveGamePath("slot_" + std::to_string(Slot), JSON), JSON);
}

namespace {

template <typename TimePointTy> std::time_t getTime(TimePointTy TimePoint) {
  using namespace std::chrono;
  auto SysClockTimePoint = time_point_cast<system_clock::duration>(
      TimePoint - TimePointTy::clock::now() + system_clock::now());
  return system_clock::to_time_t(SysClockTimePoint);
}

std::string getFileWriteDate(const std::filesystem::path &Path) {
  std::stringstream SS;
  auto LastWrite = getTime(std::filesystem::last_write_time(Path));
  std::tm *Gmt = std::gmtime(&LastWrite);
  SS << std::put_time(Gmt, "%F %R");
  return SS.str();
}

} // namespace

bool SaveGameInfo::exists() const { return std::filesystem::exists(Path); }

std::string SaveGameInfo::getDateStr() const {
  if (!exists()) {
    throw std::runtime_error("SaveGameInfo: Save game does not exist: " +
                             Path.string());
  }
  return getFileWriteDate(Path);
}

} // namespace rogue