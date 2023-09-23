#ifndef ROGUE_JSON_H
#define ROGUE_JSON_H

#include <filesystem>
#include <rapidjson/document.h>

namespace rogue {

using JSONDocument = rapidjson::Document;

std::pair<std::string, JSONDocument>
loadJSON(const std::filesystem::path &JsonPath,
         const std::filesystem::path *SchemaPath);

} // namespace rogue

#endif // #ifndef ROGUE_JSON_H