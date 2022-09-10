#include "vga.h"
extern "C" void main() {
    print_string("Hello from kernel!\n\r");
    for (;;);
}