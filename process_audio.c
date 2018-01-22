#include <stdio.h>
#include <math.h>
#include <sndfile.h>	/* libsndfile */
#include "main.h"
#include "compressor.h"	/* FFT-based multiband compression */

#define MODE	2

void process_audio(Buf *ibuf, int iframes, int channels, Buf *obuf, float ratio1, float ratio2, float ratio3)
{
#if (MODE == 1)
	/* just copy input to output */
	int i, j;
	float *ip;
	for (i=0; i<channels; i++) {
		ip = ibuf->buf[i];
		for (j=0; j<iframes; j++) {
			obuf->buf[i][j] = ip[j];
		}
	}
#else
#if (MODE == 2)
	/* this runs the compressor as a pass through right now */
	int i;
	/* for each channel */
	for (i=0; i<channels; i++) {
		/* runs fft, applies parity gain, and ifft */
		/* should just play stuff back normally */
		compress(ibuf->buf[i], iframes, obuf->buf[i], ratio1, ratio2, ratio3);
	}

#endif
#endif
}
