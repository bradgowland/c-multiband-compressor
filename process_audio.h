#ifndef __PROCESS_AUDIO
#define __PROCESS_AUDIO

void process_audio(Buf *ibuf, int iframes, int channels, Buf *obuf, float ratio1, float ratio2, float ratio3);

#endif /* __PROCESS_AUDIO */