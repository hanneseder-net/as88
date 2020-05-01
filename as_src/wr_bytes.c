/*
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
#include <unistd.h>

#include "obj.h"
#include "comm7.h"

static int maxchunk = 16384;

/*
 * Just write "cnt" bytes to file-descriptor "fd".
 */
void wr_bytes(int fd, char *string, long cnt) {
	while (cnt) {
		int n = cnt >= maxchunk ? maxchunk : cnt;

		if (write(fd, string, n) != n)
			wr_fatal();
		string += n;
		cnt -= n;
	}
}
