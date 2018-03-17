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
  printf("HBInjector v0.3\n\n");
	printf("This will replace a system application with VitaShell\n");
	printf("Backups will be stored in ux0:data/HBInjector\n");
	printf("Icon layout will be reset\n\n");
	printf("Press X to continue\n");
	printf("Press any other key to exit\n\n");

  char *titleid = (char *) malloc(100);
  char *title = (char *) malloc(100);
  char vara[255];
  char varb[255];
  char varc[255];
  char vard[255];
  char vare[255];
  char varf[255];
  char varg[255];

  switch (get_key()) {
    case SCE_CTRL_CROSS:
      one:
        psvDebugScreenClear( COLOR_BLACK );
        printf("HBInjector v1.0\n\n");
        printf("Press X to replace near with VitaShell\n");
        printf("Press O to replace Parental Controls with VitaShell\n");
        printf("Press /\\ to replace Party with VitaShell\n");
        printf("Press [] to replace Calendar with VitaShell\n");
        printf("Press R to enter Restore Mode\n");
        printf("Press any other key to exit\n\n");
        switch (get_key()) {
        	case SCE_CTRL_CROSS: {
            strcpy(titleid, "NPXS10000");
            strcpy(title, "near");
            goto inject;
          }
        	case SCE_CTRL_CIRCLE: {
            strcpy(titleid, "NPXS10094");
            strcpy(title, "Parental Controls");
            goto inject;
          }
        	case SCE_CTRL_TRIANGLE: {
            strcpy(titleid, "NPXS10001");
            strcpy(title, "Party");
            goto inject;
          }
          case SCE_CTRL_SQUARE: {
            strcpy(titleid, "NPXS10091");
            strcpy(title, "Calendar");
            goto inject;
          }
          case SCE_CTRL_RTRIGGER: {
            goto two;
          }
          default:
            scePowerRequestColdReset();
      	}

        inject:
          snprintf(vara, sizeof(vara), "ux0:data/HBInjector/%s", titleid);
          snprintf(varb, sizeof(varb), "ux0:/data/HBInjector/%s/eboot.bin", titleid);
          snprintf(varc, sizeof(varc), "vs0:app/%s/eboot.bin", titleid);
          snprintf(vard, sizeof(vard), "Failed to backup %s\n", title);
          snprintf(vare, sizeof(vare), "Backing up %s...\n\n", title);
          snprintf(varf, sizeof(varf), "Installing VitaShell to %s\nPress any key to continue\n\n", title);
          snprintf(varg, sizeof(varg), "If it is not, delete ux0:data/HBInjector/%s/eboot.bin\n", titleid);
          printf(varf);
          get_key();

          vshIoUmount(0x300, 0, 0, 0);
          _vshIoMount(0x300, 0, 2, malloc(0x100));

          sceIoMkdir("ux0:data/HBInjector", 0777);
          sceIoMkdir(vara, 0777);  /* ux0:data/HBInjector/title */
          sceIoMkdir("ux0:data/HBInjector/appdb", 0777);
          SceUID fd;
          if(access(varb, F_OK) != -1) {
              printf("VitaShell already installed\n");
              printf(varg);
              printf("Press any key to exit");
              get_key();
              scePowerRequestColdReset();
          } else {
            if (cp(varb, varc) != 0) { /* ux0:/data/HBInjector/title/eboot.bin, vs0:app/title/eboot.bin */
              printf(vard);  /* Failed to backup title */
              printf("Rebooting in 3 seconds...");
              sceKernelDelayThread(3*1000000);
              scePowerRequestColdReset();
            }
            else {
              printf(vare); /* Backing up title... */
            }
          }
          fd = sceIoOpen("app0:VitaShell.bin", SCE_O_RDONLY, 0777);
          if (fd >= 0) {
            printf("Copying VitaShell to System...\n");
            sceIoRemove(varc); /* vs0:app/title/eboot.bin */
            if (cp(varc, "app0:VitaShell.bin") >= 0) { /* vs0:app/title/eboot.bin */
              printf("Copied VitaShell to System\n\n");
            } else {
              printf("Failed to copy VitaShell to system\n");
              printf("Rebooting in 3 seconds...");
              sceKernelDelayThread(3*1000000);
              scePowerRequestColdReset();
            }
          } else {
            printf("ERROR: VitaShell not found!\n");
            printf("Rebooting in 3 seconds...");
            sceKernelDelayThread(3*1000000);
            scePowerRequestColdReset();
          }

          printf("Rebuilding database...\n\n");
          sceIoRemove("ux0:data/HBInjector/appdb/app.db.bak");
          cp("ux0:data/HBInjector/appdb/app.db.bak", "ur0:shell/db/app.db");
          sceIoRemove("ur0:shell/db/app.db");

          printf("Press any key to reboot\n\n");

          switch (get_key()) {
            default: {
              scePowerRequestColdReset();
            }
          }

      two:
        psvDebugScreenClear( COLOR_BLACK );
        printf("HBInjector v1.0\n\n");
        printf("Press X to restore near\n");
        printf("Press O to restore Parental Controls\n");
        printf("Press /\\ to restore Party\n");
        printf("Press [] to restore Calendar\n");
        printf("Press L to enter Inject Mode\n");
        printf("Press any other key to exit\n\n");
        switch (get_key()) {
        	case SCE_CTRL_CROSS: {
            strcpy(titleid, "NPXS10000");
            strcpy(title, "near");
            goto restore;
          }
        	case SCE_CTRL_CIRCLE: {
            strcpy(titleid, "NPXS10094");
            strcpy(title, "Parental Controls");
            goto restore;
          }
        	case SCE_CTRL_TRIANGLE: {
            strcpy(titleid, "NPXS10001");
            strcpy(title, "Party");
            goto restore;
          }
          case SCE_CTRL_SQUARE: {
            strcpy(titleid, "NPXS10091");
            strcpy(title, "Calendar");
            goto restore;
          }
          case SCE_CTRL_LTRIGGER: {
            goto one;
          }
          default: {
            scePowerRequestColdReset();
          }
      	}

        restore:
          snprintf(vara, sizeof(vara), "ux0:/data/HBInjector/%s/eboot.bin", titleid);
          snprintf(varb, sizeof(varb), "vs0:app/%s/eboot.bin", titleid);
          snprintf(varc, sizeof(varc), "Restoring %s to system...\n", title);
          snprintf(vard, sizeof(vard), "Restored %s to system\n\n", title);
          snprintf(vare, sizeof(vare), "Failed to restore %s to system\n", title);
          snprintf(varf, sizeof(varf), "ERROR: %s backup not found!\n", title);
          snprintf(varg, sizeof(varg), "Restoring %s to system\nPress any key to continue\n\n", title);
          printf(varg);
          get_key();

          vshIoUmount(0x300, 0, 0, 0);
          _vshIoMount(0x300, 0, 2, malloc(0x100));

          sceIoMkdir("ux0:data/HBInjector/appdb", 0777);
          fd = sceIoOpen(vara, SCE_O_RDONLY, 0777); /* ux0:/data/HBInjector/title/eboot.bin */
          if (fd >= 0) {
            printf(varc); /* Restoring title to system... */
            sceIoRemove(vara); /* vs0:app/title/eboot.bin */
            if (cp(varb, vara) >= 0) { /* vs0:app/title/eboot.bin */ /* ux0:/data/HBInjector/title/eboot.bin */
              printf(vard); /* Restored title to system */
            } else {
              printf(vare); /* Failed to restore title to system */
              printf("Rebooting in 3 seconds...");
              vshIoUmount(0x300, 0, 0, 0);
              sceKernelDelayThread(3*1000000);
              scePowerRequestColdReset();
            }
          } else {
            printf(varf); /* ERROR: title backup not found! */
            printf("Rebooting in 3 seconds...");
            vshIoUmount(0x300, 0, 0, 0);
            scePowerRequestColdReset();
          }

          printf("Rebuilding database...\n\n");
          sceIoRemove("ux0:data/HBInjector/appdb/app.db.bak");
          cp("ux0:data/HBInjector/appdb/app.db.bak", "ur0:shell/db/app.db");
          sceIoRemove("ur0:shell/db/app.db");
          sceIoRemove(vara);

          printf("Press any key to reboot\n\n");

          switch (get_key()) {
            case SCE_CTRL_CROSS: {
              scePowerRequestColdReset();
            }
            default: {
              scePowerRequestColdReset();
            }
          }
    default: {
      scePowerRequestColdReset();
    }
  }
}
