#include <psp2/display.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>
#include <psp2/io/stat.h>
#include <psp2/ctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "graphics.h"
#define printf psvDebugScreenPrintf

int cp(const char *to, const char *from)
{
    SceUID fd_to, fd_from;
    char buf[16*1024];
    ssize_t nread;
    int saved_errno;
	//
    fd_from = sceIoOpen(from, SCE_O_RDONLY, 0777);
    if (fd_from < 0)
        return -1;

    fd_to = sceIoOpen(to, SCE_O_WRONLY|SCE_O_CREAT, 0777);
    if (fd_to < 0)
        goto out_error;

    while (nread = sceIoRead(fd_from, buf, sizeof buf), nread > 0)
    {
        char *out_ptr = buf;
        ssize_t nwritten;

        do {
            nwritten = sceIoWrite(fd_to, out_ptr, nread);

            if (nwritten >= 0)
            {
                nread -= nwritten;
                out_ptr += nwritten;
            }
            else if (errno != EINTR)
            {
                goto out_error;
            }
        } while (nread > 0);
    }

    if (nread == 0)
    {
        if (sceIoClose(fd_to) < 0)
        {
            fd_to = -1;
            goto out_error;
        }
        sceIoClose(fd_from);

        return 0;
    }

  out_error:
    saved_errno = errno;

    sceIoClose(fd_from);
    if (fd_to >= 0)
        sceIoClose(fd_to);

    errno = saved_errno;
    return -1;
}

static unsigned buttons[] = {
	SCE_CTRL_SELECT,
	SCE_CTRL_START,
	SCE_CTRL_UP,
	SCE_CTRL_RIGHT,
	SCE_CTRL_DOWN,
	SCE_CTRL_LEFT,
	SCE_CTRL_LTRIGGER,
	SCE_CTRL_RTRIGGER,
	SCE_CTRL_TRIANGLE,
	SCE_CTRL_CIRCLE,
	SCE_CTRL_CROSS,
	SCE_CTRL_SQUARE,
};

int get_key(void) {
	static unsigned prev = 0;
	SceCtrlData pad;
	while (1) {
		memset(&pad, 0, sizeof(pad));
		sceCtrlPeekBufferPositive(0, &pad, 1);
		unsigned new = prev ^ (pad.buttons & prev);
		prev = pad.buttons;
		for (int i = 0; i < sizeof(buttons)/sizeof(*buttons); ++i)
			if (new & buttons[i])
				return buttons[i];

		sceKernelDelayThread(1000); // 1ms
	}
}





int main(int argc, char *argv[]) {
	psvDebugScreenInit();
  printf("HBInjector v0.2\n\n");
	printf("This will replace a system application with VitaShell\n");
	printf("Backups will be stored in ux0:data/HBInjector\n");
	printf("Icon layout will be reset\n\n");
	printf("Press X to continue\n");
	printf("Press any other key to exit\n\n");
	switch (get_key()) {
	case SCE_CTRL_CROSS:
    printf("Press X to replace near with VitaShell\n");
    printf("Press O to replace Parental Controls with VitaShell\n");
    printf("Press /\\ to replace Party with VitaShell\n");
    printf("Press [] to replace Calendar with VitaShell\n");
    printf("Press any other key to exit\n\n");
    switch (get_key()) {
  	case SCE_CTRL_CROSS:
  		printf("Mounting vs0 as RW\n");
  		vshIoUmount(0x300, 0, 0, 0);
  		_vshIoMount(0x300, 0, 2, malloc(0x100));

  		sceIoMkdir("ux0:data/HBInjector", 0777);
  		sceIoMkdir("ux0:data/HBInjector/NPXS10000", 0777);
  		sceIoMkdir("ux0:data/HBInjector/appdb", 0777);
  		if (cp("ux0:/data/HBInjector/NPXS10000/eboot.bin", "vs0:app/NPXS10000/eboot.bin") != 0)
  			printf("Failed to backup NPXS10000\n");
  		else
  			printf("Backing up NPXS10000...\n");

  		SceUID fd;
  		fd = sceIoOpen("app0:VitaShell.bin", SCE_O_RDONLY, 0777);
  		if (fd >= 0)
  		{
  			printf("Using app0:VitaShell.bin\n");
  			sceIoRemove("vs0:app/NPXS10000/eboot.bin");
  			if (cp("vs0:app/NPXS10000/eboot.bin", "app0:VitaShell.bin") >= 0)
  				printf("Copied VitaShell to System\n");
  			else
  				printf("Failed to copy VitaShell to system\n");
  		}
  		else
  		{
  			printf("ERROR: VitaShell not found!\n");
  		}

  		printf("Rebuilding database...\n\n");
  		sceIoRemove("ux0:data/HBInjector/appdb/app.db.bak");
  		cp("ux0:data/HBInjector/appdb/app.db.bak", "ur0:shell/db/app.db");
  		sceIoRemove("ur0:shell/db/app.db");

    	printf("Press X to reboot\n");
    	printf("Press any other key to exit\n\n");
    	switch (get_key()) {
    	case SCE_CTRL_CROSS:
    		scePowerRequestColdReset();
    	default:
    		sceKernelExitProcess(0);
    	}
  	case SCE_CTRL_CIRCLE:
  		printf("Mounting vs0 as RW\n");
  		vshIoUmount(0x300, 0, 0, 0);
  		_vshIoMount(0x300, 0, 2, malloc(0x100));

  		sceIoMkdir("ux0:data/HBInjector", 0777);
  		sceIoMkdir("ux0:data/HBInjector/NPXS10094", 0777);
  		sceIoMkdir("ux0:data/HBInjector/appdb", 0777);
  		if (cp("ux0:/data/HBInjector/NPXS10094/eboot.bin", "vs0:app/NPXS10094/eboot.bin") != 0)
  			printf("Failed to backup NPXS10094\n");
  		else
  			printf("Backing up NPXS10094...\n");

  		fd = sceIoOpen("app0:VitaShell.bin", SCE_O_RDONLY, 0777);
  		if (fd >= 0)
  		{
  			printf("Using app0:VitaShell.bin\n");
  			sceIoRemove("vs0:app/NPXS10094/eboot.bin");
  			if (cp("vs0:app/NPXS10094/eboot.bin", "app0:VitaShell.bin") >= 0)
  				printf("Copied VitaShell to System\n");
  			else
  				printf("Failed to copy VitaShell to system\n");
  		}
  		else
  		{
  			printf("ERROR: VitaShell not found!\n");
  		}

  		printf("Rebuilding database...\n\n");
  		sceIoRemove("ux0:data/HBInjector/appdb/app.db.bak");
  		cp("ux0:data/HBInjector/appdb/app.db.bak", "ur0:shell/db/app.db");
  		sceIoRemove("ur0:shell/db/app.db");

    	printf("Press X to reboot\n");
    	printf("Press any other key to exit\n\n");
    	switch (get_key()) {
    	case SCE_CTRL_CROSS:
    		scePowerRequestColdReset();
    	default:
    		sceKernelExitProcess(0);
    	}
  	case SCE_CTRL_TRIANGLE:
  		printf("Mounting vs0 as RW\n");
  		vshIoUmount(0x300, 0, 0, 0);
  		_vshIoMount(0x300, 0, 2, malloc(0x100));

  		sceIoMkdir("ux0:data/HBInjector", 0777);
  		sceIoMkdir("ux0:data/HBInjector/NPXS10001", 0777);
  		sceIoMkdir("ux0:data/HBInjector/appdb", 0777);
  		if (cp("ux0:/data/HBInjector/NPXS10001/eboot.bin", "vs0:app/NPXS10001/eboot.bin") != 0)
  			printf("Failed to backup NPXS10001\n");
  		else
  			printf("Backing up NPXS10001...\n");

  		fd = sceIoOpen("app0:VitaShell.bin", SCE_O_RDONLY, 0777);
  		if (fd >= 0)
  		{
  			printf("Using app0:VitaShell.bin\n");
  			sceIoRemove("vs0:app/NPXS10001/eboot.bin");
  			if (cp("vs0:app/NPXS10001/eboot.bin", "app0:VitaShell.bin") >= 0)
  				printf("Copied VitaShell to System\n");
  			else
  				printf("Failed to copy VitaShell to system\n");
  		}
  		else
  		{
  			printf("ERROR: VitaShell not found!\n");
  		}

  		printf("Rebuilding database...\n\n");
  		sceIoRemove("ux0:data/HBInjector/appdb/app.db.bak");
  		cp("ux0:data/HBInjector/appdb/app.db.bak", "ur0:shell/db/app.db");
  		sceIoRemove("ur0:shell/db/app.db");

    	printf("Press X to reboot\n");
    	printf("Press any other key to exit\n\n");
    	switch (get_key()) {
    	case SCE_CTRL_CROSS:
    		scePowerRequestColdReset();
    	default:
    		sceKernelExitProcess(0);
    	}
    case SCE_CTRL_SQUARE:
  		printf("Mounting vs0 as RW\n");
  		vshIoUmount(0x300, 0, 0, 0);
  		_vshIoMount(0x300, 0, 2, malloc(0x100));

  		sceIoMkdir("ux0:data/HBInjector", 0777);
  		sceIoMkdir("ux0:data/HBInjector/NPXS10091", 0777);
  		sceIoMkdir("ux0:data/HBInjector/appdb", 0777);
  		if (cp("ux0:/data/HBInjector/NPXS10091/eboot.bin", "vs0:app/NPXS10091/eboot.bin") != 0)
  			printf("Failed to backup NPXS10091\n");
  		else
  			printf("Backing up NPXS10091...\n");

  		fd = sceIoOpen("app0:VitaShell.bin", SCE_O_RDONLY, 0777);
  		if (fd >= 0)
  		{
  			printf("Using app0:VitaShell.bin\n");
  			sceIoRemove("vs0:app/NPXS10091/eboot.bin");
  			if (cp("vs0:app/NPXS10091/eboot.bin", "app0:VitaShell.bin") >= 0)
  				printf("Copied VitaShell to System\n");
  			else
  				printf("Failed to copy VitaShell to system\n");
  		}
  		else
  		{
  			printf("ERROR: VitaShell not found!\n");
  		}

  		printf("Rebuilding database...\n\n");
  		sceIoRemove("ux0:data/HBInjector/appdb/app.db.bak");
  		cp("ux0:data/HBInjector/appdb/app.db.bak", "ur0:shell/db/app.db");
  		sceIoRemove("ur0:shell/db/app.db");

    	printf("Press X to reboot\n");
    	printf("Press any other key to exit\n\n");
    	switch (get_key()) {
    	case SCE_CTRL_CROSS:
    		scePowerRequestColdReset();
    	default:
    		sceKernelExitProcess(0);
    	}
    default:
  		sceKernelExitProcess(0);
  	}
	default:
		sceKernelExitProcess(0);
	}
}
