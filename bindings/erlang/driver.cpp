#include "driver.hpp"

int main () {
    erl_init(NULL, 0);
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
    ethelo::engine_processor processor;
    return processor.main();
}
