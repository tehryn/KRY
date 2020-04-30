#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <gmp.h>
#define debug(str,n) std::cerr << __LINE__ << ": " << str << ": " << mpz_get_str( nullptr, FORMAT, n ) << std::endl
#define print(str) std::cerr << str << std::endl


enum Settings     { GENERATE, DECRYPT, ENCRYPT, BREAK, INVALID };
enum ReturnValues { SUCCESS = 0, INVALID_ARGUMENTS, MPZ_INIT_FAIL, FILE_ACCESS_FAIL };
const int FORMAT = 10;

bool isUnsigned( const std::string & str ) {
    for ( char c : str ) {
        if ( !std::isdigit( c ) ) {
            return false;
        }
    }
    return true;
}

bool isHexaDecimal( const std:: string & str ) {
    if ( str.size() <= 2 || str[0] != '0' || str[1] != 'x' ) {
        return false;
    }
    for( auto it = str.begin() + 2; it != str.end(); it++ ) {
        if ( !std::isxdigit( *it ) ) {
            return false;
        }
    }
    return true;
}

std::string bytes2hex( const std::vector<char> & data ) {
    std::stringstream s;
    s << std::hex << std::setfill ( '0' );

    for( char c : data ) {
        s << std::setw(2) << std::setfill('0') << (0xff & c);
    }

     return s.str();
}

Settings parseArguments( int argc, const char ** argv ) {
    if ( argc < 3 ) {
        return INVALID;
    }
    else if ( argc == 3) {
        if ( std::string( argv[1] ) != "-g" || !isUnsigned( argv[2] ) ) {
            return INVALID;
        }
        return GENERATE;
    }
    else if ( argc == 5 ) {
        std::string arg = argv[1];
        if ( !isHexaDecimal( argv[2] ) || !isHexaDecimal( argv[3] ) ) {
            return INVALID;
        }
        else if ( arg == "-e" ) {
            return ENCRYPT;
        }
        else if ( arg == "-d" ) {
            return DECRYPT;
        }
        else if ( arg == "-b" ) {
            return BREAK;
        }
    }
    
    return INVALID;
}

void printResultVector( ReturnValues ret_value, const std::vector<mpz_t> & result ) {
    if ( ret_value != SUCCESS ) {
        std::cerr << "Task failed." << std::endl;
    } 
    else {
        // for( unsigned int i = 0; i < result.size(); i++ ) {
        //     std::cout << result[i];
        //     if ( i == result.size() - 1 ) {
        //         std::cout << std::endl;
        //     }
        //     else {
        //         std::cout << ' ';
        //     }
        // }
    }
    return;
}


ReturnValues randomNumber( mpz_t & result, size_t bits, bool mask = false ) {
    size_t extra = bits % 8;
    size_t size  = ( ( bits + 8 - extra ) ) >> 3;
    std::vector<char> bytes( size );
    
    //std::cout << size << std::endl;
    
    std::ifstream randomSrc( "/dev/urandom", std::ios::out | std::ios::binary );
    if ( !randomSrc.is_open() || !randomSrc.read( bytes.data(), size ) ) {
        return FILE_ACCESS_FAIL;
    }
    randomSrc.close();
    
    std::string hex = bytes2hex( bytes );
    
    if ( mask ) {
        bytes[0] >>= extra;
        bytes[0] |= (0b1000000 >> extra); 
        bytes[ bytes.size() - 1 ] |= 1;
    }
    
    if ( mpz_set_str( result, hex.c_str(), 16 ) ) {
        return MPZ_INIT_FAIL;
    }
    
    return SUCCESS;
}

/*
function powerMod(b, e, m)
    x := 1
    while e > 0
        if e%2 == 1
            x, e := (x*b)%m, e-1
        else b, e := (b*b)%m, e//2
    return x
 */

void power( mpz_t & result, const mpz_t & base, const mpz_t & exp, const mpz_t & mod ) {
    mpz_t a, e, tmp;
    mpz_inits( a, e, tmp, nullptr );
    mpz_set_ui( result, 1 );
    
    mpz_set( a, base );
    mpz_set( e, exp );
    mpz_mod( a, a, mod );
    
    while( mpz_cmp_ui( e, 0 ) > 0 ) {
        mpz_mod_ui( tmp, e, 2 );
        if ( mpz_cmp_ui( tmp, 1 ) == 0 ) {
            mpz_mul( result, result, a );
            mpz_sub_ui( e, e, 1 );
        }
        else {
            
            mpz_div_ui( e, e, 2 );
            mpz_mul( a, a, a );
            mpz_mod( a, a, mod );
        }
    }
    mpz_clears( a, e, tmp, nullptr   );
}

void gcd( mpz_t & result, const mpz_t & a, const mpz_t & b ) {
    if ( mpz_cmp( a, b ) < 0 ) {
        gcd( result, b, a );
    }
    else {
        mpz_t mod;
        mpz_init( mod );
        mpz_mod( mod, a, b );
        if ( mpz_cmp_ui( mod, 0 ) == 0 ) {
            mpz_set( result, b );
        }
        else {
            gcd( result, b, mod );
        }
        mpz_clear( mod );
    }
}

bool isPrime( const mpz_t & n, size_t primeSize, size_t iterations = 30 ) {
    debug( "Testing", n );
    if ( mpz_cmp_ui( n, 1 ) <= 0 || mpz_cmp_ui( n, 4 ) == 0 ) {
        return false;
    }
    if ( mpz_cmp_ui( n, 3 ) <= 0 ) {
        return true;
    }
    
    mpz_t r, n4, n1, tmp;
    mpz_inits( r, n1, n4, tmp, nullptr );
    mpz_sub_ui( n4, n, 4 );
    mpz_sub_ui( n1, n, 1 );
    
    while ( iterations-- > 0 ) {
        randomNumber( r, primeSize );
        //debug( "Random", n );
        mpz_mod( r, r, n4 );
        mpz_add_ui( r, r, 2 );
        //debug( "Result", n );
        gcd( tmp, n, r );
        if ( mpz_cmp_ui( tmp, 1 ) != 0 ) {
            debug( "GCD", tmp );
            return false;
        }
        
        power( tmp, r, n1, n );
        if ( mpz_cmp_ui( tmp, 1 ) != 0 ) {
            debug( "POWER", tmp );
            return false;
        }
    }
    
    return true;
}

ReturnValues generate_key( size_t b, mpz_t & p, mpz_t & q, mpz_t & n, mpz_t & e, mpz_t & d ) {

    size_t sizep = ( b >> 1 ) + b % 2;
    size_t sizeq = ( b >> 1 );
    do {
        ReturnValues test = randomNumber( p, sizep, true );
        if ( test != SUCCESS ) {
            return test;
        }
        print( "==========================" );
    } while ( !isPrime( p, sizep ) );
    debug( "Prime", p );
    
    do {
        ReturnValues test = randomNumber( q, sizeq, true );
        if ( test != SUCCESS ) {
            return test;
        }
        print( "===========================" );
    } while ( !isPrime( q, sizeq ) );
    debug( "Prime", q );    
    /*
    mpz_mul( n, p, q );
    
    mpz_t p_1, q_1, phi, gcd, mul, mod;
    mpz_inits(p_1, q_1, phi, gcd, mul, mod, NULL);
    
    mpz_sub_ui(p_1, p, 1);
    mpz_sub_ui(q_1, q, 1);
    mpz_lcm(phi, p_1, q_1);
    
    gmp_randstate_t x;
    gmp_randinit_mt( x );
    do {
        do {
            do {
                mpz_urandomm( e, x, phi );
            } while ( mpz_cmp_ui(e, 1) <= 0 || mpz_cmp(phi, e) <= 0 );
            mpz_gcd(gcd, e, phi);
        } while ( mpz_cmp_ui(gcd, 1) != 0 );
        mpz_invert(d, e, phi);
        mpz_mul(mul, e, d);
        mpz_mod(mod, mul, phi);
    } while ( mpz_cmp_ui( mod, 1 ) != 0 );
    mpz_clears(p_1, q_1, phi, gcd, mul, mod, NULL);
    */
    return SUCCESS;
}

ReturnValues encrypt( const mpz_t & e, const mpz_t & n, const std::vector<char> & message, std::vector<char> & result ) {
    return SUCCESS;
}

ReturnValues decrypt( const mpz_t & d, const mpz_t & n, const std::vector<char> & message, std::vector<char> & result ) {
    return SUCCESS;
}

ReturnValues unlimitedPower( const mpz_t & e, const mpz_t & n, const std::vector<char> & message, std::vector<char> & result ) {
    return SUCCESS;
}

int main( int argc, const char ** argv ) {
    Settings     mode      = parseArguments( argc, argv );
    ReturnValues ret_value = SUCCESS;
    
    if ( mode == GENERATE ) {
        mpz_t p, q, n, e, d;
        mpz_inits( p, q, n, e, d, nullptr );
        ret_value = generate_key( std::atoi( argv[2] ), p, q, n, e, d );
        if ( ret_value == SUCCESS ) {
            std::string p_str( mpz_get_str( nullptr, FORMAT, p ) ),
                        q_str( mpz_get_str( nullptr, FORMAT, q ) ),
                        n_str( mpz_get_str( nullptr, FORMAT, n ) ),
                        e_str( mpz_get_str( nullptr, FORMAT, e ) ),
                        d_str( mpz_get_str( nullptr, FORMAT, d ) );
            std::cout << p_str << ' ' << q_str << ' ' << n_str << ' ' << e_str << ' ' << d_str << std::endl;
        }
        mpz_clears( p, q, n, e, d, nullptr );
    }
    else if ( mode == DECRYPT ) {
        mpz_t d, n;
        mpz_inits( d, n, nullptr );
        int flag1 = mpz_set_str( d, argv[2], 16 );
        int flag2 = mpz_set_str( n, argv[3], 16 );
        std::vector<char> result;
        std::vector<char> message;
        if ( flag1 && flag2 ) {
            ret_value = decrypt( d, n, message, result );
        }
        else {
            ret_value = MPZ_INIT_FAIL;
            std::cerr << "Unable to init MPZ numbers." << std::endl;
        }
        mpz_clear(d);
        mpz_clear(n);
    }
    else if ( mode == ENCRYPT ) {
        
    }
    else if ( mode == BREAK ) {
        
    }
    else {
        std::cerr << "Invalid arguments." << std::endl; ret_value = INVALID_ARGUMENTS;
    }
    
    return ret_value;
}
