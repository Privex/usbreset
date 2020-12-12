# Linux USB Device Reset Application

This application can be used to reset USB devices on Linux, saving you from having to unplug and re-plug the device back in.

This is a fork of the original version, uploaded by @jkulesza https://github.com/jkulesza/usbreset - which itself was originally
written by Alan Stern. 

We've (Privex Inc.) made various improvements on the original version, such as:

 - Adding detailed help and usage information displayed when you run the program without arguments
 - Improved the error messages to be more descriptive, to help users understand why it went wrong.
 - A completely new feature (part of the help/usage) which loops over the available USB devices and 
   outputs their paths, allowing a user to easily copy paste the direct device path.

This tool is especially useful in scenarios where you don't have physical access to the system, for example, a co-located server.

If the device is still recognised by Linux (shows up in `lsusb`), but is malfunctioning, then you can often reset just that
individual device.

If the device isn't recognised by Linux due to severe malfunctioning, and doesn't show up in `lsusb`, you can still reset
the device by resetting the USB port / hub that the device is connected to, which will force Linux to re-detect the device
and try to bring it online again.


## Quickstart

If you'd rather skip the detailed step-by-step example, simply follow these quickstart commands :)

It's best to run these under the `root` user, or if you prefer, you can simply use `sudo` when running `./usbreset` .

```sh
# Use lsusb and/or dmesg to find the appropriate device + bus you need to reset
lsusb

# Download usbreset.c
wget https://raw.githubusercontent.com/Privex/usbreset/master/usbreset.c

# Compile usbreset.c using either 'cc' or 'gcc'
cc usbreset.c -o usbreset

# Assuming the problematic device is Bus 003 Device 002
# you'd pass the following device path
./usbreset /dev/bus/usb/003/002

# You can also run usbreset with no arguments to see help / usage information,
# plus a list of available USB device paths that can be used.
# If you can't reset the device itself (e.g. because the device isn't recognised),
# you can reset the port / hub device it's connected to instead.
./usbreset
```

## License

This fork by Privex, is released under the MIT / X11 open source License.

## Step-by-step example

This README describes the steps to compile, run, and then automate execution of this application to reset a USB wireless adapter with a greater degree of authority than is available using `ifdown wlan0`/`ifup wlan0`.

Credit to Alan Stern for developing this program, as retrieved from: http://marc.info/?l=linux-usb&m=121459435621262&w=2

The following sequence is adequate to execute the program:


1. Compile the program:


        cc usbreset.c -o usbreset


2. Set the permissions on the executable:


        chmod +x usbreset


3. Get the Bus and Device ID of the USB device you want to reset:

   
        lsusb


   which provides output:

   
        Bus 001 Device 007: ID 050d:945a Belkin Components F7D1101 v1 Basic Wireless Adapter [Realtek RTL8188SU]
        Bus 001 Device 003: ID 0403:6014 Future Technology Devices International, Ltd FT232H Single HS USB-UART/FIFO IC
        Bus 001 Device 002: ID 0bc2:5031 Seagate RSS LLC FreeAgent GoFlex USB 3.0
        Bus 001 Device 001: ID 1d6b:0002 Linux Foundation 2.0 root hub
        Bus 002 Device 002: ID 046d:c06b Logitech, Inc. G700 Wireless Gaming Mouse
        Bus 002 Device 003: ID 413c:2106 Dell Computer Corp. Dell QuietKey Keyboard
        Bus 002 Device 001: ID 1d6b:0001 Linux Foundation 1.1 root hub


   where the device of interest is on Bus 001 identified as Device 007 (our USB wireless adapter).


4. Execute the program with sudo privilege or as the root user, making necessary substitution for with the `Bus` and `Device` ids as found by running the lsusb command:

       sudo ./usbreset /dev/bus/usb/Bus/Device

   or in our case

       sudo ./usbreset /dev/bus/usb/001/007

   Now, we can automate execution with the following inline Perl command (assuming that `usbreset` is in the $PATH):

5. Execute or embed in a script:

        echo $(lsusb | grep "Wireless Adapter");  wifipath=$( lsusb | grep "Wireless Adapter" | perl -nE "/\D+(\d+)\D+(\d+).+/; print qq(\$1/\$2)")  sudo usbreset /dev/bus/usb/$wifipath

    which was inspired by the code provided by *knb* at http://askubuntu.com/questions/645/how-do-you-reset-a-usb-device-from-the-command-line
