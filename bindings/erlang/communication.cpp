#include "driver.hpp"

namespace erl
{
    static int read_exact(char *buf, int len)
    {
        int i, got=0;

        do {
            if ((i = read(3, buf+got, len-got)) <= 0)
                return(i);
            got += i;
        } while (got<len);

        return(len);
    }

    static int write_exact(char *buf, int len)
    {
        int i, wrote = 0;

        do {
            if ((i = write(4, buf+wrote, len-wrote)) <= 0)
                return (i);
            wrote += i;
        } while (wrote<len);

        return (len);
    }

    int read_packet(std::vector<char>& buffer)
    {
        // Read the header
        buffer.resize(4);
        if (read_exact(buffer.data(), 4) != 4)
            return -1;

        // Parse the header
        db::parameters header(buffer.data(), buffer.size());
        uint32_t size; header >> size;

        // Read the rest of the packet
        buffer.resize(size);
        return read_exact(buffer.data(), buffer.size());
    }

    int write_packet(const std::vector<char>& buffer)
    {
        db::writer stream; stream << buffer;
        return write_exact(stream.data(), stream.size());
    }
}
