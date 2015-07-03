#ifndef CONTROLLER_INTERFACE_H
#define CONTROLLER_INTERFACE_H

#include "kernel_interface.h"

#include "hwdep_interface.h"
#include "pcm_interface.h"
#include "midi_interface.h"

namespace Alsa
{
  namespace Control
  {
    typedef int __bitwise elem_type_t;
    typedef int __bitwise elem_iface_t;

    struct card_info_t
    {
      int                   card      ; // card number
      int                   reserved1 ; // reserved for future use (was type)
      std::array<__u8,  16> id        ; // ID of card (user selectable)
      std::array<__u8,  16> driver    ; // Driver name
      std::array<__u8,  32> name      ; // Short name of soundcard
      std::array<__u8,  80> longname  ; // name + info text about soundcard
      std::array<__u8,  16> reserved2 ; // reserved for future use (was ID of mixer)
      std::array<__u8,  80> mixername ; // visual mixer identification
      std::array<__u8, 128> components; // card components / fine identification, delimited with one space (AC97 etc..)
    };

    struct elem_id_t
    {
      unsigned int         numid    ; // numeric identifier, zero = invalid
      elem_iface_t         iface    ; // interface identifier
      unsigned int         device   ; // device/client number
      unsigned int         subdevice; // subdevice (substream) number
      std::array<__u8, 44> name     ; // ASCII name of item
      unsigned int         index    ; // index of item
    };

    struct elem_list_t
    {
      unsigned int         offset   ; // W: first element ID to get
      unsigned int         space    ; // W: count of element IDs to get
      unsigned int         used     ; // R: count of element IDs set
      unsigned int         count    ; // R: count of all elements
      elem_id_t*           pids     ; // R: IDs
      std::array<__u8, 50> reserved ;
    };

    template<typename T>
    struct minmaxstep_t
    {
      T min;   // R: minimum value
      T max;   // R: maximum value
      T step;  // R: step (0 variable)
    };

    struct elem_info_t
    {
      elem_id_t      id     ; // W: element ID
      elem_type_t    type   ; // R: value type - SNDRV_CTL_ELEM_TYPE_*
      unsigned int   access ; // R: value access (bitmask) - SNDRV_CTL_ELEM_ACCESS_*
      unsigned int   count  ; // count of values
      __kernel_pid_t owner  ; // owner's PID of this control

      union
      {
        minmaxstep_t<__s32> data32;
        minmaxstep_t<__s64> data64;

        struct
        {
          unsigned int         items        ; // R: number of items
          unsigned int         item         ; // W: item number
          std::array<char, 64> name         ; // R: value name
          __u64                names_ptr    ; // W: names list (ELEM_ADD only)
          unsigned int         names_length ;
        } enumerated;

        std::array<__u8, 128> bytes;
      } value;

      std::array<unsigned short, 4> dimensions;
      std::array<__u8, 64 - sizeof(dimensions)> reserved;
    };

    struct aes_iec958_t
    {
      std::array<__u8,  24> status      ;
      std::array<__u8, 147> subcode     ;
      __u8                  : 0         ;
      std::array<__u8,   4> dig_subframe;
    };

    struct elem_value_t
    {
      elem_id_t id;  // W: element ID
      unsigned int : 0; // obsoleted

      union
      {
        std::array<__u64,  64> data64;
        std::array<__u32, 128> data32;
        std::array<__u8 , 512> data8 ;
        aes_iec958_t           data  ;
      }; // RO

      timespec      tstamp;
      std::array<__u8, 128 - sizeof(tstamp)> reserved;
    };

    struct tlv_t
    {
      unsigned int numid ; // control element numeric identification
      unsigned int length; // in bytes aligned to 4
      unsigned int tlv[0]; // first TLV
    };


  }

  class KernelControlInterface : private KernelInterface
  {
  public:
    inline const char* device_filename(void) const { return "/dev/snd/controlC%i"; }
    inline int protocol_version_requirement(void) const { return build_protocol_version(2, 0, 3); }

    inline void protocol_version (       int                  & value ) { io_r  ('U', 0x00, value); }
    inline void info             (       Control::card_info_t & value ) { io_r  ('U', 0x01, value); }

    inline void element_list     (       Control::elem_list_t & value ) { io_rw ('U', 0x10, value); }
    inline void element_info     (       Control::elem_info_t & value ) { io_rw ('U', 0x11, value); }
    inline void element_read     (       Control::elem_value_t& value ) { io_rw ('U', 0x12, value); }
    inline void element_write    (       Control::elem_value_t& value ) { io_rw ('U', 0x13, value); }
    inline void element_lock     ( const Control::elem_id_t   & value ) { io_w  ('U', 0x14, value); }
    inline void element_unlock   ( const Control::elem_id_t   & value ) { io_w  ('U', 0x15, value); }
    inline void subscribe_events (       int                  & value ) { io_rw ('U', 0x16, value); }
    inline void element_add      (       Control::elem_info_t & value ) { io_rw ('U', 0x17, value); }
    inline void element_replace  (       Control::elem_info_t & value ) { io_rw ('U', 0x18, value); }
    inline void element_remove   (       Control::elem_id_t   & value ) { io_rw ('U', 0x19, value); }

    // TLV = Type Length Value
    inline void tlv_read         (       Control::tlv_t       & value ) { io_rw ('U', 0x1A, value); }
    inline void tlv_write        (       Control::tlv_t       & value ) { io_rw ('U', 0x1B, value); }
    inline void tlv_command      (       Control::tlv_t       & value ) { io_rw ('U', 0x1C, value); }

    inline void hw_next          (       int                  & value ) { io_rw ('U', 0x20, value); }
    inline void hw_info          (       HWDep::info_t        & value ) { io_r  ('U', 0x21, value); }

    inline void pcm_next         (       int                  & value ) { io_r  ('U', 0x30, value); }
    inline void pcm_info         (       PCM::info_t          & value ) { io_rw ('U', 0x31, value); }
    inline void pcm_subdevice    ( const int                  & value ) { io_w  ('U', 0x32, value); }

    inline void midi_next        (       int                  & value ) { io_rw ('U', 0x40, value); }
    inline void midi_info        (       Midi::info_t         & value ) { io_rw ('U', 0x41, value); }
    inline void midi_subdevice   ( const int                  & value ) { io_w  ('U', 0x42, value); }

    inline void power            (       int                  & value ) { io_rw ('U', 0xD0, value); }
    inline void power_state      (       int                  & value ) { io_r  ('U', 0xD1, value); }
  };
}

#endif // CONTROLLER_INTERFACE_H
