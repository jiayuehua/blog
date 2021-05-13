---
layout: post
title: "Making floppy disk images under OS X"
date: 2019-07-26 00:01:00 +0000
tags:
  digital-antiquaria
  how-to
  sre
---

This week I got an external floppy drive, dug up some old 3.5" floppy disks
from my youth, and imaged them to see what was on them. Here's the process I used.

Thanks to these articles for getting me started:

- ["How To Create Disk Image on Mac OS X With dd Command"](https://www.cyberciti.biz/faq/how-to-create-disk-image-on-mac-os-x-with-dd-command/) (Vivek Gite, August 2018)

- ["Running dd. Why resource is busy?"](https://unix.stackexchange.com/questions/271471/running-dd-why-resource-is-busy) (StackExchange, March 2016)

- ["How to mount raw disk images?"](https://apple.stackexchange.com/a/188556/266950) (StackExchange, May 2015)


## To make a disk image of a floppy disk on OS X

Insert the floppy into your external disk drive.
Wait for it to auto-mount.

Run `diskutil list` to find the identifier of your external disk drive.
It will be marked `(external, physical)`. In my case, what I see is
`/dev/disk3`.

Run `diskutil umount /dev/disk3`. You'll see the message
"Volume (whatever) on disk3 unmounted".

This next step can be fatal if you make a typo! Carefully, run
`sudo dd if=/dev/disk3 of=mydisk.dd bs=512 conv=noerror,sync`,
replacing `/dev/disk3` with the identifier of your
external disk drive. The argument `conv=noerror,sync` is not strictly
needed, but if you omit it, any single I/O error on the disk will
abort the transfer and leave you with an incomplete image.
The argument `bs=512` gives you a "block size" of 512 bytes, the same size
as one of the sectors on a 1.4MB floppy.

> Be careful with [`dd`](https://linux.die.net/man/1/dd)!
> If you type `of=mydisk.dd` instead, you'll trash your floppy disk.
> If you type `if=/dev/thewrongdisk`, you'll copy the wrong disk
> (and it might be very big).

If all goes well, you'll see output almost identical to the following:

    2880+0 records in
    2880+0 records out
    1474560 bytes transferred in 114.128214 secs (12920 bytes/sec)

But, if the disk had I/O errors, you'll see output like this:

    dd: /dev/disk2: Input/output error
    2872+0 records in
    2872+0 records out
    1470464 bytes transferred in 113.303469 secs (12978 bytes/sec)
    dd: /dev/disk2: Input/output error
    dd: /dev/disk2: Input/output error
    2873+0 records in
    2873+0 records out
    1470976 bytes transferred in 114.298722 secs (12870 bytes/sec)
    dd: /dev/disk2: Input/output error
    2880+0 records in
    2880+0 records out
    1474560 bytes transferred in 114.299519 secs (12901 bytes/sec)

Each time it sees an I/O error, `dd` will skip to the next 512-byte sector, padding
the current one out with zeros. (Notice how 1470464 bytes is evenly divisible by
512.)

When you're done, push the button to manually eject the floppy disk from the disk drive.
There should be no need to unmount it via Finder or `diskutil umount`,
because it should still be unmounted from when you ran `diskutil umount /dev/disk3`
earlier.


# To mount a `dd` disk image on OS X

This one is simple. Run `cp mydisk.dd mydisk.dmg`, and
then double-click `mydisk.dmg` in Finder. The mounted volume will magically appear
under `/Volumes/volume-label` (or apparently as `/Volumes/Untitled` if the volume
has no [label](https://linux.die.net/man/1/mlabel)).

Beware — when OS X mounts a volume from a .dmg, it doesn't make a "scratch copy"
anywhere. If you create, modify, or delete a file on the mounted volume, you will
change the structure of the filesystem *in the original .dmg file!*

Naming the file with a `.dd` extension (unrecognized by OS X) forces you to
make a `.dmg` copy before you'll be able to get at the files; which thus prevents
you from losing data by accidentally double-clicking the original file.
