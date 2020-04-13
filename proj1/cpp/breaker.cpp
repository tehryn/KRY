#include <iostream>
#include "base64.h"
#include <string>
#include <vector>
#include <fstream>
#define KEYSIZE 331

std::vector<std::string> getFileContent( std::string const filename ) {
    std::fstream file( filename );
    std::vector<std::string> v;
    if ( file.is_open() ) {
        std::string line;
        while ( std::getline( file, line ) ) {
            v.push_back( line );
        }
    }
    return v;
}

void decodeLines( std::vector<std::string> & lines ) {
    for( std::string & line : lines ) {
        line = base64_decode( line ).substr(0, KEYSIZE);
    }
}

void encrypt( std::string const & keyFile, std::vector<std::string> messages ) {
    std::ifstream file( keyFile, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> key( size );
    std::string result = "";
    if ( file.read( key.data(), size) ) {
        for( std::string const & message : messages ) {
            for( size_t i = 0; i < key.size() && i < message.size(); i++ ) {
                result += key[i] ^ message[i];
            }
            std::cout << base64_encode( result.c_str(), result.size() ) << std::endl;
            result = "";
        }
    }
}

std::string xorStrings( const std::string & s1, const std::string & s2, size_t pre = 0 ) {
    std::string result = "";
    const char * a = s1.c_str();
    const char * b = s2.c_str();
    size_t i = 0, j = pre;
    while ( i < s1.size() && j < s2.size() ) {
        result += a[i++] ^ b[j++];
    }
    return result;
}

std::vector<std::string> xorMessages( const std::vector<std::string> & messages, const std::string & message ) {
    std::vector<std::string> v;
    for( const std::string & m : messages ) {
        v.push_back( xorStrings( m, message ) );
    }
    return v;
}

bool testReadable( const std::string s) {
    for( const char & a : s ) {
        if ( ( a < 'A' || a > 'Z' ) && ( a < 'a' || a > 'z' ) && a != '.' && a != ',' && a != ' ' ) {
            return false;
        }
    }
    return true;
}

void print_hashtag_result( std::string result, size_t lineSize, size_t pre, size_t message_count ) {
    size_t j = 0;
    std::cout << message_count << ": ";
    for(; j < pre; j++ ) {
        std::cout << '#';
    }
    std::cout << result;
    for( j += result.size(); j < lineSize; j++ ) {
        std::cout << '#';
    }
    std::cout << '\n';
}

void print_index_result( std::string result, size_t pre, size_t xoredMessage, size_t step ) {
    std::cout << "Message[" << xoredMessage << "];step[" << step << "];shift[" << pre << "]: " << result << std::endl;
}

void test() {
    std::string s1 = "Hello world";
    std::string s2 = "How are you";
    std::string k  = "abcdefghijk";
    
    std::string e1 = xorStrings( s1, k );
    std::string e2 = xorStrings( s2, k );
    
    std::string x  = xorStrings( s1, s2 );

    std::string word = "world";
    for( size_t i = 0; i < x.size(); i++ ) {
        std::string result = xorStrings( word, x, i );
        if ( testReadable( result ) ) {
            //print_hashtag_result( result, line.size(), i, message_count  );
            print_index_result( result, i, 0, 0);
        }
    }
}

std::string unlimitedPower( std::string const & word, std::vector<std::string> const & xoredMessages, size_t limit = 300, size_t pre = 0 ) {
    static const std::string alfabet = "abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if ( limit <= word.size() || pre == 0 ) {
        return word;
    }
    std::string test = word + '#';
    for( size_t i = 0; i < alfabet.size(); i++ ) {
        test[ test.size() - 1 ] = alfabet[i];
        std::cerr << "testing: \'" << test << "'.......";
        bool ok = true;
        for( std::string const & line : xoredMessages ) {
            std::string result = xorStrings( test, line, pre );
            if ( !testReadable( result ) ) {
                ok = false;
                break;
            }
        }
        if ( ok ) {
            std::cerr << "Passed" << std::endl;
            return unlimitedPower( test, xoredMessages, limit, pre );
        }
        std::cerr << "Failed" << std::endl;
    }
    
    test = '#' + word;
    for( size_t i = 0; i < alfabet.size(); i++ ) {
        test[0] = alfabet[i];
        std::cerr << "testing: \'" << test << "'.......";
        bool ok = true;
        for( std::string const & line : xoredMessages ) {
            std::string result = xorStrings( test, line, pre - 1 );
            if ( !testReadable( result ) ) {
                ok = false;
                break;
            }
        }
        if ( ok ) {
            std::cerr << "Passed" << std::endl;
            return unlimitedPower( test, xoredMessages, limit, pre - 1 );
        }
        std::cerr << "Failed" << std::endl;
    }
    
    return word;
}

std::string getKey( std::string cypher, std::string plain ) {
    return xorStrings( cypher, plain );
}

int main(int argc, char const *argv[]) {
    if ( argc < 3 ) {
        std::cerr << "Invalid arguments. Run as ./breaker b64messages word [ messageIndex ] [ prefix ]" << std::endl;
        return 1;
    }
    //encrypt( "./key.bin", "../result.txt" );
/*    std::vector<std::string> messages = getFileContent( "../messages.txt" );
    decodeLines( messages );
    std::ofstream wf("../messages.shorted.txt", std::ios::out | std::ios::binary);
    for( std::string & line : messages ) {
        wf << base64_encode( line.c_str(), line.size() ) << '\n';
    }
    wf.close();
    */
    const char * filename = argv[1];
    std::string word( argv[2] );
    int index = argc >= 4 ? std::atoi( argv[3] ) : 0;
    
    std::vector<std::string> lines = getFileContent( filename );
    decodeLines( lines );
    
    std::string current = lines[index];
//    std::string plain   = "mluvit poamerictenejsi zaprisahnuti Plesak oindexovani environmentalista vizovicky podavanejsi shodovani tachyon Mysikova signalizovat opletacky Tikalova zolikovy Drahozalova starcu heovetstejsi zahmyzenejsi pristavaci lemniskata respektovani Nemeckuv Holeckuv nivelizacni wehrmacht dojmologie pojistovateluv federalizacni pazbicka";
//    std::cout << getKey( current, plain );
    //lines.erase( lines.begin() + index );
    std::vector<std::string> xored = xorMessages( lines, current );

    if ( argc > 4 ) {
        std::cout << unlimitedPower( word, xored, KEYSIZE, std::atoi( argv[4] ) ) << std::endl;
        return 0;
    }
    
    size_t min = -1;
    for( std::string & line : xored ){
        min = line.size() < min ? line.size() : min;
    }

    // pro kazdou z xorovanych zprav budu hledat slovo
    size_t k = 0;
    std::string tmp = "";
    for( size_t j = 0; j < min; j++ ) {
        k = 0;
        for( std::string & line : xored ) {
            std::string result = xorStrings( word, line, j );
            if ( testReadable( result ) ) {
                tmp += result + '\n';
            }
            else {
                tmp = "";
                break;
            }
            k++;
        }
        if ( tmp.size() ) {
            std::cout << tmp;
            tmp = "";
        }
    }
    return 0;
}