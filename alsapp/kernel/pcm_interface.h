#ifndef PCM_INTERFACE_H
#define PCM_INTERFACE_H

#include "kernel_interface.h"

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define ENDIAN_EXTENTION(x)  x##_le
#else
#define ENDIAN_EXTENTION(x)  x##_be
#endif


namespace Alsa
{

  namespace PCM
  {

    typedef unsigned long uframes_t;
    typedef signed long sframes_t;

    enum class_e
    {
      generic = 0,  // standard mono or stereo device
      multi,        // multichannel device
      modem,        // software modem class
      digitizer,    // digitizer class
      class_count,
    };

    enum subclass_e
    {
      generic_mix = 0, // mono or stereo subdevices are mixed together
      class_multi,     // multichannel subdevices are mixed together
      subclass_count,
    };

    enum stream_e
    {
      playback = 0,
      capture,
      stream_count,
    };

    enum access_e : int __bitwise
    {
      mmap_interleaved = 0,  // interleaved mmap
      mmap_noninterleaved,   // noninterleaved mmap
      mmap_complex,          // complex mmap
      rw_interleaved,        // readi/writei
      rw_noninterleaved,      // readn/writen
      access_count,
    };

    enum format_e : int __bitwise
    {
      s8 = 0,
      u8,
      s16_le,
      s16_be,
      u16_le,
      u16_be,
      s24_le,
      s24_be,
      u24_le,
      u24_be,
      s32_le,
      s32_be,
      u32_le,
      u32_be,
      float32_le,
      float32_be,
      float64_le,
      float64_be,

      iec958_subframe_le,
      iec958_subframe_be,

      mu_law,
      a_law,
      ima_adpcm,
      mpeg,
      gsm,

      special = 31,

      s24_3le,
      s24_3be,
      u24_3le,
      u24_3be,

      s20_3le,
      s20_3be,
      u20_3le,
      u20_3be,

      s18_3le,
      s18_3be,
      u18_3le,
      u18_3be,

      g723_24,
      g723_24_1b,
      g723_40,
      g723_40_1b,

      dsd_u8,
      dsd_u16_le,
      format_count,


      s16 = ENDIAN_EXTENTION(s16),
      u16 = ENDIAN_EXTENTION(u16),
      s24 = ENDIAN_EXTENTION(s24),
      u24 = ENDIAN_EXTENTION(u24),
      s32 = ENDIAN_EXTENTION(s32),
      u32 = ENDIAN_EXTENTION(u32),
      float32         = ENDIAN_EXTENTION(float32),
      float64         = ENDIAN_EXTENTION(float64),
      iec958_subframe = ENDIAN_EXTENTION(iec958_subframe),
    };

    enum subformat_e : int __bitwise
    {
      subformat_standard = 0,
      subformat_count
    };

    enum info_e
    {
      mmap             = 0x00000001, // hardware supports mmap
      mmap_valid       = 0x00000002, // period data are valid during transfer
      double_buffer    = 0x00000004, // Double buffering needed for PCM start/stop
      batch_buffer     = 0x00000010, // double buffering
      interleaved      = 0x00000100, // channels are interleaved
      noninterleaved   = 0x00000200, // channels are not interleaved
      complex          = 0x00000400, // complex frame organization (mmap only)
      block_transfer   = 0x00010000, // hardware transfer block of samples
      overrange        = 0x00020000, // hardware supports ADC (capture) overrange detection
      resume           = 0x00040000, // hardware supports stream resume after suspend
      pause            = 0x00080000, // pause ioctl is supported
      half_duplex      = 0x00100000, // only half duplex
      joint_duplex     = 0x00200000, // playback and capture stream are somewhat correlated
      sync_start       = 0x00400000, // pcm support some kind of sync go
      no_period_wakeup = 0x00800000, // period wakeup can be disabled
      has_wall_clock   = 0x01000000, // has audio wall clock for audio/system time sync
      fifo_in_frames   = 0x80000000, // internal kernel flag - FIFO size is in frames
    };


    enum state_e : int __bitwise
    {
      open     = 0, // stream is open
      setup       , // stream has a setup
      prepared    , // stream is ready to start
      running     , // stream is running
      xrun        , // stream reached an xrun
      draining    , // stream is draining
      paused      , // stream is paused
      suspended   , // hardware is suspended
      disconnected, // hardware is disconnected
      state_count ,
    };

    enum offset_e
    {
      data    = 0x00000000,
      status  = 0x80000000,
      control = 0x81000000,
    };


    union sync_id_t
    {
      std::array<__u8 , 16> id  ;
      std::array<__u16,  8> id16;
      std::array<__u32,  4> id32;
    };

    struct info_t
    {
      unsigned int         device          ; // RO/WR (control): device number
      unsigned int         subdevice       ; // RO/WR (control): subdevice number
      int                  stream          ; // RO/WR (control): stream direction
      int                  card            ; // R: card number
      std::array<__u8, 64> id              ; // ID (user selectable)
      std::array<__u8, 80> name            ; // name of this device
      std::array<__u8, 32> subname         ; // subdevice name
      int                  dev_class       ; // SNDRV_PCM_CLASS_*
      int                  dev_subclass    ; // SNDRV_PCM_SUBCLASS_*
      unsigned int         subdevices_count;
      unsigned int         subdevices_avail;
      sync_id_t            sync            ; // hardware synchronization ID
      std::array<__u8, 64> reserved        ; // reserved for future use
    };

    enum hw_param_e : int
    {
      access      = 0, // Access type
      format         , // Format
      subformat      , // Subformat
      mask_count     ,

      sample_bits = 8, // Bits per sample
      frame_bits     , // Bits per frame
      channels       , // Channels
      rate           , // Approx rate
      period_time    , // Approx distance between interrupts in microseconds
      period_size    , // Approx frames between interrupts
      period_bytes   , // Approx bytes between interrupts
      periods        , // Approx interrupts per buffer

      buffer_time    , // Approx duration of buffer in microseconds
      buffer_size    , // Size of buffer in frames
      buffer_bytes   , // Size of buffer in bytes
      tick_time      , // Approx tick duration in microseconds

      interval_count = tick_time - sample_bits + 1,

      noresample       = 1, // avoid rate resampling
      export_buffer    = 2, // export buffer
      no_period_buffer = 4, // disable period wakeups
    };

    struct interval_t
    {
      unsigned int min;
      unsigned int max;
      unsigned int openmin : 1;
      unsigned int openmax : 1;
      unsigned int integer : 1;
      unsigned int empty   : 1;
    };

#define SNDRV_MASK_MAX	256

    struct mask_t
    {
      std::array<__u32, (SNDRV_MASK_MAX + 31) / 32> bits;
    };

    struct hw_params_t
    {
      unsigned int flags;

      std::array<mask_t, mask_count> masks;
      std::array<mask_t, 5>         mres; // reserved masks

      std::array<interval_t, interval_count> intervals;
      std::array<interval_t, 9>     ires; // reserved intervals

      unsigned int         rmask     ; // W: requested masks
      unsigned int         cmask     ; // R: changed masks
      unsigned int         info      ; // R: Info flags for returned setup
      unsigned int         msbits    ; // R: used most significant bits
      unsigned int         rate_num  ; // R: rate numerator
      unsigned int         rate_den  ; // R: rate denominator
      uframes_t            fifo_size ; // R: chip FIFO size in frames
      std::array<__u8, 64> reserved  ; // reserved for future use
    };

    enum timestamp_e
    {
      timestamp_none = 0,
      timestamp_enable,
      timestamp_count,
    };

    struct sw_params_t
    {
      int                  tstamp_mode      ; // timestamp mode
      unsigned int         period_step      ;
      unsigned int         sleep_min        ; // min ticks to sleep
      uframes_t            avail_min        ; // min avail frames for wakeup
      uframes_t            xfer_align       ; // obsolete: xfer size need to be a multiple
      uframes_t            start_threshold  ; // min hw_avail frames for automatic start
      uframes_t            stop_threshold   ; // min avail frames for automatic stop
      uframes_t            silence_threshold; // min distance from noise for silence filling
      uframes_t            silence_size     ; // silence block size
      uframes_t            boundary         ; // pointers wrap point
      std::array<__u8, 64> reserved         ; // reserved for future use
    };

    struct channel_info_t
    {
      unsigned int    channel ;
      __kernel_off_t  offset  ; // mmap offset
      unsigned int    first   ; // offset to first sample in bits
      unsigned int    step    ; // samples distance in bits
    };

    struct status_t
    {
      state_e   state           ; // stream state
      timespec  trigger_tstamp  ; // time when stream was started/stopped/paused
      timespec  tstamp          ; // reference timestamp
      uframes_t appl_ptr        ; // appl ptr
      uframes_t hw_ptr          ; // hw ptr
      sframes_t delay           ; // current delay in frames
      uframes_t avail           ; // number of frames available
      uframes_t avail_max       ; // max frames available on hw since last status
      uframes_t overrange       ; // count of ADC (capture) overrange detections from last status
      state_e   suspended_state ; // suspended stream state
      __u32     : 0             ; // must be filled with zero
      timespec  audio_tstamp    ; // from sample counter or wall clock


      std::array<__u8, 56 - sizeof(timespec)> reserved; // must be filled with zero
    };

    struct mmap_status_t
    {
      state_e   state           ; // RO: state - SNDRV_PCM_STATE_XXXX
      int       : 0             ; // Needed for 64 bit alignment
      uframes_t hw_ptr          ; // RO: hw ptr (0...boundary-1)
      timespec  tstamp          ; // Timestamp
      state_e   suspended_state ; // RO: suspended stream state
      timespec  audio_tstamp    ; // from sample counter or wall clock
    };

    struct mmap_control_t
    {
      uframes_t appl_ptr  ; // RW: appl ptr (0...boundary-1)
      uframes_t avail_min ; // RW: min available frames for wakeup
    };

    enum sync_ptr_e
    {
      hw_sync   = 0x00000001, // execute hwsync
      appl      = 0x00000002, // get appl_ptr from driver (r/w op)
      avail_min	= 0x00000004, // get avail_min from driver
    };


    //static_assert(sizeof(mmap_status_t ) == 64, "whoops");
    //static_assert(sizeof(mmap_control_t) == 64, "whoops");

    struct sync_ptr_t
    {
      unsigned int flags;
      union {
        struct mmap_status_t status;
        std::array<__u8, 64> reserved;
      } s;
      union {
        mmap_control_t control;
        std::array<__u8, 64> reserved;
      } c;
    };

    struct xfer_i_t
    {
      sframes_t result;
      void*     buf   ;
      uframes_t frames;
    };

    struct xfer_n_t
    {
      sframes_t result;
      void**    bufs  ;
      uframes_t frames;
    };

  }

  enum timestamp_type_e
  {
    time_of_day = 0,	// gettimeofday equivalent
    monotonic,  // posix_clock_monotonic equivalent
    timestamp_type_count,
  };

  // channel positions
  enum channel_e
  {
    unknown = 0,
    silent,
    mono,
    // this follows the alsa-lib mixer channel value + 3
    front_left,
    front_right,
    rear_left,
    rear_right,
    front_center,
    LFE,
    side_left,
    side_right,
    rear_center,
    // new definitions
    front_left_center,
    front_right_center,
    rear_left_center,
    rear_right_center,
    front_left_wide,
    front_right_wide,
    front_left_high,
    front_center_high,
    front_right_high,
    top_center,
    top_front_left,
    top_front_right,
    top_front_center,
    top_rear_left,
    top_rear_right,
    top_rear_center,
    // new definitions for UAC2
    top_front_left_center,
    top_front_right_center,
    top_side_left,
    top_side_right,
    left_LFE,
    right_LFE,
    bottom_center,
    bottom_left_center,
    bottom_right_center,
    channel_count,
  };

  class KernelPCMInterface : private KernelInterface
  {
  public:
    inline const char* device_filename(void) const { return "/dev/snd/pcm%i"; }
    inline int protocol_version_requirement(void) const { return build_protocol_version(2, 0, 11); }

    inline void protocol_version (       int                & value ) { io_r ('A', 0x00, value); }
    inline void info             (       PCM::info_t        & value ) { io_r ('A', 0x01, value); }
    inline void tstamp           ( const int                & value ) { io_w ('A', 0x02, value); }
    inline void ttstamp          ( const int                & value ) { io_w ('A', 0x03, value); }

    inline void hw_refine        (       PCM::hw_params_t   & value ) { io_rw('A', 0x10, value); }
    inline void hw_params        (       PCM::hw_params_t   & value ) { io_rw('A', 0x11, value); }
    inline void hw_free          ( void                             ) { io   ('A', 0x12       ); }

    inline void sw_params        (       PCM::sw_params_t   & value ) { io_rw('A', 0x13, value); }
    inline void status           (       PCM::status_t      & value ) { io_r ('A', 0x20, value); }
    inline void delay            (       PCM::sframes_t     & value ) { io_r ('A', 0x21, value); }


    inline void hw_sync          ( void                             ) { io   ('A', 0x22       ); }

    inline void sync_ptr         (       PCM::sync_ptr_t    & value ) { io_rw('A', 0x23, value); }

    inline void channel_info     (       PCM::channel_info_t& value ) { io_r ('A', 0x32, value); }
    inline void prepare          ( void                             ) { io   ('A', 0x40       ); }
    inline void reset            ( void                             ) { io   ('A', 0x41       ); }
    inline void start            ( void                             ) { io   ('A', 0x42       ); }
    inline void drop             ( void                             ) { io   ('A', 0x43       ); }
    inline void drain            ( void                             ) { io   ('A', 0x44       ); }

    inline void xrun             ( void                             ) { io   ('A', 0x48       ); }
    inline void pause            ( const int                & value ) { io_w ('A', 0x45, value); }
    inline void resume           ( void                             ) { io   ('A', 0x47       ); }

    inline void forward          ( const PCM::uframes_t     & value ) { io_w ('A', 0x49, value); }
    inline void rewind           ( const PCM::uframes_t     & value ) { io_w ('A', 0x46, value); }

    inline void write_i_frames   ( const PCM::xfer_i_t      & value ) { io_w ('A', 0x50, value); }
    inline void read_i_frames    (       PCM::xfer_i_t      & value ) { io_r ('A', 0x51, value); }
    inline void write_n_frames   ( const PCM::xfer_n_t      & value ) { io_w ('A', 0x52, value); }
    inline void read_n_frames    (       PCM::xfer_n_t      & value ) { io_r ('A', 0x53, value); }


    inline void link             ( const int                & value ) { io_w ('A', 0x60, value); }
    inline void unlink           ( void                             ) { io   ('A', 0x61       ); }
  };
}

#endif // PCM_INTERFACE_H
