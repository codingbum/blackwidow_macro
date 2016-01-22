/*
 * Razer BlackWidow Macro Keys Enabler
 * Copyright (C) 2016 B.Walden
 *
 * This program sends a init packet used by Razer proprietary drivers
 * to initialize the 'Macro' keys on Razer BlackWidow keyboards.
 *
 * Note: BlackWidow Keyboards do not have any onboard memory for macros
 *       Razer use software for macro functionality in windows
 *       You can however use 'xmacro' in linux
 *       You can also use xmodmap or your WM to allocate functions to the
 *       macro key keycodes.
 *
 * M Key Keycodes: M1 = 191, M2 = 192, M3 = 193, M4 = 194, M5 = 195
 *
 * License: MIT (LICENSE file should be included with source)
 */

#include <libusb-1.0/libusb.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Device IDs
#define DEV_VID 0x1532      //Razer
#define DEV_PID 0x011b      //Blackwidow 2013

// USB Device
#define DEV_INTF 2

// Version
#define VERSION 1

// Blackwidow M Key Init Code
unsigned char Blackwidow_Init[90] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04,
    0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x04, 0x00
};

libusb_device_handle * handle;
int verbose = 0;
int send_init = 0;

// Initialize libusb
int init () {
    libusb_init(NULL);
    libusb_set_debug(NULL, 3);

    // Open device based on vender & pid
    handle = libusb_open_device_with_vid_pid(NULL, DEV_VID, DEV_PID);
    if (handle == NULL) {
        printf("ERROR - Device Not Found");
    } else {
        printf("Razer BlackWidow Device Found\n");
    }

    // Detaching libusb kernel driver
    if (libusb_kernel_driver_active(handle,DEV_INTF))
        libusb_detach_kernel_driver(handle, DEV_INTF);
    
    // Give feedback & quit on error
    if (libusb_claim_interface(handle, DEV_INTF) < 0) {
        printf("IO Error Opening USB Device\n");
        libusb_close(handle);
        libusb_exit(NULL);
        return 1;
    }
    
    return 0;
}

void showBanner () {
    printf("Usage: ./bwidow -s -v \n");
}

void closeHandle () {
    libusb_release_interface(handle, DEV_INTF);
    libusb_close(handle);
    libusb_exit(NULL);
}

void printData (unsigned char * data) {
    int k = 0;
    for (int i = 0; i < 90; i++) {
        printf(" %02x", data[i]);
        if (k++ == 7) {
            printf("\n");
            k = 0;
        }
    }
    printf("\n");
}

// Send data to USB
int sendcmd (unsigned char * data) {
    if (verbose) {
        printf("Sending data:\n");
        printData(data);
    }

    int res;
    res = libusb_control_transfer(handle,
        LIBUSB_REQUEST_TYPE_CLASS,
        0x9, 0x300, 0, data, 90, 1000);

    if (verbose) {
        printf("Transmitted: %d\n", res);
    }
    return res;
}

// Console argument parsing
int scanArgs (char* arg, char* argv[], int argc) {
    int i = 0;
    for (i = 1; i < argc; i++) {
        if (strcmp(arg, argv[i]) == 0) {
            return i;
        }
    }
    return 0;
}

// Main
int main (int argc, char * argv[]) {
    printf("Razer BlackWidow Macro Keys v%d\n", VERSION);

    if ( argc<2 )
    {
        // Show Help Banner If No Args Given
        printf("\nUsage: %s <arg>\n",argv[0]);
        printf("\nArguments:\n \t-s\tsend init packet\n");
        printf("\t-v\tshow verbose output\n");
        printf("\nNote: This script requires root access for kernel driver\n");
        return 0;
    }
    else
    {

        if (init() != 0) return 1;

        // Verbose mode
        if (scanArgs("-v", argv, argc)) {
            verbose = 1;
        }

        // Send Button Init Command
        if (scanArgs("-s", argv, argc)){
            send_init = 1;
        }

        if (send_init) {
        //Send Data
            sendcmd(Blackwidow_Init);
        }

        closeHandle();
    }
    return 0;
}