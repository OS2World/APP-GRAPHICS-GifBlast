
Contents of this disk (GIFBLAST version 2.0):

README
    -  This file.
GIFBLAST.EXE
    -  Executable GIFBLAST program, MSDOS version 2.0.
       Run GIFBLAST -H for instructions.
SOURCE\
    -  Directory containing source code for GIFBLAST version 2.0
       in fairly generic C. This is provided to let you compile
       and use GIFBLAST on systems other than the PC.
GIFBLA20.ZIP
    -  An image of this disk compressed using PKZIP version 1.1.
       Use PKUNZIP -d GIFBLA20 to unzip. This is provided for your
       convenience when giving GIFBLAST to friends or uploading it
       to bulletin boards.



General information on GIFBLAST:

What is GIFBLAST?
GIFBLAST is a compressor designed especially for GIF files. The usual
file compressors (PKZIP, ARJ, etc.) do not work on GIF files; GIFBLAST
does. When you run GIFBLAST on a GIF file X.GIF, you get a compressed
file X.GFB that is usually 20-25% smaller. In order to view this file
you have to decompress X.GFB to get back the original X.GIF file. The
compression used in GIFBLAST is lossless, i.e. it perfectly preserves
all image pixels and other information in GIF files.

What can I use GIFBLAST for?
The compression/decompression algorithm used in GIFBLAST is quite
slow, so you would probably not want to store frequently viewed GIF
files in compressed form. However, GIFBLAST should be useful in
applications where getting maximum compression is important, such as:
- Storing GIF files on bulletin boards for downloading.
- Posting GIF files on usenet.
- Transmitting GIF files via modem.
- Sending GIF files on floppy disk.
Because there is no loss involved when compressing with GIFBLAST,
the user need not be concerned with harming the image or introducing
compression artifacts.
