#ifndef ROGUE_PARSER_H
#define ROGUE_PARSER_H

#include <ymir/Config/AnyDict.hpp>
#include <filesystem>

namespace rogue {

ymir::Config::AnyDict loadConfigurationFile(const std::filesystem::path &File);

}

#endif // #ifndef ROGUE_PARSER_H