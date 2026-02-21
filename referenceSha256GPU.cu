#include <stdio.h>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <stdint.h>

#include <memory.h>
#include <string.h>
#include <stdint.h>

#define PASSWORDSIZE 3
#define BATCH 1

#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> (32-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << (32-(b))))

#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x,y,z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))

/**************************** VARIABLES *****************************/
__constant__ uint32_t k[64] = {
	0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
	0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
	0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
	0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
	0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
	0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
	0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
	0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

__global__ void sha256_init_gpu(uint32_t *in, uint32_t *out)
{
	uint32_t tid = threadIdx.x;
	//for(int i=0; i<16; i++)
	//	printf("%x\n", in[i]);
	uint32_t a, b, c, d, e, f, g, h, w[64];
	uint32_t i, j, t1, t2;
	for (i = 0; i < 16; i++ )
		w[i] = in[i] ;
	for (i=16 ; i < 64; i++)
		w[i] = SIG1(w[i - 2]) + w[i - 7] + SIG0(w[i - 15]) + w[i - 16];
	// Initialize hash value
	a = out[0];
	b = out[1];
	c = out[2];
	d = out[3];
	e = out[4];
	f = out[5];
	g = out[6];
	h = out[7];

	for (i = 0; i < 64; ++i) {
		t1 = h + EP1(e) + CH(e,f,g) + k[i] + w[i];
		t2 = EP0(a) + MAJ(a,b,c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}

	out[0] += a;
	out[1] += b;
	out[2] += c;
	out[3] += d;
	out[4] += e;
	out[5] += f;
	out[6] += g;
	out[7] += h;
	//printf("%x %x %x %x %x %x %x %x\n", out[0], out[1], out[2], out[3], out[4], out[5], out[6], out[7]);
}

/*********************** FUNCTION DEFINITIONS ***********************/
int main(int argc, char** argv)
{
	uint32_t *text1CPU, *inHost, *outHost;
	uint32_t *inDev, *outDev;

	// GPU code starts here
	cudaMalloc((uint32_t**)&inDev, 16*sizeof(uint32_t));	// 512-bit buffer to hold one SHA-256 operation
	cudaMalloc((uint32_t**)&outDev, 16*sizeof(uint32_t));	// 
	cudaMallocHost((void **) &inHost, 16*sizeof(uint32_t));	// 512-bit buffer to hold one SHA-256 operation
	cudaMallocHost((void **) &outHost, 16*sizeof(uint32_t));
	cudaMallocHost((void **) &text1CPU, sizeof(uint32_t));
	
	text1CPU[0] = 0x61626300;	// "abc" in ASCII code
	printf("password: %x\n", text1CPU[0]);

	if(PASSWORDSIZE<4)
	{
		if(PASSWORDSIZE==1)
			text1CPU[0] = text1CPU[0] | 0x800000;	// pad with 0x800000
		else if(PASSWORDSIZE==2)
			text1CPU[0] = text1CPU[0] | 0x8000;	// pad with 0x8000
		else if(PASSWORDSIZE==3)
		{
			inHost[0] = text1CPU[0] | 0x80;	// pad with 0x80
			for(int i=1; i<14; i++)			// followed by all zero
				inHost[i] = 0;	
			inHost[14] = 0;
			inHost[15] = 24;				// last 64-bit is the length of message (3B = 24 bit)
		}
			outHost[0] = 0x6a09e667;
			outHost[1] = 0xbb67ae85;
			outHost[2] = 0x3c6ef372;
			outHost[3] = 0xa54ff53a;
			outHost[4] = 0x510e527f;
			outHost[5] = 0x9b05688c;
			outHost[6] = 0x1f83d9ab;
			outHost[7] = 0x5be0cd19;
	}
	cudaMemcpy(inDev, inHost, 16*sizeof(uint32_t), cudaMemcpyHostToDevice);
	cudaMemcpy(outDev, outHost, 16*sizeof(uint32_t), cudaMemcpyHostToDevice);
	sha256_init_gpu<<<1,1>>>(inDev, outDev);
	cudaMemcpy(outHost, outDev, 16*sizeof(uint32_t), cudaMemcpyDeviceToHost);
	// Expected hash value for "abc"
	// ba7816bf 8f01cfea 414140de 5dae2223 b00361a3 96177a9c b410ff61 f20015ad (refer FIPS180-2, pg. 40)
	printf("%x %x %x %x %x %x %x %x", outHost[0], outHost[1], outHost[2], outHost[3], outHost[4], outHost[5], outHost[6], outHost[7]);
	cudaDeviceReset();
	return 0;
}

