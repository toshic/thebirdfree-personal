CHANGES TO BLUECORE RELEASE FILES - IMPORTANT NOTES FOR OEM CUSTOMERS

The BlueCore firmware is composed of a boot loader and a stack. The boot
loader handles DFU (Device Firmware Upgrade) to allow the stack to be
upgraded via a USB or UART connection. The boot loader can only be upgraded
via SPI or by programming the flash device directly; it is not affected by DFU.

Releases of the boot loader since beta-10.1 have included the ability to
check signatures within DFU files to verify the integrity of the stack being
downloaded. As an interim measure, support for old-style unsigned DFU files
was retained to smooth the transition to use of signed DFU files.

The support for unsigned DFU files has been withdrawn from hci-branch-11.1
and later builds. Hence, it is no longer possible for CSR to supply pre-built
DFU files for use with products other than Casira, Microsira and Nanosira.
Each manufacturer must generate their own DFU files, signed using their own
key.

The following files are now supplied:

loader_unsigned.xpv & loader_unsigned.xdv

	These files contain the boot loader software in a form suitable for
	programming via SPI using BlueFlash.

	This should have an appropriate public key inserted prior to
	programming. Failure to do this will disable checking of DFU file
	signatures, allowing persistent store security to be bypassed.

stack_unsigned.xpv & stack_unsigned.xdv

	The stack software in a form suitable for programming via SPI using
	BlueFlash. These files can also be used to generate DFU files with
	custom idVendor and idProduct fields using dfubuild (see below).

	These files should be signed using an appropriate private key
	prior to programming or construction of a DFU file.

	These are equivalent to the "stack.xpv" and "stack.xdv" files
	contained in some previous releases.
	
The "bcXXX.*" files from this release, available from the Casira
download area of the CSR website, (XXX stands for any 3 alpha-numerical
characters) have been signed for use with Casiras, Microsiras and
Nanosiras only; they are not suitable for OEM use. Attempting
to use these files will mean that the modules can only be upgraded via DFU
using files intended for Casiras, Microsiras and Nanosiras.

Signed DFU files can be generated using the DFU Tools suite of software
which includes full documentation describing the signing process and
associated tools. Old format unsigned DFU files can be generated if necessary
(for upgrading devices with older boot loaders) using the -t0 option of
dfubuild. Contact CSR for copies of these tools.

29/05/2009