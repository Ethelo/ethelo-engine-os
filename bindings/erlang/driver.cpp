#include "driver.hpp"
#include <ei.h>

int main () {
    ei_init();
    freopen("/dev/null", "w", stdout);
    freopen("/dev/null", "w", stderr);
    ethelo::engine_processor processor;
    return processor.main();
}
