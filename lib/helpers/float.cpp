#include <api/ArduinoAPI.h>

//
// Created by kit on 7/26/21.
//

unsigned long Pow(long V, byte shift)
{
    unsigned long Val = 1;
    while (shift-- != 0)
        Val *= V;

    return Val;
}

char * ftos(char * buf, byte W, byte D, double data)
{
    byte shf = 0;
    if (data < 0)
    {
        data *= -1;
        shf = 1;
        buf[0] = '-';
    }
    long Wdata = data, tmp;
    float dec = data - long(data);

    // get the first whole number and convert it
    buf[0 + shf] = Wdata / Pow(10, W - 1) + '0';
    tmp = Wdata % Pow(10, W - 1);

    //now get the rest of the whole numbers
    for (byte i = 1; i < W; i++)
    {
        long factor = Pow(10, W - 1 - i);
        buf[i + shf] = (tmp / factor) + '0';
        tmp %= factor;
    }

    buf[W + shf] = '.'; // add the decimal point

    // now do the decimal numbers
    for (byte i = 0; i < D; i++)
    {
        dec *= 10;
        buf[W + i + 1 + shf] = (long(dec) % 10) + '0';
    }
    // don't forget the NULL terminator
    buf[W + D + 1 + shf] = NULL;
    return buf;
}
