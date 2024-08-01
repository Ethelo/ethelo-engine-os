#pragma once

namespace erl
{
    int read_packet(std::vector<char>& buffer);
    int write_packet(const std::vector<char>& buffer);
}
