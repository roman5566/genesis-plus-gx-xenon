/****************************************************************************
 *  config.c
 *
 *  Genesis Plus GX configuration file support
 *
 *  Copyright Eke-Eke (2007-2011)
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
#include "gui.h"
#include "menu.h"
#include "file_load.h"

static int config_load(void)
{
  /* open configuration file */
  char fname[MAXPATHLEN];
  sprintf (fname, "%s/config.ini", DEFAULT_PATH);
  FILE *fp = fopen(fname, "rb");
  if (fp)
  {
    /* read version */
    char version[16];
    fread(version, 16, 1, fp); 
    fclose(fp);
    if (strncmp(version,CONFIG_VERSION,16))
      return 0;

    /* read file */
    fp = fopen(fname, "rb");
    if (fp)
    {
      fread(&config, sizeof(config), 1, fp);
      fclose(fp);
      return 1;
    }
  }
  return 0;
}

void config_save(void)
{
  /* open configuration file */
  char fname[MAXPATHLEN];
  sprintf (fname, "%s/config.ini", DEFAULT_PATH);
  FILE *fp = fopen(fname, "wb");
  if (fp)
  {
    /* write file */
    fwrite(&config, sizeof(config), 1, fp);
    fclose(fp);
  }
}

void config_default(void)
{
  /* version TAG */
  strncpy(config.version,CONFIG_VERSION,16);

  /* sound options */
  config.psg_preamp     = 150;
  config.fm_preamp      = 100;
  config.hq_fm          = 1;
  config.psgBoostNoise  = 0;
  config.filter         = 0;
  config.lp_range       = 50;
  config.low_freq       = 880;
  config.high_freq      = 5000;
  config.lg             = 1.0;
  config.mg             = 1.0;
  config.hg             = 1.0;
  config.rolloff        = 0.990;
  config.dac_bits 		  = 14;
  config.ym2413         = 2; /* AUTO */

  /* system options */
  config.system         = 0; /* AUTO */
  config.region_detect  = 0; /* AUTO */
  config.force_dtack    = 0;
  config.addr_error     = 1;
  config.tmss           = 0;
  config.lock_on        = 0;
  config.romtype        = 0;
  config.hot_swap       = 0;

  /* video options */
  config.xshift   = 0;
  config.yshift   = 0;
  config.xscale   = 0;
  config.yscale   = 0;
  config.aspect   = 1;
  config.overscan = 3; /* FULL */
  config.ntsc     = 0;
  if (VIDEO_HaveComponentCable())
  {
    config.render   = 2;
    config.bilinear = 1;
  }
  else
  {
    config.render   = 0;
    config.bilinear = 0;
  }

#ifdef HW_RVL
  config.trap     = 0;
  config.gamma = VI_GM_1_0 / 10.0;
#endif

  /* controllers options */
  config.gun_cursor[0]  = 1;
  config.gun_cursor[1]  = 1;
  config.invert_mouse   = 0;
  gx_input_SetDefault();

  /* menu options */
  config.autoload     = 0;
  config.autocheat    = 0;
#ifdef HW_RVL
  config.s_auto       = 1;
#else
  config.s_auto       = 0;
#endif
  config.s_default    = 1;
  config.s_device     = 0;
  config.bg_type      = 0;
  config.bg_overlay   = 1;
  config.screen_w     = 658;
  config.bgm_volume   = 100.0;
  config.sfx_volume   = 100.0;

  /* default ROM directories */
#ifdef HW_RVL
  sprintf (config.lastdir[TYPE_SD],  "sd:%s/roms/",  DEFAULT_PATH);
  sprintf (config.lastdir[TYPE_USB], "usb:%s/roms/", DEFAULT_PATH);
  sprintf (config.lastdir[TYPE_DVD], "dvd:%s/roms/", DEFAULT_PATH);
#else
  sprintf (config.lastdir[TYPE_SD],  "%s/roms/",  DEFAULT_PATH);
  sprintf (config.lastdir[TYPE_DVD], "dvd:%s/roms/", DEFAULT_PATH);
#endif

  /* try to restore settings from config file */
  if (!config_load()) GUI_WaitPrompt("Info","Default Settings restored");

  /* restore inputs */
  input_init();

  /* restore menu settings */
  menu_configure();
}

