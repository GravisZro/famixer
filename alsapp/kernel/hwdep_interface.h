#ifndef HWDEP_INTERFACE_H
#define HWDEP_INTERFACE_H

#include "kernel_interface.h"
#include "timer_interface.h"

namespace Alsa
{
  // Section for driver hardware dependent interface - /dev/snd/hw?
  namespace HWDep
  {
    enum interface_e
    {
      opl2 = 0,
      opl3,
      opl4,
      sb16csp,        // Creative Signal Processor
      emu10k1,        // FX8010 processor in EMU10K1 chip
      yss225,         // Yamaha FX processor
      ics2115,        // Wavetable synth
      sscape,         // Ensoniq SoundScape ISA card (MC68EC000)
      vx,             // Digigram VX cards
      mixart,         // Digigram miXart cards
      usx2y,          // Tascam US122, US224 & US428 usb
      emux_wavetable, // EmuX wavetable
      bluetooth,      // Bluetooth audio
      usx2y_pcm,      // Tascam US122, US224 & US428 rawusb pcm
      pcxhr,          // Digigram PCXHR
      sb_rc,          // SB Extigy/Audigy2NX remote control
      hda,            // HD-audio
      usb_stream,     // direct access to usb stream
      interface_count,
    };

    struct info_t
    {
      unsigned int         device  ; // WR: device number
      int                  card    ; // R: card number
      std::array<__u8, 64> id      ; // ID (user selectable)
      std::array<__u8, 80> name    ; // hwdep name
      int                  iface   ; // hwdep interface
      std::array<__u8, 64> reserved; // reserved for future use
    };

    // generic DSP loader
    struct dsp_status_t
    {
      unsigned int version    ; // R: driver-specific version
      std::array<__u8, 32> id ; // R: driver-specific ID string
      unsigned int num_dsps   ; // R: number of DSP images to transfer
      unsigned int dsp_loaded ; // R: bit flags indicating the loaded DSPs
      unsigned int chip_ready ; // R: 1 = initialization finished
      std::array<__u8, 16> reserved; // reserved for future use
    };

    struct dsp_image_t
    {
      unsigned int          index ; // W: DSP index
      std::array<__u8, 64>  name  ; // W: ID (e.g. file name)
      __u8*                 image ; // W: binary image
      size_t                length; // W: size of image in bytes
      unsigned long driver_data;    // W: driver-specific data
    };
  }

  class KernelHWDepInterface : private KernelInterface
  {
  public:
    inline const char* device_filename(void) const { return "/dev/snd/hw%i"; }
    inline int protocol_version_requirement(void) const { return build_protocol_version(1, 0, 1); }

    inline void protocol_version ( int                & value ) { io_r ('H', 0x00, value); }
    inline void info             ( HWDep::info_t      & value ) { io_r ('H', 0x01, value); }
    inline void dsp_status       ( HWDep::dsp_status_t& value ) { io_r ('H', 0x02, value); }
  };
}

#endif // HWDEP_INTERFACE_H
