#ifndef ROGUE_PARSER_H
#define ROGUE_PARSER_H

#include <filesystem>
#include <ymir/Config/AnyDict.hpp>

namespace rogue {

ymir::Config::AnyDict loadConfigurationFile(const std::filesystem::path &File);

}

#endif // #ifndef ROGUE_PARSER_H