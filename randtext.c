#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define DEBUG_ENABLED

#ifdef DEBUG_ENABLED
#define DEBUG(...)	printf(__VA_ARGS__)
#else
#define DEBUG(...)
#endif

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(*(a)))

static const char alphabet[] = "abcdefghijklmnopqrstuvxyz";
static const char vowels[] = "aeiouy";
static const char consonants[] = "bcdfghjklmnpqrstvxz";

int random_seed()
{
	int fd;
	unsigned int seed;
	int ret;

	fd = open("/dev/random", O_RDONLY);

	if (fd < 0)
		return -errno;

	ret = read(fd, &seed, sizeof(seed));
	if (ret < 0)
		return -errno;
	if (ret != sizeof(seed))
		return -EIO;

	close(fd);
	DEBUG("seed is %u\n", seed);

	srandom(seed);

	return 0;
}

double drand()
{
	double tmp = ((double)random())/RAND_MAX;
	DEBUG("random dnum is %lf\n", tmp);
	return tmp;
}

int main()
{
	char letter;
	int num;

	if (random_seed())
		goto error;

	num = drand()*2;
	DEBUG("random num is %d\n", num);

	letter = 'a' + num;
	printf("random letter is \"%c\"\n", letter);

	return 0;
error:
	fprintf(stderr, "Error... exit.\n");
	return 1;
}
