#include "game_data_source.hpp"


std::unordered_map<std::string, UniformType> debug_watchlist;

const void DataSource::debug_watch(std::string name, UniformType val)
{
    debug_watchlist[name] = val;
}

const std::unordered_map<std::string, UniformType> &DataSource::debug_get_watch_list()
{
    return debug_watchlist;
}
