#include <stdio.h>
#include <stdlib.h> 	/* malloc */
#include <stdbool.h>	/* true, false */
#include <string.h>		/* memset */
#include <sndfile.h>	/* libsndfile */
#include "main.h"	/* data structures */
#include "process_audio.h"	/* external function prototypes */

/* local function prototypes */
bool read_input(SNDFILE *sfile, SF_INFO *sfinfo, Buf *buf);
bool write_output(SNDFILE *sfile, SF_INFO *sfinfo, Buf *buf, long frames);

/* usage */
void usage()
{
    // usage help
    printf("Usage: ./main ifile.wav ofile.wav [-h ratio1] [-m ratio2] [-l ratio3]\n");
    printf("Input an integer 1 - 10 for ratio of compression in high, mid, and low bands.\n");
    printf("Omitting any ratio will default to pass through on that band.\n");
}

int main(int argc, char *argv[])
{
	char *ifilename, *ofilename;
	int i, oframes;
	float hRatio = 0;
	float mRatio = 0;
	float lRatio = 0;	

	/* my data structures */
	Buf ibuf, obuf;
	/* libsndfile data structures */
	SNDFILE *isfile, *osfile; 
	SF_INFO isfinfo, osfinfo;

	/* zero libsndfile structures */
	memset(&isfinfo, 0, sizeof(isfinfo));
  	memset(&osfinfo, 0, sizeof(osfinfo));

  	/* zero buffer pointers in Buf structures */
  	for (i=0; i<MAX_CHN; i++) {
  		ibuf.buf[i] = 0;
  		obuf.buf[i] = 0;
  	}

	// Parse command line and open all files
    if(argc < 3){
        usage();
        return -1;
    } else if (argc > 9) {
    	usage();
    	return -1;
    }

    // set filenames
    ifilename = argv[1];
    ofilename = argv[2];

    /* assign args appropriately */
	if (argc > 4) { 
	    if (strcmp(argv[3], "-h") == 0) {
	    	hRatio = atof(argv[4]);
	    } else if (strcmp(argv[3], "-m") == 0) {
	    	mRatio = atof(argv[4]);
	    } else if (strcmp(argv[3], "-l") == 0) {
	    	lRatio = atof(argv[4]);
	    }
	}
	if (argc > 6) { 
	    if (strcmp(argv[5], "-h") == 0) {
	    	hRatio = atof(argv[6]);
	    } else if (strcmp(argv[5], "-m") == 0) {
	    	mRatio = atof(argv[6]);
	    } else if (strcmp(argv[5], "-l") == 0) {
	    	lRatio = atof(argv[6]);
	    }
	} 
	if (argc > 8) { 
	    if (strcmp(argv[7], "-h") == 0) {
	    	hRatio = atof(argv[8]);
	    } else if (strcmp(argv[7], "-m") == 0) {
	    	mRatio = atof(argv[8]);
	    } else if (strcmp(argv[7], "-l") == 0) {
	    	lRatio = atof(argv[8]);
	    }
	}
	// set defaults if no match found
	if (hRatio == 0) {
		hRatio = 1;
	}
	if (mRatio == 0) {
		mRatio = 1;
	}
	if (lRatio == 0) {
		lRatio = 1;
	}

	// print user selections
    printf("%s will be used as the input file.\n", ifilename);
    printf("%s will be used as the output file.\n", ofilename);
    printf("The high band compression factor is %f\n", hRatio);
    printf("The mid band compression factor is %f\n", mRatio);
    printf("The low band compression factor is %f\n", lRatio);

    /* set format to zero, req by libsndfile */
    isfinfo.format = 0;
		
	/* open files */
	isfile = sf_open(ifilename, SFM_READ, &isfinfo);

	/* Print input file information */
	printf("Input audio file %s:\n\tFrames: %d Channels: %d Samplerate: %d\n", 
		ifilename, (int)isfinfo.frames, isfinfo.channels, isfinfo.samplerate);

	/* When opening a file for writing, the caller must fill in structure members 
	* 	samplerate, channels, and format. 
	* Make these the same as input file.
	*/
	osfinfo.samplerate = isfinfo.samplerate;
	osfinfo.channels = isfinfo.channels;
	osfinfo.format = isfinfo.format;
	oframes = isfinfo.frames;
	/* open output file */
	osfile = sf_open(ofilename, SFM_WRITE, &osfinfo);


	// Problem 2:
	// ToDo: Allocate all buffers and read input into buffers

	/* Allocate separate buffers for each channel of 
	 * input, reverb and output files
	 */

	// input
	for (i=0; i<isfinfo.channels; i++) {
		ibuf.buf[i] = malloc(sizeof(float) * isfinfo.frames);
	}
	// output
	for (i=0; i<isfinfo.channels; i++) {
		/* output is length of input + length of reverb -1 */
		obuf.buf[i] = malloc(sizeof(float) * (isfinfo.frames));
	}
	printf("Allocated buffers\n");

	/* read interleaved data from files into de-interleaved buffers */
	/* input */
	if ( !read_input(isfile, &isfinfo, &ibuf) ) {
		fprintf(stderr, "ERROR: Cannot read input file %s", ifilename);
		return -1;
	}
	printf("Read input file\n");

	/* process each channel of input with reverb impulse response
	 * to make output
	 */
	printf("Processing audio\n");
	process_audio(&ibuf, isfinfo.frames, isfinfo.channels, &obuf, hRatio, mRatio, lRatio);
	printf("Finished processing audio\n");

	/* interleave output data and write output file */
	if ( !write_output(osfile, &osfinfo, &obuf, oframes) ) {
		fprintf(stderr, "ERROR: Cannot write output file %s", ofilename);
		return -1;
	}

	/* Must close file; output will not be written correctly if you do not do this */
	sf_close (isfile);
	sf_close (osfile);

	/* free all buffer storage */
	printf("Freeing buffers\n");
	for (i=0; i<MAX_CHN; i++) {
		if (ibuf.buf[i] != NULL)
			free(ibuf.buf[i]);
		if (obuf.buf[i] != NULL)
			free(obuf.buf[i]);
	}

}

bool read_input(SNDFILE *sfile, SF_INFO *sfinfo, Buf *buf)
{
	int i, j, count;
	float frame_buf[MAX_CHN]; /* to hold one sample frame of audio data */
	for (i=0; i<sfinfo->frames; i++) {
		/* for each frame */
		if ( (count = sf_read_float (sfile, frame_buf, sfinfo->channels)) != sfinfo->channels) {
			fprintf(stderr, "Error: on sample frame %d\n", i);
			return false;
		}
		//ToDo: de-interleave the frame[j] into separate channel buffers buf->buf[j][i]
		for (j = 0; j < sfinfo->channels; j++) {
			buf->buf[j][i] = frame_buf[j];
		}
	}
	return true;
}

bool write_output(SNDFILE *sfile, SF_INFO *sfinfo, Buf *buf, long frames)
{
	int i, j, count;
	float frame_buf[2]; /* to hold one sample frame of audio data */
	for (i=0; i<frames; i++) {
		/* for each frame */
		//ToDo: interleave separate channel buffers buf->buf[j][i] into a frame_buf[j]
		for (j = 0; j < sfinfo->channels; j++) {
			frame_buf[j] = buf->buf[j][i];
		}

		// count = sf_write_float (sfile, frame_buf, sfinfo->channels);
		if ( (count = sf_write_float (sfile, frame_buf, sfinfo->channels)) != sfinfo->channels) {
		  	fprintf(stderr, "Error: on sample frame %d\n", i);
		  	return false;
		}
	}
	printf("Wrote %ld frames\n", frames);
	return true;
}