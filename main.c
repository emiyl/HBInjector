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
#define WRAPVAL(v, min, max) (v < min ? max : v > max ? min : v)

int cp(const char *to, const char *from) {
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

static const char* sysTitles[][2] = {
    {"NPXS10000", "near"},
    {"NPXS10094", "Parental Controls"},
    {"NPXS10012", "PS3 Remote Play"},
    {"NPXS10004", "Photos"},
    {"NPXS10006", "Friends"},
    {"NPXS10008", "Trophies"},
    {"NPXS10009", "Music"},
    {"NPXS10010", "Video"},
    {"NPXS10013", "PS4 Link"},
    {"NPXS10014", "Messages"},
    {"NPXS10072", "Mail"},
    {"NPXS10095", "Panoramic Camera"},
    {"NPXS10091", "Calendar"},
};

#define TITLEID(n) sysTitles[n][0]
#define TITLENAME(n) sysTitles[n][1]
#define TITLEMAX sizeof(sysTitles) / sizeof(sysTitles[0]) - 1

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

		sceKernelDelayThread(1000); // 1s
	}
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int main(int argc, char *argv[]) {
	psvDebugScreenInit();
  const char *version = ("1.2.5") ;
  char *titleid = (char *) malloc(100);
  char *title = (char *) malloc(100);
  char *selecttitlename = (char *) malloc(100);
  char *modename = (char *) malloc(100);
  int nTitle = 0;
  int mode = 0;
  char header[255];
  char backupDir[255], backupPath[255], sysappPath[255], flagPath[255];
  sceIoMkdir("ux0:data/HBInjector", 0777);
  sceIoMkdir("ux0:data/HBInjector/flags", 0777);

  psvDebugScreenClear( COLOR_BLACK );
  snprintf(header, sizeof(header), "\n HBInjector v%s\n -----------------\n\n", version);
  printf("%s%s",
    header,
    " This will replace a system application with VitaShell\n"
    " Backups will be stored in ux0:data/HBInjector\n"
    " Icon layout will be reset\n"
    "\n"
    " The system application selected will not be able to be\n"
    " used for its original purpose until you restore it\n"
    "\n"
    " If you are looking to install this before updating Enso on\n"
    " 3.65, remember that when updating all system apps are\n"
    " reset to stock, including HBInjected Applications\n"
    "\n"
    " Press X to continue\n"
    " Press O to exit\n"
  );

  while(1) switch (get_key()) {
    case SCE_CTRL_CROSS:
      one:
        psvDebugScreenClear( COLOR_BLACK );

        strcpy(selecttitlename, TITLENAME(nTitle));
        strcpy(titleid, TITLEID(nTitle));
        strcpy(title, selecttitlename);
        strcpy(modename, mode == 0 ? "Inject" : "Restore");

        printf(
            " %s"
            " Select Title:\n\n"
            " < %s >\n\n"
            " Use the D-Pad to select a title\n"
            " Use the L and R buttons to change the mode\n"
            "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
            " Mode: %s",
            header, selecttitlename, modename
        );

        while(1) switch (get_key()) {
            case SCE_CTRL_CROSS:
                if (mode == 0)
                    goto inject;
                else
                    goto restore;
                break;

            case SCE_CTRL_RIGHT:
                nTitle = WRAPVAL(nTitle+1, 0, TITLEMAX);
                goto one;

            case SCE_CTRL_LEFT:
                nTitle = WRAPVAL(nTitle-1, 0, TITLEMAX);
                goto one;

            case SCE_CTRL_RTRIGGER:
            case SCE_CTRL_LTRIGGER:
                mode = !mode;
                goto one;
      	}

        inject:
          psvDebugScreenClear( COLOR_BLACK );

          printf(
            "%s"
            " Installing VitaShell to %s\n"
            " Press X to continue\n"
            " Press O to go cancel\n\n",
            header, title
          );

          while (1) {
            switch(get_key()) {
              case SCE_CTRL_CROSS:
                break;
              case SCE_CTRL_CIRCLE:
                goto one;
              default:
                continue;
            }
            break;
          }

          snprintf(backupDir, sizeof(backupDir), "ux0:data/HBInjector/%s", titleid);
          snprintf(backupPath, sizeof(backupPath), "%s/eboot.bin", backupDir);
          snprintf(sysappPath, sizeof(sysappPath), "vs0:app/%s/eboot.bin", titleid);
          snprintf(flagPath, sizeof(flagPath), "ux0:data/HBInjector/flags/%s.flg", titleid);

          sceIoMkdir(backupDir, 0777);
          sceIoMkdir("ux0:data/HBInjector/appdb", 0777);
          SceUID fd;
          if(access(flagPath, F_OK) != -1) {
              printf(
                " VitaShell is already installed\n"
                " If not, delete %s\n"
                " Press any key to cancel",
                flagPath
              );
              get_key();
              goto one;
          } else {
            if (cp(backupPath, sysappPath) != 0) {
              printf(
                " Failed to backup %s\n"
                " Title likely doesn't exist\n"
                " Press any key to cancel",
                title
              );
              get_key();
              goto one;
            }
            else {
              vshIoUmount(0x300, 0, 0, 0);
              _vshIoMount(0x300, 0, 2, malloc(0x100));
              printf(" Backing up %s...\n\n", title);
            }
          }
          fd = sceIoOpen("app0:VitaShell.bin", SCE_O_RDONLY, 0777);
          if (fd >= 0) {
            printf(" Copying VitaShell to System...\n");
            sceIoRemove(sysappPath);
            if (cp(sysappPath, "app0:VitaShell.bin") >= 0) {
              printf(" Copied VitaShell to System\n\n");
            } else {
              printf(
                " Failed to copy VitaShell to system\n"
                " Likely due to vs0 not mounting correctly\n"
                " Press any key to reboot"
              );
              get_key();
              goto end;
            }
          } else {
            printf(
                " ERROR: VitaShell not found!\n"
                " Press any key to reboot"
            );
            get_key();
            goto end;
          }

          printf(" Rebuilding database...\n\n");
          sceIoRemove("ux0:data/HBInjector/appdb/app.db");
          cp("ux0:data/HBInjector/appdb/app.db", "ur0:shell/db/app.db");
          sceIoRemove("ur0:shell/db/app.db");
          WriteFile(flagPath, 0, 1);

          printf(" Press any key to reboot\n\n");
          get_key();
          goto end;

        restore:
          psvDebugScreenClear( COLOR_BLACK );

          printf(
            "%s"
            " Restoring %s to system\n"
            " Press X to continue\n"
            " Press O to go cancel\n\n",
            header, title
          );

          while (1) {
            switch(get_key()) {
              case SCE_CTRL_CROSS:
                break;
              case SCE_CTRL_CIRCLE:
                goto one;
              default:
                continue;
            }
            break;
          }

          snprintf(backupPath, sizeof(backupPath), "ux0:data/HBInjector/%s/eboot.bin", titleid);
          snprintf(sysappPath, sizeof(sysappPath), "vs0:app/%s/eboot.bin", titleid);
          snprintf(flagPath, sizeof(flagPath), "ux0:data/HBInjector/%s.flg", titleid);

          vshIoUmount(0x300, 0, 0, 0);
          _vshIoMount(0x300, 0, 2, malloc(0x100));

          sceIoMkdir("ux0:data/HBInjector/appdb", 0777);
          fd = sceIoOpen(backupPath, SCE_O_RDONLY, 0777);
          if (fd >= 0) {
            printf(" Restoring %s to system...\n", title);
            if (cp(sysappPath, backupPath) >= 0) {
              printf(" Restored %s to system\n\n", title);
            } else {
              printf(
                " Failed to restore %s to system\n"
                " Likely due to vs0 not mounting correctly\n"
                " Press any key to reboot",
                title
              );
              get_key();
              goto end;
            }
          } else {
            printf(
                " ERROR: %s backup not found!\n"
                " Likely due to vs0 not mounting correctly\n"
                " Press any key to reboot",
                title
            );
            get_key();
            goto end;
          }

          printf(" Rebuilding database...\n\n");
          sceIoRemove("ux0:data/HBInjector/appdb/app.db");
          cp("ux0:data/HBInjector/appdb/app.db", "ur0:shell/db/app.db");
          sceIoRemove("ur0:shell/db/app.db");
          sceIoRemove(flagPath);

          printf(" Press any key to reboot\n\n");
          get_key();
          goto end;

    case SCE_CTRL_CIRCLE:
      sceKernelExitProcess(0);
      break;
  }
end:
  scePowerRequestColdReset();
  return 0;
}
