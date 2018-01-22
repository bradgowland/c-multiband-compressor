gcc -g -std=c99 -Wall -I/usr/local/include -o main \
	main.c compressor.c process_audio.c -L/usr/local/lib -lsndfile
