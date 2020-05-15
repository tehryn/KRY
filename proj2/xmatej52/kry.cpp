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
enum ReturnValues { SUCCESS = 0, INVALID_ARGUMENTS, MPZ_INIT_FAIL, FILE_ACCESS_FAIL, INVALID_PARAM_E, INVALID_PARAM_N };
const int FORMAT    = 16;
const char * PREFIX = FORMAT == 16 ? "0x" : "";

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
        if ( !isHexaDecimal( argv[2] ) || !isHexaDecimal( argv[3] ) || !isHexaDecimal( argv[4] ) ) {
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

ReturnValues randomNumber( mpz_t & result, size_t bits, bool mask = false ) {
    size_t extra = bits % 8;
    size_t size  = ( ( bits + 8 - ( extra > 0 ? extra : 8 ) ) ) >> 3;
    std::vector<char> bytes( size );
    
    std::ifstream randomSrc( "/dev/urandom", std::ios::out | std::ios::binary );
    if ( !randomSrc.is_open() || !randomSrc.read( bytes.data(), size ) ) {
        return FILE_ACCESS_FAIL;
    }
    randomSrc.close();
    
    if ( mask ) {
        bytes[0] &= 0b11111111 >> extra;
        bytes[0] |= 0b10000000 >> extra; 
        bytes[ bytes.size() - 1 ] |= 1;
    }
    
    std::string hex = bytes2hex( bytes );
    
    if ( mpz_set_str( result, hex.c_str(), 16 ) ) {
        return MPZ_INIT_FAIL;
    }
    
    return SUCCESS;
}

void invert( mpz_t & result, const mpz_t & num, const mpz_t & modulo ) {
    if ( mpz_cmp_ui( modulo, 0 ) == 0 ) {
        mpz_set_ui( result, 0 );
        return;
    }
    
    mpz_t mod, a, y, q, tmp;
    mpz_inits( mod, a, y, q, tmp, nullptr );
    
    mpz_set( mod, modulo );
    mpz_set( a, num );
    
    mpz_set_ui( result, 1 );
    mpz_set_ui( y, 0 );
    
    while ( mpz_cmp_ui( a, 1 ) > 0 ) {
        mpz_tdiv_q( q, a, mod );
        mpz_set( tmp, mod );
        mpz_mod( mod, a, mod );
        mpz_set( a, tmp );
        mpz_set( tmp, y );
        mpz_mul( y, y, q );
        mpz_sub( y, result, y );
        mpz_set( result, tmp );
    }
    
    if ( mpz_cmp_ui( result, 0 ) < 0 ) {
        mpz_add( result, result, modulo );
    }
    
    mpz_clears( mod, a, y, q, tmp, nullptr );
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

/*
 * Computes 
 */
void powm( mpz_t & result, const mpz_t & num, const mpz_t & exp, const mpz_t & modulo ) {
    mpz_t n;
    mpz_init( n );
    mpz_mod( n, num, modulo );
    if ( mpz_cmp_ui( n, 0 ) == 0 ) {
        mpz_set_ui( result, 0 );
        mpz_clear( n );
        return;
    }
    
    mpz_t odd, e;
    mpz_inits( odd, e, nullptr );
    mpz_set_ui( result, 1 );
    mpz_set( e, exp );
    
    while ( mpz_cmp_ui( e, 0 ) > 0 ) {
        mpz_mod_ui( odd, e, 2 );
        if ( mpz_cmp_ui( odd, 1 ) == 0 ) {
            mpz_mul( result, result, n );
            mpz_mod( result, result, modulo );
        }
        mpz_mul( n, n, n );
        mpz_mod( n, n, modulo );
        mpz_div_ui( e, e, 2 );
    }
    
    mpz_clears( odd, e, n, nullptr );
}

/*
 * Used for testing primes
 */
bool powerTest( const mpz_t & num, const mpz_t & exp, const mpz_t & modulo ) {
    mpz_t a;
    mpz_init( a );
    powm( a, num, exp, modulo );
    int ret = mpz_cmp_ui( a, 1 );
    mpz_clear( a );
    return ret != 0;
}

/*
 * Tests if number is prime
 */
bool isPrime( const mpz_t & n, size_t primeSize, size_t iterations = 30 ) {
    if ( mpz_cmp_ui( n, 1 ) == 0 ) {
        return false;
    }
    
    mpz_t tmp, n1;
    bool ret = true;
    mpz_inits( tmp, n1, nullptr );
    mpz_sub_ui( n1, n, 1 );
    
    while ( iterations-- > 0 ) {
        randomNumber( tmp, primeSize );
        mpz_mod( tmp, tmp, n1 );
        mpz_add_ui( tmp, tmp, 1 );
        
        if ( powerTest( tmp, n1, n ) ) {
            ret = false;
            break;
        }
    }
    
    mpz_clears( tmp, n1, nullptr );
    return ret;
}

/*
 * Computes p and q using Pollard’s Rho algorithm for prime factorization
 */
ReturnValues primeFactorPollard( mpz_t & p, mpz_t & q, const mpz_t & n ) {
    if ( mpz_cmp_ui( n, 1 ) == 0 ) {
        mpz_set_ui( p, 1 );
        mpz_set_ui( q, 1 );
        return SUCCESS;
    }
    
    if ( isPrime( n, mpz_sizeinbase( n, 2 ) ) ) {
        mpz_set( p, n );
        mpz_set_ui( q, 1 );
        return SUCCESS;
    }
    
    mpz_t mod, d, x, x2, y, y2, c, num2, sub;
    bool set = false;
    ReturnValues ret = SUCCESS;
    mpz_inits( mod, d, x, x2, y, y2, c, num2, sub, nullptr );
    mpz_mod_ui( mod, n, 2 );
    mpz_set_ui( num2, 2 );
    
    if ( mpz_cmp_ui( mod, 0 ) == 0 ) {
        mpz_set_ui( p, 2 );
        mpz_div_ui( q, n, 2 );
        set = true;
    }
    
    while ( !set ) {
        ret = randomNumber( x, mpz_sizeinbase( n, 2 ) );
        ret = ret == SUCCESS ? randomNumber( c, mpz_sizeinbase( n, 2 ) ) : ret;
        if ( ret == SUCCESS ) {
            mpz_set( mod, n );
            mpz_sub_ui( mod, mod, 2 );
            mpz_mod( x, x, mod );
            mpz_add_ui( x, x, 2 );
            mpz_set( y, x );
            mpz_add_ui( mod, mod, 1 );
            mpz_mod( c, c, mod );
            mpz_add_ui( c, c, 1 );
            mpz_set_ui( d, 1 );
            set = true;
            while( mpz_cmp_ui( d, 1 ) == 0 ) {
                mpz_set( x2, x );
                powm( x, x2, num2, n );
                mpz_add( x, x, c );
                mpz_add( x, x, n );
                mpz_mod( x, x, n );
                mpz_set( y2, y );
                powm( y, y2, num2, n );
                mpz_add( y, y, c );
                mpz_add( y, y, n );
                mpz_mod( y, y, n );
                mpz_set( y2, y );
                powm( y, y2, num2, n );
                mpz_add( y, y, c );
                mpz_add( y, y, n );
                mpz_mod( y, y, n );
                
                if ( mpz_cmp( x, y ) >= 0 ) {
                    mpz_sub( sub, x, y );
                    gcd( d, sub, n );
                }
                else {
                    mpz_sub( sub, y, x );
                    gcd( d, sub, n );
                }
                
                if ( mpz_cmp( d, n ) == 0 ) {
                    set = false;
                    break;
                }
            }
            if ( set ) {
                mpz_set( p, d );
                mpz_div( q, n, d );
            }
        }
        else {
            break;
        }
    }
    mpz_inits( mod, d, x, x2, y, y2, c, num2, sub, nullptr );
    return ret;
}

/*
 * Computes p and q using common algorithm for prime factorization
 */
void primeFactor( mpz_t & p, mpz_t & q, const mpz_t & n ) {
    bool set = false;
    if ( isPrime( n, mpz_sizeinbase( n, 2 ) ) ) {
        mpz_set( p, n );
        mpz_set_ui( q, 1 );
        return;
    }
    
    mpz_t n2, mod, i;
    mpz_inits( n2, mod, i, nullptr );
    mpz_set( n2, n );
    mpz_mod_ui( mod, n, 2 );
    
    if ( mpz_cmp_ui( mod, 0 ) == 0 ) {
        mpz_set_ui( p, 2 );
        mpz_div_ui( n2, n2, 2 );
        mpz_set( q, n2 );
        set = true;
    }
    
    if ( !set ) {
        for ( mpz_sqrt( i, n ); (mpz_cmp_ui( i, 3 ) >= 0); mpz_sub_ui( i, i, 2 ) ) {
            mpz_mod( mod, n2, i );
            if ( mpz_cmp_ui( mod, 0 ) == 0 ) {
                mpz_set( p, i );
                mpz_tdiv_q( n2, n2, i );
                mpz_set( q, n2 );
                set = true;
                break;
            }
        }
    }
    
    if ( !set ) {
        mpz_set_ui( p, 0 );
        mpz_set_ui( q, 0 );
        return;
    }
    
    mpz_clears( n2, mod, i, nullptr );
}

/*
 * Computes public and private key from p and q
 */
ReturnValues computeKeys( const mpz_t & p, const mpz_t & q, mpz_t & e, mpz_t & d, bool skip_e = false ) {
    mpz_t p1, q1, phi, g, mul, mod;
    mpz_inits( p1, q1, phi, g, mul, mod, nullptr );
    
    mpz_sub_ui( p1, p, 1 );
    mpz_sub_ui( q1, q, 1 );
    mpz_mul( phi, p1, q1 );
    
    bool skipped = false;
    ReturnValues ret = SUCCESS;
    do {
        do {
            do {
                if ( !skip_e ) {
                    ret = randomNumber( e, mpz_sizeinbase( phi, 2 ) );
                    if ( ret != SUCCESS ) {
                        goto clean;
                    }
                }
                else if ( skipped ) {
                    ret = INVALID_PARAM_E;
                    goto clean;
                }
                else {
                    skipped = true;
                }
            } while ( mpz_cmp_ui( e, 1 ) <= 0 || mpz_cmp( phi, e ) <= 0 );
            gcd( g, e, phi );
        } while ( mpz_cmp_ui( g, 1 ) != 0 );
        invert( d, e, phi) ;
        mpz_mul( mul, e, d );
        mpz_mod( mod, mul, phi );
    } while ( mpz_cmp_ui( mod, 1 ) != 0 );
    
    clean:
        mpz_clears( p1, q1, phi, g, mul, mod, nullptr );
    
    return ret;
}

/*
 * Generates public and private keys
 */
ReturnValues generate_key( size_t b, mpz_t & p, mpz_t & q, mpz_t & n, mpz_t & e, mpz_t & d ) {
    size_t sizep = ( b >> 1 ) + b % 2;
    size_t sizeq = ( b >> 1 );
    do {
        ReturnValues test = randomNumber( p, sizep, true );
        if ( test != SUCCESS ) {
            return test;
        }
    } while ( !isPrime( p, sizep ) );
    
    do {
        ReturnValues test = randomNumber( q, sizeq, true );
        if ( test != SUCCESS ) {
            return test;
        }
    } while ( !isPrime( q, sizeq ) );
    
    mpz_mul( n, p, q );
    return computeKeys( p, q, e, d );
}

/*
 * Encrypt message
 */
ReturnValues encrypt( mpz_t & result, const mpz_t & e, const mpz_t & n, const mpz_t & message ) {
    powm( result, message, e, n);
    return SUCCESS;
}

/*
 * Decrypts message
 */
ReturnValues decrypt( mpz_t & result, const mpz_t & d, const mpz_t & n, const mpz_t & message ) {
    powm( result, message, d, n);
    return SUCCESS;
}

/*
 * Compute primes that were used for key generations and decrypts message
 */
ReturnValues unlimitedPower( mpz_t & p, mpz_t & q, mpz_t & decrypted, mpz_t & e, const mpz_t & n, const mpz_t & encrypted ) {
    primeFactorPollard( p, q, n );
    //primeFactor( p, q, n );
    if ( mpz_cmp_ui( p, 0 ) == 0 || mpz_cmp_ui( q, 0 ) == 0 ) {
        return INVALID_PARAM_N;
    }
    mpz_t d;
    mpz_init( d );
    ReturnValues ret = computeKeys( p, q, e, d, true );
    
    if ( ret == SUCCESS ) {
        ret = decrypt( decrypted, d, n, encrypted );
    }
    mpz_clear( d );
    return ret;
}

int main( int argc, const char ** argv ) {
    Settings     mode      = parseArguments( argc, argv );
    ReturnValues ret_value = SUCCESS;
    
    if ( mode == GENERATE ) {
        mpz_t p, q, n, e, d;
        mpz_inits( p, q, n, e, d, nullptr );
        ret_value = generate_key( std::atoi( argv[2] ), p, q, n, e, d );
        if ( ret_value == SUCCESS ) {
            char * p_str = mpz_get_str( nullptr, FORMAT, p );
            char * q_str = mpz_get_str( nullptr, FORMAT, q );
            char * n_str = mpz_get_str( nullptr, FORMAT, n );
            char * e_str = mpz_get_str( nullptr, FORMAT, e );
            char * d_str = mpz_get_str( nullptr, FORMAT, d );
            std::cout << PREFIX << p_str << ' ' << PREFIX << q_str << ' ' << PREFIX << n_str << ' ' << PREFIX << e_str << ' ' << PREFIX << d_str << std::endl;
            free( p_str );
            free( q_str );
            free( n_str );
            free( e_str );
            free( d_str );
        }
        else {
            std::cerr << "Task Failed" << std::endl;
        }
        mpz_clears( p, q, n, e, d, nullptr );
    }
    else if ( mode == DECRYPT ) {
        mpz_t d, n, message, result;
        mpz_inits( d, n, message, result, nullptr );
        int flag1 = mpz_set_str( d, argv[2] + 2, 16 );
        int flag2 = mpz_set_str( n, argv[3] + 2, 16 );
        int flag3 = mpz_set_str( message, argv[4] + 2, 16 );
        if ( !flag1 && !flag2 && !flag3 ) {
            ret_value = decrypt( result, d, n, message);
            if ( ret_value  == SUCCESS ) {
                char * decrypted( mpz_get_str( nullptr, FORMAT, result ) );
                std::cout << PREFIX << decrypted << std::endl;
                free( decrypted );
            }
            else {
                std::cerr << "Task Failed" << std::endl;
            }
        }
        else {
            ret_value = MPZ_INIT_FAIL;
            std::cerr << "Unable to init MPZ numbers." << std::endl;
        }
        mpz_clears( d, n, message, result, nullptr );
    }
    else if ( mode == ENCRYPT ) {
        mpz_t e, n, message, result;
        mpz_inits( e, n, message, result, nullptr );
        int flag1 = mpz_set_str( e, argv[2] + 2, 16 );
        int flag2 = mpz_set_str( n, argv[3] + 2, 16 );
        int flag3 = mpz_set_str( message, argv[4] + 2, 16 );
        if ( !flag1 && !flag2 && !flag3 ) {
            ret_value = encrypt( result, e, n, message);
            if ( ret_value  == SUCCESS ) {
                char * encrypted( mpz_get_str( nullptr, FORMAT, result ) );
                std::cout << PREFIX << encrypted << std::endl;
                free( encrypted );
            }
            else {
                std::cerr << "Task Failed" << std::endl;
            }
        }
        else {
            ret_value = MPZ_INIT_FAIL;
            std::cerr << "Unable to init MPZ numbers." << std::endl;
        }
        mpz_clears( e, n, message, result, nullptr );
    }
    else if ( mode == BREAK ) {
        mpz_t p, q, e, n, encrypted, decrypted;
        mpz_inits( p, q, e, n, encrypted, decrypted, nullptr );
        int flag1 = mpz_set_str( e, argv[2] + 2, 16 );
        int flag2 = mpz_set_str( n, argv[3] + 2, 16 );
        int flag3 = mpz_set_str( encrypted, argv[4] + 2, 16 );
        if ( !flag1 && !flag2 && !flag3 ) {
            ret_value = unlimitedPower( p, q, decrypted, e, n, encrypted );
            if ( ret_value  == SUCCESS ) {
                char * p_str   = mpz_get_str( nullptr, FORMAT, p );
                char * q_str   = mpz_get_str( nullptr, FORMAT, q );
                char * message = mpz_get_str( nullptr, FORMAT, decrypted );
                std::cout << PREFIX << p_str << ' ' << PREFIX << q_str << ' ' << PREFIX << message << std::endl;
                free( p_str );
                free( q_str );
                free( message );
            }
            else {
                std::cerr << "Task Failed" << std::endl;
            }
        }
        else {
            ret_value = MPZ_INIT_FAIL;
            std::cerr << "Unable to init MPZ numbers." << std::endl;
        }
        mpz_clears(  p, q, e, n, encrypted, decrypted, nullptr );
    }
    else {
        std::cerr << "Invalid arguments." << std::endl; ret_value = INVALID_ARGUMENTS;
    }
    
    return ret_value;
}
