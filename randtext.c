#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//#define DEBUG_ENABLED

#ifdef DEBUG_ENABLED
#define DEBUG(...)	printf("DBG : "__VA_ARGS__)
#else
#define DEBUG(...)
#endif

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(*(a)))

/* Conf */
#define MAX_NB_LETTER_IN_NAME		6
#define MIN_NB_LETTER_IN_NAME		2

#define MAX_CONSECUTIVE_VOWELS		2
#define MAX_CONSECUTIVE_CONSONANT	3

enum letter_type {
	LETTER_TYPE_ERROR = -1,
	LETTER_TYPE_ANY = 0,
	LETTER_TYPE_VOWEL,
	LETTER_TYPE_CONSONANT,
};

static const char alphabet[] = "abcdefghijklmnopqrstuvwxyz";
static const char vowels[] = "aeiouy";
static const char consonants[] = "bcdfghjklmnpqrstvwxz";

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

char randinarray(const char *array, size_t size)
{
	int num;

	do {
		num = drand()*size;
		DEBUG("random num is %d\n", num);
	} while(num == size);

	return array[num];
}

char randletter(enum letter_type lt, char avoid)
{
	char letter;
	int num;
	const char *array;
	size_t arraysize;

	switch (lt) {
	case LETTER_TYPE_VOWEL:
		DEBUG("vowel\n");
		array = vowels;
		arraysize = sizeof(vowels)-1;
		break;
	case LETTER_TYPE_CONSONANT:
		DEBUG("consonant\n");
		array = consonants;
		arraysize = sizeof(consonants)-1;
		break;
	case LETTER_TYPE_ANY:
	default:
		DEBUG("any\n");
		array = alphabet;
		arraysize = sizeof(alphabet)-1;
		break;
	}

	DEBUG("arraysize is %lu\n", arraysize);

	do {
		letter = randinarray(array, arraysize);
	} while(avoid && avoid == letter);

	DEBUG("random letter is \"%c\"\n", letter);
	return letter;
}

unsigned int randwordsize()
{
	int wsize;

	wsize = drand() * (MAX_NB_LETTER_IN_NAME - MIN_NB_LETTER_IN_NAME);
	wsize += MIN_NB_LETTER_IN_NAME;
	DEBUG("wsize is %u\n", wsize);

	return wsize;
}

enum letter_type find_lettertype(char l)
{
	int i;

	if ((l < 'a') || (l > 'z'))
		return LETTER_TYPE_ERROR;

	for (i = 0; i<(sizeof(vowels)-1); i++)
		if (l == vowels[i])
			return LETTER_TYPE_VOWEL;

	return LETTER_TYPE_CONSONANT;
}

int main()
{
	char letter;
	unsigned int wsize;
	int i;
	char word[MAX_NB_LETTER_IN_NAME+1];
	enum letter_type lettertype;
	int consonant_cpt = 0;
	int vowel_cpt = 0;
	char avoid;

	if (random_seed())
		goto error;

	wsize = randwordsize();

	avoid = 0;
	lettertype = LETTER_TYPE_ANY;
	for (i = 0; i<wsize; i++) {
		word[i] = randletter(lettertype, avoid);

		switch (find_lettertype(word[i])) {
		case LETTER_TYPE_VOWEL:
			vowel_cpt++;
			consonant_cpt = 0;

			if (vowel_cpt >= MAX_CONSECUTIVE_VOWELS)
				lettertype = LETTER_TYPE_CONSONANT;
			else
				lettertype = LETTER_TYPE_ANY;
			break;

		case LETTER_TYPE_CONSONANT:
			vowel_cpt = 0;
			consonant_cpt++;

			if (consonant_cpt >= MAX_CONSECUTIVE_CONSONANT)
				lettertype = LETTER_TYPE_VOWEL;
			else
				lettertype = LETTER_TYPE_ANY;
			break;

		default:
			goto error;
			break;
		}

		if (i && word[i] == word[i-1])
			avoid = word[i];
		else
			avoid = 0;
	}
	word[wsize] = '\0';

	DEBUG("\n");
	printf("random word is \"%s\"\n", word);

	return 0;
error:
	fprintf(stderr, "Error... exit.\n");
	return 1;
}
