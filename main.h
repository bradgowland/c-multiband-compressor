#ifndef __MAIN_H
#define __MAIN_H

#define MAX_CHN		2

struct BUF_tag {
	float *buf[MAX_CHN];
};

typedef struct BUF_tag Buf;

#endif /* __MAIN_H */