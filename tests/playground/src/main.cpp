#include "playground_types.h"
#include <iostream>
#include <thread>
#include <vector>
#include <wired.h>

int main() {

    wired::message<some_types> msg;

    std::vector<int> v = {1, 2, 3, 4, 5};
    msg << v;

    msg << int(42);

    msg << a_type(42);

    return 0;
}