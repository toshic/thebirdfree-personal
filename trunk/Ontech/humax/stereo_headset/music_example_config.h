// *****************************************************************************
// Copyright (C) Cambridge Silicon Radio plc 2008             http://www.csr.com
// %%version
//
// $Revision$ $Date$
// *****************************************************************************

// *****************************************************************************
// DESCRIPTION
//    Music Example configuration file
//
//    This file is used to include/exclude music example modules from the build.
//    To include a module define it as 1.  To exclude a module define it as 0.
//
// NOTE
//    A clean build must be executed after making modifications to this file.
//    (i.e. "Rebuild All")
//
// *****************************************************************************
.ifndef MUSIC_EXAMPLE_CONFIG_H
.define MUSIC_EXAMPLE_CONFIG_H

.define uses_PEQ                            0

.ifndef BC5ROM
.define uses_STEREO_ENHANCEMENT             0
.else
.define uses_STEREO_ENHANCEMENT             0
.endif

.ifndef BC5ROM
.define uses_CMPD                           1
.else
.define uses_CMPD                           0
.endif

.ifndef BC5ROM
.define uses_DITHER                         1
.else
.define uses_DITHER                         0
.endif


// Bit-mask flags to return in SPI status, which tells the Parameter Manager
// (Windows Realtime Tuning GUI) which modules are included in the build.  The
// mask is also written to the kap file so that the build configuration can be
// identified using a text editor.

.ifndef BC5ROM
.define MUSIC_EXAMPLE_CONFIG_FLAG\
                        uses_DITHER                     *0x001000 + \
                        uses_CMPD                       *0x000800
.else
.define MUSIC_EXAMPLE_CONFIG_FLAG\
                        uses_PEQ                        *0x008000
.endif
.endif
