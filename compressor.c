/*
** Simple multiband compression
**
** Compressor.c based on convolve.c assignment
** Uses fft, ifft, and elements of structure
**
** M. Farbood, August 5, 2011
**
** Function that convolves two signals.
** Factored discrete Fourier transform, or FFT, and its inverse iFFT.
**
** fft and ifft are taken from code for the book,
** Mathematics for Multimedia by Mladen Victor Wickerhauser
** The function convolve is based on Stephen G. McGovern's fconv.m
** Matlab implementation.
**
*/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef float real;
typedef struct{real Re; real Im;} complex;

#ifndef PI
#define PI 3.14159265358979323846264338327950288
#endif

/* Print a vector of complexes as ordered pairs. */
static void print_vector(const char *title, complex *x, int n)
{
  int i;
  printf("%s (dim=%d):", title, n);
  for(i=0; i<n; i++ ) printf(" %8.3f,%8.3f ", x[i].Re,x[i].Im);
  putchar('\n');
  return;
}

/* Multiply two complex numbers */
complex complex_mult(complex a, complex b)
{
  complex c;
  c.Re =  (a.Re * b.Re) + (a.Im * b.Im * -1);
  c.Im =  a.Re * b.Im + a.Im * b.Re;
  return c;
}

/*
   fft(v,N):
   [0] If N==1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute fft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute fft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = -sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */

void fft( complex *v, int n, complex *tmp )
{
  if(n > 1) {			/* otherwise, do nothing and return */
    int k,m;
    complex z, w, *vo, *ve;
    ve = tmp;
    vo = tmp + n/2;
    for(k = 0; k < n/2; k++) {
      ve[k] = v[2*k];
      vo[k] = v[2*k+1];
    }
    fft(ve, n/2, v);		/* FFT on even-indexed elements of v[] */
    fft(vo, n/2, v);		/* FFT on odd-indexed elements of v[] */
    for(m=0; m<n/2; m++) {
      w.Re = cos(2 * PI * m/(double)n);
      w.Im = -sin(2 * PI * m/(double)n);
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      v[m].Re = ve[m].Re + z.Re;
      v[m].Im = ve[m].Im + z.Im;
      v[m+n/2].Re = ve[m].Re - z.Re;
      v[m+n/2].Im = ve[m].Im - z.Im;
    }
  }
  return;
}

/*
   ifft(v,N):
   [0] If N == 1 then return.
   [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
   [2] Compute ifft(ve, N/2);
   [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
   [4] Compute ifft(vo, N/2);
   [5] For m = 0 to N/2-1, do [6] through [9]
   [6]   Let w.re = cos(2*PI*m/N)
   [7]   Let w.im = sin(2*PI*m/N)
   [8]   Let v[m] = ve[m] + w*vo[m]
   [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void ifft(complex *v, int n, complex *tmp)
{
  if(n > 1) {			/* otherwise, do nothing and return */
    int k, m;
    complex z, w, *vo, *ve;
    ve = tmp; vo = tmp + n/2;
    for(k = 0; k < n/2; k++) {
      ve[k] = v[2*k];
      vo[k] = v[2*k+1];
    }
    ifft(ve, n/2, v);		/* FFT on even-indexed elements of v[] */
    ifft(vo, n/2, v);		/* FFT on odd-indexed elements of v[] */
    for(m = 0; m < n/2; m++) {
      w.Re = cos(2 * PI * m/(double)n);
      w.Im = sin(2 * PI * m/(double)n);
      z.Re = w.Re*vo[m].Re - w.Im*vo[m].Im;	/* Re(w*vo[m]) */
      z.Im = w.Re*vo[m].Im + w.Im*vo[m].Re;	/* Im(w*vo[m]) */
      v[m].Re = ve[m].Re + z.Re;
      v[m].Im = ve[m].Im + z.Im;
      v[m+n/2].Re = ve[m].Re - z.Re;
      v[m+n/2].Im = ve[m].Im - z.Im;
    }
  }
  return;
}

int compress(float *x, int lenX, float *output, float hRatio, float mRatio, float lRatio)
{
  complex *xComp = NULL;
  complex *HiComp = NULL;
  complex *MidComp = NULL;
  complex *LoComp = NULL;
  complex *scratch = NULL;

  int lenY = lenX;
  int currPow = 0;
  int lenY2 = pow(2, currPow);
  int i;

  /* Get first first power of two larger than lenY */
  while (lenY2 < lenY) {
    currPow++;
    lenY2 = pow(2, currPow);
  }
  //printf("len: %d %d %d %d\n", lenX, lenY, lenY2);

  /* Allocate a lot of memory */
  scratch = calloc(lenY2, sizeof(complex));
  if (scratch == NULL) {
    printf("Error: unable to allocate memory for convolution. Exiting.\n");
    exit(1);
  }
  xComp = calloc(lenY2, sizeof(complex));
  if (xComp == NULL) {
    printf("Error: unable to allocate memory for convolution. Exiting.\n");
    exit(1);
  }
  HiComp = calloc(lenY2, sizeof(complex));
  if (HiComp == NULL) {
    printf("Error: unable to allocate memory for convolution. Exiting.\n");
    exit(1);
  }
  MidComp = calloc(lenY2, sizeof(complex));
  if (MidComp == NULL) {
    printf("Error: unable to allocate memory for convolution. Exiting.\n");
    exit(1);
  }
  LoComp = calloc(lenY2, sizeof(complex));
  if (LoComp == NULL) {
    printf("Error: unable to allocate memory for convolution. Exiting.\n");
    exit(1);
  }

  /* Copy over real values */
  for (i = 0; i < lenX; i++) {
    xComp[i].Re = x[i];
    // possibly set imag to zero
  }

  /* FFT of x */
  // print_vector("Input", xComp, 40);
  fft(xComp, lenY2, scratch);
  // print_vector(" FFT", xComp, lenY2);

  // split into bands
  int nyquist = lenY2 / 2;
  int band1 = 1024;
  int band2 = nyquist / 3;
  int band3 = nyquist * 2 / 3;

  // high pass
  for (int i = 0; i < lenY2; i++) {
    if (i > band2 && i < band3) {
      HiComp[i].Re = xComp[i].Re;
      HiComp[i].Im = xComp[i].Im;
    } else {
      HiComp[i].Re = 0;
      HiComp[i].Im = 0;
    }
  }

  // band pass
  for (int i = 0; i < lenY2; i++) {
    if (i > band1 && i < band2) {
      MidComp[i].Re = xComp[i].Re;
      MidComp[i].Im = xComp[i].Im;
    } else {
      MidComp[i].Re = 0;
      MidComp[i].Im = 0;
    }
  }

  // low pass
  for (int i = 0; i < lenY2; i++) {
    if (i < band1) {
      LoComp[i].Re = xComp[i].Re;
      LoComp[i].Im = xComp[i].Im;
    } else {
      LoComp[i].Re = 0;
      LoComp[i].Im = 0;
    }
  }

  /* Take the inverse FFT of each band */
  ifft(HiComp, lenY2, scratch);
  ifft(MidComp, lenY2, scratch);
  ifft(LoComp, lenY2, scratch);
  //print_vector("iFFT", HiComp, lenY2);
  //print_vector("iFFT", MidComp, lenY2);
  //print_vector("iFFT", LoComp, lenY2);

  // normalize and process band separated signals
  float scale = 1.0/lenY2;

  // compression step function components
  float hKnee = 1.0/(hRatio + 1);
  float mKnee = 1.0/(mRatio + 1);
  float lKnee = 1.0/(lRatio + 1);

  float hOffset = (hRatio - 1) / hRatio;
  float mOffset = (mRatio - 1) / mRatio;
  float lOffset = (lRatio - 1) / lRatio;

  // High Band - compressed
  for (int i = 0; i < lenY2; i++) {
    HiComp[i].Re = HiComp[i].Re * scale;

    if (fabs(HiComp[i].Re) < hKnee) {
      HiComp[i].Re = HiComp[i].Re * (hRatio);
    } else {
      HiComp[i].Re = (HiComp[i].Re / hRatio) + hOffset;
    }
  }

  // Mid Band - normalize and pass through
  for (int i = 0; i < lenY2; i++) {
    MidComp[i].Re = MidComp[i].Re * scale;

    if (fabs(MidComp[i].Re) < mKnee) {
      MidComp[i].Re = MidComp[i].Re * (mRatio);
    } else {
      MidComp[i].Re = (MidComp[i].Re / mRatio) + mOffset;
    }
  }

  // Low Band - compressed
  for (int i = 0; i < lenY2; i++) {
    LoComp[i].Re = LoComp[i].Re * scale;

    if (fabs(LoComp[i].Re) < lKnee) {
      LoComp[i].Re = LoComp[i].Re * (lRatio);
    } else {
      LoComp[i].Re = (LoComp[i].Re / lRatio) + lOffset;
    }
  }

  // combine bands
  for (i = 0; i < lenY; i++) {
    output[i] = HiComp[i].Re + MidComp[i].Re + LoComp[i].Re;
  }

  // scale to make output rms value be equal to input rms value
  float rms_i = 0;
  float rms_o = 0;
  float gain;

  // input rms
  for (int i = 0; i < lenX; i++) {
    rms_i +=  x[i] * x[i];
  }
  rms_i = sqrt(rms_i / lenX);

  // output rms
  for (int i = 0; i < lenX; i++) {
    rms_o +=  output[i] * output[i];
  }
  rms_o = sqrt(rms_o / lenX);

  // determine gain coefficient
  gain = rms_i / rms_o;

  // scale to output
  for (i = 0; i < lenY; i++) {
    output[i] = output[i] * gain;
  }
  // printf("%f\n", output);
  
  // free buffers
  free(scratch);
  free(xComp);
  free(HiComp);
  free(MidComp);
  free(LoComp);

  return lenY;
}
