#include "types.h"
#include "port.h"
uint8_t get_input_keycode()
{
    uint8_t keycode = 0;
    while((keycode = inb(0x60)) != 0){
        if(keycode > 0)
            return keycode;
    }
    return keycode;
}