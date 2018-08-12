#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_
//---------------------------------------------------------------------------
#include <stdint.h>
#include <linux/watchdog.h>   // /usr/local/arm/cross/am335xt3/devkit/arm-arago-linux-gnueabi/usr/include

class WatchDog
{
public:
    WatchDog(){
        fd = NULL;
    };
    ~WatchDog(){};

    /*! \brief Feed watchdog
     Sends an IOCTL to the driver, which in turn ticks the PC Watchdog card 
     to reset its internal timer so it doesn't trigger a computer reset.
    */
    void Feed() {
        if (!fd) return;
        int dummy;
        ioctl(fd, WDIOC_KEEPALIVE, &dummy);
    };
    void SetTimeout(int secs) { if(!fd) return; ioctl(fd, WDIOC_SETTIMEOUT, &secs); };
    int Enable() {
#if 1
        Open();
#else   //AM335x driver not support below
        OnOff(WDIOS_DISABLECARD);
	    fprintf(stderr, "Watchdog card enabled.\n");
	    fflush(stderr);
#endif
    };
    int Disable() {
#if 1
        Close();
#else   //AM335x driver not support below
        OnOff(WDIOS_ENABLECARD);
	    fprintf(stderr, "Watchdog card disabled.\n");
	    fflush(stderr);
#endif
	};
    
protected:
private:
	int fd;
	void Open() {
        if (fd) return;
        fd = open("/dev/watchdog", O_WRONLY);

        if (fd == -1) {
        	fprintf(stderr, "Watchdog device not enabled.\n");
        	fflush(stderr);
        	fd = NULL;
        }
	};
	void Close() {
        if (!fd) {
            close(fd);
            fd = NULL;
        }
	};
	void OnOff(int flag) {
	    Open();
	    ioctl(fd, WDIOC_SETOPTIONS, &flag);
	};
};

inline WatchDog &watchdog()
{
	static WatchDog wtdog;
	return wtdog;
};



#endif //_WATCHDOG_H_
