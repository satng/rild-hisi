#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>


static pthread_t s_tid_reader;
static int s_fd = -1;    /* fd of the AT channel */
#define MAX_AT_RESPONSE (8 * 1024)
static char s_ATBuffer[MAX_AT_RESPONSE+1];
static char *s_ATBufferCur = s_ATBuffer;
static int s_readCount = 0;


/**
 * Returns a pointer to the end of the next line
 * special-cases the "> " SMS prompt
 *
 * returns NULL if there is no complete line
 */
static char * findNextEOL(char *cur)
{
    if (cur[0] == '>' && cur[1] == ' ' && cur[2] == '\0') {
        /* SMS prompt character...not \r terminated */
        return cur+2;
    }

    // Find next newline
    while (*cur != '\0' && *cur != '\r' && *cur != '\n') cur++;

    return *cur == '\0' ? NULL : cur;
}


static const char *readline()
{
    ssize_t count;

    char *p_read = NULL;
    char *p_eol = NULL;
    char *ret;

    /* this is a little odd. I use *s_ATBufferCur == 0 to
     * mean "buffer consumed completely". If it points to a character, than
     * the buffer continues until a \0
     */
    if (*s_ATBufferCur == '\0') {
        /* empty buffer */
        s_ATBufferCur = s_ATBuffer;
        *s_ATBufferCur = '\0';
        p_read = s_ATBuffer;
    } else {   /* *s_ATBufferCur != '\0' */
        /* there's data in the buffer from the last read */

        // skip over leading newlines
        while (*s_ATBufferCur == '\r' || *s_ATBufferCur == '\n')
            s_ATBufferCur++;

        p_eol = findNextEOL(s_ATBufferCur);

        if (p_eol == NULL) {
            /* a partial line. move it up and prepare to read more */
            size_t len;

            len = strlen(s_ATBufferCur);

            memmove(s_ATBuffer, s_ATBufferCur, len + 1);
            p_read = s_ATBuffer + len;
            s_ATBufferCur = s_ATBuffer;
        }
        /* Otherwise, (p_eol !- NULL) there is a complete line  */
        /* that will be returned the while () loop below        */
    }

    while (p_eol == NULL) {
        if (0 == MAX_AT_RESPONSE - (p_read - s_ATBuffer)) {
            printf("ERROR: Input line exceeded buffer\n");
            /* ditch buffer and start over again */
            s_ATBufferCur = s_ATBuffer;
            *s_ATBufferCur = '\0';
            p_read = s_ATBuffer;
        }

        do {
            count = read(s_fd, p_read,
                            MAX_AT_RESPONSE - (p_read - s_ATBuffer));
        } while (count < 0 && errno == EINTR);

        if (count > 0) {
            printf( "<< %s, count %d\n", p_read, count );
            s_readCount += count;

            p_read[count] = '\0';

            // skip over leading newlines
            while (*s_ATBufferCur == '\r' || *s_ATBufferCur == '\n')
                s_ATBufferCur++;

            p_eol = findNextEOL(s_ATBufferCur);
            p_read += count;
        } else if (count <= 0) {
            /* read error encountered or EOF reached */
            if(count == 0) {
                printf("atchannel: EOF reached\n");
            } else {
                printf("atchannel: read error %s\n", strerror(errno));
            }
            return NULL;
        }
    }

    /* a full line in the buffer. Place a \0 over the \r and return */

    ret = s_ATBufferCur;
    *p_eol = '\0';
    s_ATBufferCur = p_eol + 1; /* this will always be <= p_read,    */
                              /* and there will be a \0 at *p_read */

    printf("AT< %s\n", ret);
    return ret;
}



static void *readerLoop(void *arg)
{
    for (;;) {
	readline();
        // sleep(1);
        // printf("attest:readerLoop\n");
    }
    return NULL;
}

static int writeline (const char *s)
{
    size_t cur = 0;
    size_t len = strlen(s);
    ssize_t written;

    if (s_fd < 0 ) {
        printf("AT_ERROR_CHANNEL_CLOSED, s_fd:%d\n\n\n",s_fd);
        return -1;
    }

    printf("writeline> %s\n\n", s);

    /* the main string */
    while (cur < len) {
        do {
            written = write (s_fd, s + cur, len - cur);
        } while (written < 0 && errno == EINTR);

        if (written < 0) {
            return -1;
        }

        cur += written;
    }

    /* the \r  */

    do {
        written = write (s_fd, "\r" , 1);
    } while ((written < 0 && errno == EINTR) || (written == 0));

    if (written < 0) {
        return -1;
    }

    return 0;
}


int main(int argc, char* argv[])
{
	int fd;
	int ret;
	pthread_t tid;
	pthread_attr_t attr;

	fd = open ("/dev/appvcom", O_RDWR);
	if ( fd >= 0 ) {
		/* disable echo on serial ports */
		struct termios  ios;
		tcgetattr( fd, &ios );
		ios.c_lflag = 0;  /* disable ECHO, ICANON, etc... */
		tcsetattr( fd, TCSANOW, &ios );
		s_fd = fd;
	}

	if (fd < 0) {
		perror ("opening AT interface. retrying...");
		return -1;
	}

	// pthread_attr_init (&attr);
	// pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	// ret = pthread_create(&s_tid_reader, &attr, readerLoop, &attr);
	// if (ret < 0) {
	// 	perror ("pthread_create");
	// 	return -1;
	// }
	// while(1){
		writeline("AT^SETPORT?");
	// 	sleep(1);
	// }
	return 0;
}
