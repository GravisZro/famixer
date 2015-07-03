#ifndef TIMER_INTERFACE_H
#define TIMER_INTERFACE_H

#include "kernel_interface.h"

namespace Alsa
{
  // Timer section - /dev/snd/timer
  namespace Timer
  {
    enum class_e
    {
      none = -1,
      slave = 0,
      global,
      card,
      pcm,
      count,
    };

    // slave timer classes
    enum slave_class_e
    {
      slave_class_none = 0,
      application,
      sequencer,		// alias
      OSS_sequencer,	// alias
      slave_class_count,
    };

    enum global_e // global timers (device member)
    {
      system = 0,
      rtc,
      hpet,
      hrtimer,
    };

    enum flags_e // info flags
    {
      flags_slave = 0x00000001, // cannot be controlled
    };

    struct id_t
    {
      int dev_class;
      int dev_sclass;
      int card;
      int device;
      int subdevice;
    };

    struct g_info_t
    {
      id_t tid;	// requested timer ID
      unsigned int flags;		// timer flags - SNDRV_TIMER_FLG_*
      int card;			// card number
      std::array<__u8, 64> id  ; // timer identification
      std::array<__u8, 80> name; // timer name
      unsigned long reserved0;	// reserved for future use
      unsigned long resolution;	// average period resolution in ns
      unsigned long resolution_min;	// minimal period resolution in ns
      unsigned long resolution_max;	// maximal period resolution in ns
      unsigned int clients;		// active timer clients
      std::array<__u8, 32> reserved; // reserved for future use
    };

    struct g_params_t
    {
      id_t tid;	// requested timer ID
      unsigned long period_num;	// requested precise period duration (in seconds) - numerator
      unsigned long period_den;	// requested precise period duration (in seconds) - denominator
      std::array<__u8, 32> reserved; // reserved for future use
    };

    struct g_status_t
    {
      id_t tid;	// requested timer ID
      unsigned long resolution;	// current period resolution in ns
      unsigned long resolution_num;	// precise current period resolution (in seconds) - numerator
      unsigned long resolution_den;	// precise current period resolution (in seconds) - denominator
      std::array<__u8, 32> reserved; // reserved for future use
    };

    struct select_t
    {
      id_t id;	// bind to timer ID
      std::array<__u8, 32> reserved; // reserved for future use
    };

    struct info_t
    {
      unsigned int flags;		// timer flags - SNDRV_TIMER_FLG_*
      int card;			// card number
      std::array<__u8, 64> id  ; // timer identification
      std::array<__u8, 80> name; // timer name
      unsigned long reserved0;	// reserved for future use
      unsigned long resolution;	// average period resolution in ns
      std::array<__u8, 64> reserved; // reserved for future use
    };

    enum ps_flags_e
    {
      automatic   = 0x00000001, // auto start, otherwise one-shot
      exclusive   = 0x00000002, // exclusive use, precise start/stop/pause/continue
      early_event = 0x00000004, // write early event to the poll queue
    };

    struct params_t
    {
      unsigned int flags;		// flags - SNDRV_MIXER_PSFLG_*
      unsigned int ticks;		// requested resolution in ticks
      unsigned int queue_size;	// total size of queue (32-1024)
      unsigned int reserved0;		// reserved, was: failure locations
      unsigned int filter;		// event filter (bitmask of SNDRV_TIMER_EVENT_*)
      std::array<__u8, 60> reserved; // reserved for future use
    };

    struct status_t
    {
      timespec     tstamp;		// Timestamp - last update
      unsigned int resolution;	// current period resolution in ns
      unsigned int lost;		// counter of master tick lost
      unsigned int overrun;		// count of read queue overruns
      unsigned int queue;		// used queue size
      std::array<__u8, 64> reserved; // reserved for future use
    };
  }

  class KernelTimerInterface : private KernelInterface
  {
  public:
    inline const char* device_filename(void) const { return "/dev/snd/timer"; }
    inline int protocol_version_requirement(void) const { return build_protocol_version(2, 0, 6); }

    inline void protocol_version (       int              & value ) { io_r ('T', 0x00, value); }
    inline void next_device      (       Timer::id_t      & value ) { io_rw('T', 0x01, value); }
    inline void tread            ( const int              & value ) { io_w ('T', 0x02, value); }

    inline void g_info           (       Timer::g_info_t  & value ) { io_rw('T', 0x03, value); }
    inline void g_params         ( const Timer::g_params_t& value ) { io_w ('T', 0x04, value); }
    inline void g_status         (       Timer::g_status_t& value ) { io_rw('T', 0x05, value); }

    inline void select           ( const Timer::select_t  & value ) { io_w ('T', 0x10, value); }
    inline void info             (       Timer::info_t    & value ) { io_r ('T', 0x11, value); }
    inline void params           ( const Timer::params_t  & value ) { io_w ('T', 0x12, value); }
    inline void status           (       Timer::status_t  & value ) { io_r ('T', 0x14, value); }

    inline void start            ( void                           ) { io   ('T', 0xA0       ); }
    inline void stop             ( void                           ) { io   ('T', 0xA1       ); }
    inline void resume           ( void                           ) { io   ('T', 0xA2       ); }
    inline void pause            ( void                           ) { io   ('T', 0xA3       ); }
  };
}

#endif // TIMER_INTERFACE_H
