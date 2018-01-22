# A Multiband Compressor
A rudimentary multiband compressor implemented in C for C Programming for Audio in the NYU Music Technology Master's program.

## Use
The program operates with a command line interface, accepting several arguments for an input audio file, an output audio file, and 3 compression factors to apply to high, mid, and low range frequency bands. The compression factor functions similar to the ratio paramenter of a traditional multiband compressor.

## Sample command line inputs
The compressor is somewhat restricted in it's capabilities, and the right set of parameters for a particular input may need to be tailored over the course of a few attempts. Mostly, difficulties arise from lack of protection against clipping when drastically boosting the gain in a particular band.

A sample command line input could be as follows:
`./main signals/sig1.wav signals/output1.wav -h 2 -m .5 -l 3`

The first argument is the input signal filepath, the second is the destination to which the output .wav file will be saved. The three compression factors are noted by the characters `-h` for the high band, `-m` for the mid-range band, and `-l` for the low band. These arguments can occur in any order, and will default to 1 if ommitted. 

The compression factor value sets the ratio of the compressor, and moves the knee of the compression curve vertically to adjust. Values higher value will apply increasingly greater compression (essentially, soft sounds become louder and loud sounds become softer). Values below 1 will actually expand the signal (increase the experienced dynamic difference in the frequency band). A value of 1 is equivalent to a pass through on a particular band.

Pass through:
`./main signals/sig1.wav signals/output1.wav -h 1 -m 1 -l 1`
Compression:
`./main signals/sig1.wav signals/output1.wav -h 2 -m 1 -l 1.5`
Expansion:
`./main signals/sig1.wav signals/output1.wav -h .75 -m .5 -l .75`
