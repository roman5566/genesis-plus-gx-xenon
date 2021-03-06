/***************************************************************************************
 *  Genesis Plus
 *  Savestate support
 *
 *  Copyright (C) 2007-2011  Eke-Eke (Genesis Plus GX)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"

int state_load(unsigned char *buffer)
{
  /* buffer size */
  int bufferptr = 0;

  /* first allocate state buffer */
  unsigned char *state = (unsigned char *)malloc(STATE_SIZE);
  if (!state) return 0;

  /* uncompress savestate */
  unsigned long inbytes, outbytes;
  memcpy(&inbytes, buffer, 4);
  outbytes = STATE_SIZE;
  uncompress ((Bytef *)state, &outbytes, (Bytef *)(buffer + 4), inbytes);

  /* signature check (GENPLUS-GX x.x.x) */
  char version[17];
  load_param(version,16);
  version[16] = 0;
  if (strncmp(version,STATE_VERSION,11))
  {
    free(state);
    return -1;
  }

  /* version check (1.5.0 and above) */
  if ((version[11] < 0x31) || ((version[11] == 0x31) && (version[13] < 0x35)))
  {
    free(state);
    return -1;
  }

  /* reset system */
  system_reset();

  // GENESIS
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    load_param(work_ram, sizeof(work_ram));
    load_param(zram, sizeof(zram));
    load_param(&zstate, sizeof(zstate));
    load_param(&zbank, sizeof(zbank));
    if (zstate == 3)
    {
      m68k_memory_map[0xa0].read8   = z80_read_byte;
      m68k_memory_map[0xa0].read16  = z80_read_word;
      m68k_memory_map[0xa0].write8  = z80_write_byte;
      m68k_memory_map[0xa0].write16 = z80_write_word;
    }
    else
    {
      m68k_memory_map[0xa0].read8   = m68k_read_bus_8;
      m68k_memory_map[0xa0].read16  = m68k_read_bus_16;
      m68k_memory_map[0xa0].write8  = m68k_unused_8_w;
      m68k_memory_map[0xa0].write16 = m68k_unused_16_w;
    }
  }
  else
  {
    load_param(work_ram, 0x2000);
  }

  /* extended state */
  load_param(&mcycles_68k, sizeof(mcycles_68k));
  load_param(&mcycles_z80, sizeof(mcycles_z80));

  // IO
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    load_param(io_reg, sizeof(io_reg));
    io_reg[0] = region_code | 0x20 | (config.tmss & 1);
  }
  else
  {
    load_param(&io_reg[0x0F], 1);
  }

  // VDP
  bufferptr += vdp_context_load(&state[bufferptr], version);

  // SOUND 
  bufferptr += sound_context_load(&state[bufferptr], version);

  // 68000 
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    uint16 tmp16;
    uint32 tmp32;
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D0, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D1, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D2, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D3, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D4, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D5, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D6, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_D7, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A0, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A1, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A2, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A3, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A4, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A5, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A6, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_A7, tmp32);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_PC, tmp32);  
    load_param(&tmp16, 2); m68k_set_reg(M68K_REG_SR, tmp16);
    load_param(&tmp32, 4); m68k_set_reg(M68K_REG_USP,tmp32);
  }

  // Z80 
  load_param(&Z80, sizeof(Z80_Regs));
  Z80.irq_callback = z80_irq_callback;

  // Cartridge HW
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {  
    bufferptr += md_cart_context_load(&state[bufferptr]);
  }
  else
  {
    bufferptr += sms_cart_context_load(&state[bufferptr]);
  }

  free(state);
  return 1;
}

int state_save(unsigned char *buffer)
{
  /* buffer size */
  int bufferptr = 0;

  /* first allocate state buffer */
  unsigned char *state = (unsigned char *)malloc(STATE_SIZE);
  if (!state) return 0;

  /* version string */
  char version[16];
  strncpy(version,STATE_VERSION,16);
  save_param(version, 16);

  // GENESIS
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    save_param(work_ram, sizeof(work_ram));
    save_param(zram, sizeof(zram));
    save_param(&zstate, sizeof(zstate));
    save_param(&zbank, sizeof(zbank));
  }
  else
  {
    save_param(work_ram, 0x2000);
  }
  save_param(&mcycles_68k, sizeof(mcycles_68k));
  save_param(&mcycles_z80, sizeof(mcycles_z80));

  // IO
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    save_param(io_reg, sizeof(io_reg));
  }
  else
  {
    save_param(&io_reg[0x0F], 1);
  }

  // VDP
  bufferptr += vdp_context_save(&state[bufferptr]);

  // SOUND
  bufferptr += sound_context_save(&state[bufferptr]);

  // 68000 
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    uint16 tmp16;
    uint32 tmp32;
    tmp32 = m68k_get_reg(M68K_REG_D0);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D1);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D2);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D3);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D4);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D5);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D6);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_D7);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A0);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A1);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A2);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A3);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A4);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A5);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A6);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_A7);  save_param(&tmp32, 4);
    tmp32 = m68k_get_reg(M68K_REG_PC);  save_param(&tmp32, 4);
    tmp16 = m68k_get_reg(M68K_REG_SR);  save_param(&tmp16, 2); 
    tmp32 = m68k_get_reg(M68K_REG_USP); save_param(&tmp32, 4);
  }

  // Z80 
  save_param(&Z80, sizeof(Z80_Regs));

  // Cartridge HW
  if ((system_hw & SYSTEM_PBC) == SYSTEM_MD)
  {
    bufferptr += md_cart_context_save(&state[bufferptr]);
  }
  else
  {
    bufferptr += sms_cart_context_save(&state[bufferptr]);
  }

  /* compress state file */
  unsigned long inbytes   = bufferptr;
  unsigned long outbytes  = STATE_SIZE;
  compress2 ((Bytef *)(buffer + 4), &outbytes, (Bytef *)state, inbytes, 9);
  memcpy(buffer, &outbytes, 4);
  free(state);

  /* return total size */
  return (outbytes + 4);
}
