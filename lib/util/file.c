
#include "omnios/file.h"

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#ifdef __linux__
#include <linux/fs.h>
#endif

static uint64_t
dev_get_size(int fd)
{
#if defined(DIOCGMEDIASIZE) /* FreeBSD */
	off_t size;

	if (ioctl(fd, DIOCGMEDIASIZE, &size) == 0) {
		return size;
	}
#elif defined(__linux__) && defined(BLKGETSIZE64)
	uint64_t size;

	if (ioctl(fd, BLKGETSIZE64, &size) == 0) {
		return size;
	}
#endif

	return 0;
}

uint32_t
dev_get_blocklen(int fd)
{
#if defined(DKIOCGETBLOCKSIZE) /* FreeBSD */
	uint32_t blocklen;

	if (ioctl(fd, DKIOCGETBLOCKSIZE, &blocklen) == 0) {
		return blocklen;
	}
#elif defined(__linux__) && defined(BLKSSZGET)
	uint32_t blocklen;

	if (ioctl(fd, BLKSSZGET, &blocklen) == 0) {
		return blocklen;
	}
#endif

	/* Assume 512 if none of the other methods worked */
	return 512;
}

uint64_t
file_get_size(int fd)
{
	struct stat st;

	if (fstat(fd, &st) != 0) {
		return 0;
	}

	if (S_ISLNK(st.st_mode)) {
		return 0;
	}

	if (S_ISBLK(st.st_mode) || S_ISCHR(st.st_mode)) {
		return dev_get_size(fd);
	} else if (S_ISREG(st.st_mode)) {
		return st.st_size;
	}

	/* Not REG, CHR or BLK */
	return 0;
}
