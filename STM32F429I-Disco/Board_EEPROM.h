/*-----------------------------------------------------------------------------
 * Copyright (c) 2013 - 2019 Arm Limited (or its affiliates). All
 * rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   1.Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   2.Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   3.Neither the name of Arm nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * Name:    Board_EEPROM.h
 * Purpose: EEPROM interface header file
 * Rev.:    1.0.0
 *----------------------------------------------------------------------------*/

#ifndef __BOARD_EEPROM_H
#define __BOARD_EEPROM_H

#include <stdint.h>

/**
  \fn          int32_t EEPROM_Initialize (void)
  \brief       Initialize EEPROM
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
/**
  \fn          int32_t EEPROM_Uninitialize (void)
  \brief       De-initialize EEPROM
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
/**
  \fn          uint32_t EEPROM_GetSize (void)
  \brief       Get EEPROM memory size in bytes
  \returns     Memory size in bytes
*/
/**
  \fn          int32_t EEPROM_ReadData (uint32_t addr, uint8_t *buf, uint32_t len)
  \brief       Read data from EEPROM
  \param[in]   addr   EEPROM address
  \param[in]   buf    Pointer where data will be read from EEPROM
  \param[in]   len    Number of data bytes to read
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/
/**
  \fn          int32_t EEPROM_WriteData (uint32_t addr, const uint8_t *buf, uint32_t len)
  \brief       Write data to EEPROM
  \param[in]   addr   EEPROM address
  \param[in]   buf    Pointer with data to write to EEPROM
  \param[in]   len    Number of data bytes to write
  \returns
   - \b  0: function succeeded
   - \b -1: function failed
*/

extern int32_t  EEPROM_Initialize   (void);
extern int32_t  EEPROM_Uninitialize (void);
extern uint32_t EEPROM_GetSize      (void);
extern int32_t  EEPROM_ReadData     (uint32_t addr,       uint8_t *buf, uint32_t len);
extern int32_t  EEPROM_WriteData    (uint32_t addr, const uint8_t *buf, uint32_t len);

#endif /* __BOARD_EEPROM_H */
