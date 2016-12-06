#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


/* default functions */
uint32_t rotate_right(uint32_t num, uint32_t bits)
{
	return ((num >> bits) | (num << (32 -bits)));
}

#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))

/* end default functions */

/* Crazy functions */

uint32_t Ch(uint32_t x, uint32_t y, uint32_t z){
	uint32_t tmp, tmp2;
	tmp = x & y;
	tmp2 = ~x & z;
	return tmp ^ tmp2;
}

uint32_t Maj(uint32_t x, uint32_t y, uint32_t z){
	uint32_t tmp, tmp2, tmp3;

	tmp = x & y;
	tmp2 = x & z;
	tmp3 = y & z;

	return tmp ^ tmp2 ^ tmp3;
}

uint32_t S0(uint32_t x){
	uint32_t tmp, tmp2, tmp3;

/*	tmp = x >> 2;
	tmp2 = x >> 13;
	tmp3 = x >> 22;
*/
	tmp = rotate_right(x, 2);
	tmp2 = rotate_right(x, 13);
	tmp3 = rotate_right(x, 22);

	return tmp ^ tmp2 ^ tmp3;

}

uint32_t S1(uint32_t x){
	uint32_t tmp, tmp2, tmp3;
/*
	tmp = x >> 6;
	tmp2 = x >> 11;
	tmp3 = x >> 25;
*/
	tmp = rotate_right(x, 6);
	tmp2 = rotate_right(x, 11);
	tmp3 = rotate_right(x, 25);

	return tmp ^ tmp2 ^ tmp3;
}

uint32_t s0(uint32_t x){
	uint32_t tmp, tmp2, tmp3;

	/*
	tmp = x >> 7;
	tmp2 = x >> 18;
	tmp3 = rotate_right(x, 3);
	*/
	tmp = rotate_right(x, 7);
	tmp2 = rotate_right(x, 18);
	tmp3 = x >> 3;

	return tmp ^ tmp2 ^ tmp3;

}

uint32_t s1(uint32_t x){
	uint32_t tmp, tmp2, tmp3;

	/*
	tmp = x >> 17;
	tmp2 = x >> 19;
	tmp3 = rotate_right(x, 10);

	*/
	tmp = rotate_right(x, 17);
	tmp2 = rotate_right(x, 19);
	tmp3 = x >> 10;

	return tmp ^ tmp2 ^ tmp3;
}

// End of crazy functions

char *pad_message(char *M, int size){
	assert(size < 512);

	M[size] = 1 << 7;
	return M;
}

char **parse_message(char *M, int size){
	char **block;
	int entries = size / 512;

	block = (char **) malloc(entries * sizeof(char *));

	for (int i = 0 ; i < entries ; i++){
		block[i] = (char *) malloc(512*sizeof(char));
		memcpy(block[i], M, 512);
	}

	return block;
}

uint32_t *do_core(char **set, uint32_t * h0, int entries){
	uint32_t a, b, c, d, e, f, g, h;
	uint32_t t1, t2;
	uint32_t w[64];
	uint32_t *ptr;
	uint32_t *H;

	uint32_t k[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98,
		0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe,
		0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6,
		0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3,
		0xd5a79147, 0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138,
		0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e,
		0x92722c85, 0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116,
		0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
		0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814,
		0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};


	// return output
	H = (uint32_t *) malloc(8*sizeof(uint32_t));

	// setting w
	memset(w, 0, 64*sizeof(uint32_t));

	for (int i = 1 ; i <= entries ; i++){
		a = h0[0];
		b = h0[1];
		c = h0[2];
		d = h0[3];
		e = h0[4];
		f = h0[5];
		g = h0[6];
		h = h0[7];
		ptr = (uint32_t *) set[entries -1];

		// Defining the W array for each M
		for (int z = 0; z<= 15; z++){
			w[z] = SWAP_UINT32(ptr[z]);
		}

		for (int z = 16; z< 64; z++){
			w[z] = s1(w[z-2]) + w[z-7] + s0(w[z-15]) + w[z-16];
		}

		// Main loop
		for (int j = 0; j <= 63; j++){
			t1 = h + S1(e) + Ch(e, f, g) + k[j] + w[j];
			t2 = S0(a) + Maj(a,b,c);
			h = g;
			g = f;
			f = e;
			e = d + t1;
			d = c;
			c = b;
			b = a;
			a = t1 + t2;
		}

		// Create the hash output
		H[0] = a + H[0];
		H[1] = b + H[1];
		H[2] = c + H[2];
		H[3] = d + H[3];
		H[4] = e + H[4];
		H[5] = f + H[5];
		H[6] = g + H[6];
		H[7] = g + H[7];
	}

	return H;
}

int main(int argc, char **argv){
	uint32_t h0[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	                   0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

	uint32_t *H;

	char *M;
	char **set;
	int size = 512;

	if (argc < 2) {
		printf("Usage:\n%s <Message>\n", argv[0]);
		exit(-1);
	}

	M = (char *) malloc(size);
	memset(M, 0, size);
	memcpy(M, argv[1], strlen(argv[1]));

	M = pad_message(M, strlen(argv[1]));
	set = parse_message(M, size);

	H = do_core(set, h0, size/512);

	for (int i = 0; i <=8 ; i++){
		printf("%08x", H[i]);
	}

	printf("\n");

	return 0;
}
