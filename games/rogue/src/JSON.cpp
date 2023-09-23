#include <fstream>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <rogue/JSON.h>
#include <string>

namespace {
std::pair<std::string, rapidjson::Document>
loadJSON(const std::filesystem::path &JsonFile) {
  rapidjson::Document Doc;
  std::ifstream FileIn(JsonFile);
  if (!FileIn.is_open()) {
    throw std::runtime_error("Failed to open JSON file: " + JsonFile.string());
  }
  std::string JsonStr((std::istreambuf_iterator<char>(FileIn)),
                      std::istreambuf_iterator<char>());
  Doc.Parse(JsonStr.c_str());
  if (Doc.HasParseError()) {
    throw std::runtime_error("Failed to parse JSON file: " + JsonFile.string());
  }
  return {JsonStr, std::move(Doc)};
}

std::tuple<std::string, rapidjson::Document, rapidjson::SchemaDocument>
loadSchema(const std::filesystem::path &SchemaFile) {
  auto [SchemaJsonStr, SchemaDoc] = loadJSON(SchemaFile);
  rapidjson::SchemaDocument Schema(SchemaDoc);
  return {SchemaJsonStr, std::move(SchemaDoc), std::move(Schema)};
}

void assertValidates(const rapidjson::Document &Doc,
                     const std::filesystem::path &DocPath,
                     const rapidjson::SchemaDocument &Schema,
                     const std::filesystem::path &SchemaPath) {
  rapidjson::SchemaValidator Validator(Schema);
  if (!Doc.Accept(Validator)) {
    // Input JSON is invalid according to the schema
    // Output diagnostic information
    std::stringstream SS;
    rapidjson::StringBuffer SB;
    SS << "Invalid schema: " << DocPath << "\n"
       << "Invalid document: " << SchemaPath << "\n"
       << "Invalid keyword: " << Validator.GetInvalidSchemaKeyword() << "\n";
    Validator.GetInvalidSchemaPointer().StringifyUriFragment(SB);
    SS << "Invalid schema pointer: " << SB.GetString() << "\n";
    SB.Clear();
    Validator.GetInvalidDocumentPointer().StringifyUriFragment(SB);
    SS << "Invalid document pointer: " << SB.GetString() << "\n";
    throw std::runtime_error(SS.str());
  }
}
} // namespace

namespace rogue {

std::pair<std::string, JSONDocument>
loadJSON(const std::filesystem::path &JsonPath,
         const std::filesystem::path *SchemaPath) {
  auto [DocStr, Doc] = ::loadJSON(JsonPath);
  if (SchemaPath) {
    auto [SchemaJsonStr, SchemaDoc, Schema] = ::loadSchema(*SchemaPath);
    assertValidates(Doc, JsonPath, Schema, *SchemaPath);
  }
  return {DocStr, std::move(Doc)};
}
} // namespace rogue