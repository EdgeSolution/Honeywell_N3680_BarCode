
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/times.h>
#include <time.h>
#include <errno.h>

#ifndef	CLOCK_MONOTONIC
#error "Need monotonic clock"
#endif

unsigned long GetTickCount()
{
 	struct timespec ts;
 	clock_gettime(CLOCK_MONOTONIC, &ts); // or CLOCK_PROCESS_CPUTIME_ID
 	unsigned int ret(ts.tv_sec);
 	return ret * 1000 + ts.tv_nsec / 1000000;
}

unsigned long long GetTickCount64()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts); // or CLOCK_PROCESS_CPUTIME_ID
	unsigned int ret(ts.tv_sec);
	return ret * 1000 + ts.tv_nsec / 1000000;
}

void Sleep(long mSec)
{
	usleep(mSec*1000);
}

/*
Use this in case the usleep is not available
void Sleep(long mSec)
{
	timespec request;
	timespec remaining;
	request.tv_sec = mSec/1000;
	request.tv_nsec = (mSec % 1000) *1000*1000;
	do
	{
		remaining.tv_sec = 0;
		remaining.tv_nsec = 0;
		if(nanosleep(&request, &remaining) < 0)
		{
			if(errno == EINTR)
			{
				request=remaining;
			}
			else
			{
				break;
			}
		}
	}
	while(remaining.tv_nsec+remaining.tv_sec != 0);
}
*/

