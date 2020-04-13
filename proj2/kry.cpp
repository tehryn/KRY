#include <iostream>
#include "base64.h"
#include <string>
#include <vector>
#include <fstream>

enum Settings { GENERATE, DECRYPT, ENCRYPT, BREAK, INVALID };

Settings parseArguments( int argc, const char ** argv ) {
    return INVALID;
}

int main( int argc, const char ** argv ) {
    Settings mode = parseArguments( argc, argv );
    if ( mode == INVALID ) {
        std::cerr << "Invalid arguments" << std::endl;
    }
    
    return 0;
}
/*
Generování klíčů (3b) 
vstup: "./kry -g B" 
výstup: "P Q N E D"

Šifrování (0.5b) 
vstup: "./kry -e E N M" 
výstup: "C"

Dešifrování (0.5b) 
vstup: "./kry -d D N C" 
výstup: "M"

Prolomení RSA (3b) 
vstup: "./kry -b E N C" 
výstup: "P Q M"
 */