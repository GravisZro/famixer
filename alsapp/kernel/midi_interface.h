#ifndef MIDI_INTERFACE_H
#define MIDI_INTERFACE_H

#include "kernel_interface.h"

namespace Alsa
{
  // Raw MIDI section - /dev/snd/midi??
  namespace Midi
  {
    enum stream_e
    {
      stream_output = 0,
      stream_input,
      stream_count,
    };

    enum info_e
    {
      info_output = 0x00000001,
      info_input  = 0x00000002,
      duplex = 0x00000004,
    };

    struct info_t
    {
      unsigned int         device          ; // RO/WR (control): device number
      unsigned int         subdevice       ; // RO/WR (control): subdevice number
      int                  stream          ; // WR: stream
      int                  card            ; // R: card number
      unsigned int         flags           ; // SNDRV_RAWMIDI_INFO_XXXX
      std::array<__u8, 64> id              ; // ID (user selectable)
      std::array<__u8, 80> name            ; // name of device
      std::array<__u8, 32> subname         ; // name of active or selected subdevice
      unsigned int         subdevices_count;
      unsigned int         subdevices_avail;
      std::array<__u8, 64> reserved        ; // reserved for future use
    };

    struct params_t
    {
      int stream;
      size_t buffer_size;		// queue size in bytes
      size_t avail_min;		// minimum avail bytes for wakeup
      unsigned int no_active_sensing : 1; // do not send active sensing byte in close()
      std::array<__u8, 16> reserved     ; // reserved for future use
    };

    struct status_t
    {
      int stream;
      timespec tstamp;		// Timestamp
      size_t avail;       // available bytes
      size_t xruns;       // count of overruns since last status (in bytes)
      std::array<__u8, 16> reserved        ; // reserved for future use
    };
  }

  class KernelMidiInterface : private KernelInterface
  {
  public:
    inline const char* device_filename(void) const { return "/dev/snd/midi%i"; }
    inline int protocol_version_requirement(void) const { return build_protocol_version(2, 0, 0); }

    inline void protocol_version (       int                & value ) { io_r ('W', 0x00, value); }
    inline void info             (       Midi::info_t       & value ) { io_r ('W', 0x01, value); }

    inline void params           (       Midi::params_t     & value ) { io_rw('W', 0x10, value); }
    inline void status           (       Midi::status_t     & value ) { io_rw('W', 0x20, value); }

    inline void drop             ( const int                & value ) { io_w ('W', 0x30, value); }
    inline void drain            ( const int                & value ) { io_w ('W', 0x31, value); }
  };
}

#endif // MIDI_INTERFACE_H
