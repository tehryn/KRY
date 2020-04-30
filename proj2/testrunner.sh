#!/bin/bash

function test_out() {
	out=$1
	ref=$2

	echo "$out" | grep "$ref" >/dev/null && echo "OK" || echo "FAIL: $out"
}

function test_mod_mult_inverse() {
	x=$1
	n=$2
	r=$3

	out=$(./kry -m $x $n)
	test_out "$out" "$r"
}

function encrypt_with_gout() {
	out_p=$1
	out_q=$2
	out_n=$3
	out_e=$4
	out_d=$5
	msg=$6

	./kry -e $out_e $out_n $msg
}

function decrypt_with_gout() {
	out_p=$1
	out_q=$2
	out_n=$3
	out_e=$4
	out_d=$5
	cipher=$6

	./kry -d $out_d $out_n $cipher
}

BITLEN=4
P="0xb"
Q="0xd"
N="0x8F"
#PHI = 120
E="0x07"
D="0x67"

M="0x9"
C="0x30" # encrypt: m^e % n, decrypt: c^d % n

make clean
make

echo "Testing multiplicative inverse"
test_mod_mult_inverse 17 120 113
test_mod_mult_inverse 59 120 59
test_mod_mult_inverse 21 100 81
test_mod_mult_inverse 29 144 5
test_mod_mult_inverse 55 144 55


echo "Testing -g"
out=$(./kry -g $BITLEN)
test_out "$out" "0x[0-9a-f] 0x[0-9a-f]* 0x[0-9a-f]* 0x[0-9a-f]* 0x[0-9a-f]*"

echo "Testing -e"
out=$(./kry -e $E $N $M)
test_out "$out" "$C"

echo "Testing -d"
out=$(./kry -d $D $N $C)
test_out "$out" "$M"

echo "Testing -g, -e, -d"
out=$(./kry -g 4)
c_test=$(encrypt_with_gout $out $M)
m_test=$(decrypt_with_gout $out $c_test)
test_out "$m_test" "$M"

echo "Testing -b"
out=$(./kry -b $E $N $C)
test_out "$out" "$P $Q $M"
