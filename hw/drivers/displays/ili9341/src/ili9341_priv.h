/*****************************************************************************/
/*!
    @file     ili9341_priv.h
    @author   ktown (Adafruit Industries)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2016, Adafruit Industries (adafruit.com)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
    THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*****************************************************************************/
#ifndef __ADAFRUIT_ILI9341_PRIV_H__
#define __ADAFRUIT_ILI9341_PRIV_H__

#define ILI9341_TFTWIDTH    (240)
#define ILI9341_TFTHEIGHT   (320)

#define ILI9341_NOP         (0x00)
#define ILI9341_SWRESET     (0x01)
#define ILI9341_RDDID       (0x04)
#define ILI9341_RDDST       (0x09)

#define ILI9341_SLPIN       (0x10)
#define ILI9341_SLPOUT      (0x11)
#define ILI9341_PTLON       (0x12)
#define ILI9341_NORON       (0x13)

#define ILI9341_RDMODE      (0x0A)
#define ILI9341_RDMADCTL    (0x0B)
#define ILI9341_RDPIXFMT    (0x0C)
#define ILI9341_RDIMGFMT    (0x0D)
#define ILI9341_RDSELFDIAG  (0x0F)

#define ILI9341_INVOFF      (0x20)
#define ILI9341_INVON       (0x21)
#define ILI9341_GAMMASET    (0x26)
#define ILI9341_DISPOFF     (0x28)
#define ILI9341_DISPON      (0x29)

#define ILI9341_CASET       (0x2A)
#define ILI9341_PASET       (0x2B)
#define ILI9341_RAMWR       (0x2C)
#define ILI9341_RAMRD       (0x2E)

#define ILI9341_PTLAR       (0x30)
#define ILI9341_MADCTL      (0x36)
#define ILI9341_PIXFMT      (0x3A)

#define ILI9341_FRMCTR1     (0xB1)
#define ILI9341_FRMCTR2     (0xB2)
#define ILI9341_FRMCTR3     (0xB3)
#define ILI9341_INVCTR      (0xB4)
#define ILI9341_DFUNCTR     (0xB6)

#define ILI9341_PWCTR1      (0xC0)
#define ILI9341_PWCTR2      (0xC1)
#define ILI9341_PWCTR3      (0xC2)
#define ILI9341_PWCTR4      (0xC3)
#define ILI9341_PWCTR5      (0xC4)
#define ILI9341_VMCTR1      (0xC5)
#define ILI9341_VMCTR2      (0xC7)

#define ILI9341_RDID1       (0xDA)
#define ILI9341_RDID2       (0xDB)
#define ILI9341_RDID3       (0xDC)
#define ILI9341_RDID4       (0xDD)

#define ILI9341_GMCTRP1     (0xE0)
#define ILI9341_GMCTRN1     (0xE1)

#ifdef __cplusplus
extern "C" {
#endif

/* ili9341.c */
int ili9341_spi_write(uint8_t);
int ili9341_write_command(uint8_t);
int ili9341_write_data(uint8_t);

/* ili9341_shell.c */
#if MYNEWT_VAL(ILI9341_CLI)
int ili9341_shell_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ADAFRUIT_ILI9341_PRIV_H_ */
