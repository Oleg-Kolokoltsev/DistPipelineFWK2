
/*
 * Convert IEEE754-1985 floating point format network binary data to the native floating point format.
 * The IEEE754-1985 is a standard for binary data transfer over SCPI protocol, it has
 * 23-bit fraction (mantissa). Sometimes the internal system "float" has 24-bit mantissa.
 * In this case a binary conversion shell be applied.
 */

#ifndef IEEE754_1985_HPP
#define IEEE754_1985_HPP

#include <memory>
#include <bitset>

/*
 * TODO: this code has a bug for 'inf' and 'undef' values
 * TODO: check i system float already has a IEEE754-1985 standard, so the conversion is not
 * necessary (use static compiler directives)
 */

//bin - 4 bytes of binary data
float ieee754_1985_to_float(const char* bin){

    //copy 4 bytes of binary data into an integer type, it will permit binary operations
    int ul = (*static_cast<int*>((void*)bin));

    //reorder bytes from network byte order to host byte order
    ul = ntohl(ul);

    //move all 1 bit left, it will eliminate sign bit and
    //put a 8-bit exponent into highest byte; the new 24-bit mantissa now has correct position
    //and shell not be modified
    /*ul = ul << 1;
    char Exp8 = (ul << 1) >> 24;

    //convert 8-bit exponent into 7-bit exponent; the 8-bit exponent has 127 center and its max is 255
    //the 7-bit exponent is centered about 63 and has max at 127. The excess of exp sizes will be truncated
    char Exp7;
    if(Exp8 < 64){
        Exp7 = 0; //truncate too small exponents
    }else if(Exp8 > 127){
        Exp7 = 127; //truncate too large exponents
    }else{
        Exp7 = Exp8 - 64; //move exp center from 127 to 63
    }

    //the resulting truncated exponent Exp7 will now have it's 7-th bit zero (important!), and
    //accordingly to the 24-bit mantissa standard we replace it with a sign bit of the original highest byte
    Exp7 = Exp7 | ((*bin)&0x80);

    //ul for the moment has old Exp8 in its highest byte and shell be replaced with new 'signed' Exp7
    //exponent of native standard
    ul = (ul & 0x00FFFFFF)|(Exp7 << 24);*/

    //ul contains the resulting float binary value that is to be copied into the system float
    float v;
    memcpy(&v,&ul,4);
    return v;
}

#endif //IEEE754_1985_HPP

//https://www.h-schmidt.net/FloatConverter/IEEE754.html