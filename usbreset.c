/*
 * (C) USBReset - Copyright 2020 - Privex Inc. ( https://www.privex.io )
 *     Released under the MIT / X11 License
 *     Based on the original usbreset.c written by Alan Stern
 *     Official repo: https://github.com/Privex/usbreset
 *
 * Usage:
 *
 *   gcc usbreset.c -o usbreset
 *   # To view help / usage info, plus a list of available USB device paths:
 *   ./usbreset
 *   # To reset device 002 on bus 003 (run lsusb to find the device/port you want to reset)
 *   ./usbreset /dev/bus/usb/003/002
 *
 *   # On some systems, you may want to use 'cc' instead of 'gcc'
 *   cc usbreset.c -o usbreset
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <sys/ioctl.h>

#include <linux/usbdevice_fs.h>
#define USBDIR "/dev/bus/usb"
#define APPVER "1.0.0"

int getfcount(char* dpath) {
    DIR *d;
    int totalf = 0;
    struct dirent *dir;
    d = opendir(dpath);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            totalf = totalf + 1;
        }
        closedir(d);
    }
    return totalf;
}

void lfiles(char* dpath, char **result) {
    /*
    Scans the folder 'dpath' for files and folders, and injects them into the 
    character array pointer 'result'


    Example usage:

        // First use getfcount to figure out how many files are in a certain folder
        int totalfiles = getfcount("/home/example");
        // Create a character array pointer using the size returned by getfcount
        char *flist[totalfiles];
        // Pass the folder path to lfiles along with the char array pointer to fill
        lfiles("/home/example", flist);
        // Now you can loop over the files as needed.
        for (int i=0;i < totalfiles;i++) {
            printf("File: %s \n", flist[i]);
        }

    */

    DIR *d;
    int totalf = 0;
    struct dirent *dir;

    // printf("Opening folder: %s \n", dpath);
    d = opendir(dpath);
    if (d) {
        // printf("Folder exists: %s \n", dpath);
        while ((dir = readdir(d)) != NULL) {
            // printf("Adding folder: %s \n", dir->d_name);
            result[totalf] = malloc(strlen(dir->d_name) + 1);
            strcpy(result[totalf], dir->d_name);
            totalf = totalf + 1;
        }
        // printf("Closing: %s \n", dpath);
        closedir(d);
    } else {
        // printf("d is not valid\n");
        return;
    }
}

void print_copyright() {
    const char* copyright = 
        "(C) USBReset Version v%s - Copyright 2020 - Privex Inc. ( https://www.privex.io )\n"
        "    Released under the MIT / X11 License\n"
        "    Based on the original usbreset.c written by Alan Stern\n"
        "    Official repo: https://github.com/Privex/usbreset\n\n";

    fprintf(stderr, copyright, APPVER);
}

void print_descr() {
    const char* descr = 
        "This program resets a given USB device, which can either be an actual connected USB device,\n"
        "or a USB hub / port (as long as they have a USB device file to use).\n"
        "\n"
        "To figure out which bus / port a certain device is connected to, you can either use the\n"
        "utility 'lsusb' (package 'usbutils' on ubuntu/debian), or check 'dmesg' if the device is\n"
        "malfunctioning and not showing up in 'lsusb'.\n"
        "\n"
        "USB device files are generally found under the devfs location /dev/bus/usb - which usually contains\n"
        "both connected devices, and the hubs/ports themselves, laid out as /dev/bus/usb/<bus>/<device> \n\n";
    fprintf(stderr, descr);
}

void print_usage(char* app_exe) {
    const char* usagex = 
        "Usage Example:\n\n"
        "    To reset 'Device 001' on 'Bus 001', you would run %s like so:\n\n"
        "        %s /dev/bus/usb/001/001\n\n"
        "    To reset 'Device 002' on 'Bus 003', you would run %s like so:\n\n"
        "        %s /dev/bus/usb/003/002\n\n";

    fprintf(stderr, usagex, app_exe, app_exe, app_exe, app_exe);
}

void print_usbs() {
    fprintf(stderr, "Available USB devices / buses:\n\n");

    int totalfiles = getfcount(USBDIR);
    int usblen = strlen(USBDIR);
    char *flist[totalfiles];  // Create a char array that can store up to 'totalfiles' filenames
    lfiles(USBDIR, flist);  // Retrieve a list of files into flist
    //int subf_total;

    for (int i=0;i < totalfiles;i++) {
        if (strcmp(flist[i], ".") == 0) continue;
        if (strcmp(flist[i], "..") == 0) continue;
        fprintf(stderr, "    Bus %s:\n\n", flist[i]);
        {
            int sflen = strlen(flist[i]);

            char sfile[sflen + usblen + 1];
            strcpy(sfile, USBDIR);    // first initialise sfile by copying USBDIR into it
            strcat(sfile, "/");
            strcat(sfile, flist[i]);  // append the filename to the end
            // just like before, retrieve a list of all files in this sub-folder
            int sftotal = getfcount(sfile);
            char *sflist[sftotal];
            lfiles(sfile, sflist);
            for (int x=0;x < sftotal;x++) {
                if (strcmp(sflist[x], ".") == 0) continue;
                if (strcmp(sflist[x], "..") == 0) continue;
                fprintf(stderr, "        %s/%s\n", sfile, sflist[x]);
            }
            fprintf(stderr, "\n");
        }
    }
    fprintf(stderr, "\n");
}


void print_help(char* app_exe) {
    fprintf(stderr, "Usage: %s device-filename\n\n", app_exe);
    print_copyright();
    print_descr();
    print_usage(app_exe);
    print_usbs();
}

int main(int argc, char **argv)
{
    const char *filename;
    if (argc != 2) {
        print_help(argv[0]);
        return 1;
    }
    int fd;
    int rc;
    filename = argv[1];
    printf("Opening handle for USB device %s\n", filename);
    fd = open(filename, O_WRONLY);
    if (fd < 0) {
        perror("Error opening device file...");
        return 1;
    }

    printf("Resetting USB device %s\n", filename);
    rc = ioctl(fd, USBDEVFS_RESET, 0);
    if (rc < 0) {
        perror("Error in ioctl (error while sending USBDEVFS_RESET control signal to device file)");
        return 1;
    }
    printf("Successfully reset USB device %s  :)\n", filename);

    close(fd);
    return 0;
}
