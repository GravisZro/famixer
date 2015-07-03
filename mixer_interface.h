#ifndef MIXER_INTERFACE_H
#define MIXER_INTERFACE_H

#include <string>
#include <map>
#include <list>

#include <alsa/asoundlib.h>

#include "errno_t.h"

struct mixer_element_funcs
{
  int (*get_volume_range)(snd_mixer_elem_t* elem, long* min, long* max);
  int (*get_volume      )(snd_mixer_elem_t* elem, snd_mixer_selem_channel_id_t channel, long* value);
  int (*set_volume      )(snd_mixer_elem_t* elem, snd_mixer_selem_channel_id_t channel, long  value);
  int (*set_volume_all  )(snd_mixer_elem_t* elem, long value);

  int (*get_dB_range    )(snd_mixer_elem_t* elem, long* min, long* max);
  int (*get_dB          )(snd_mixer_elem_t* elem, snd_mixer_selem_channel_id_t channel, long* value);
  int (*set_dB          )(snd_mixer_elem_t* elem, snd_mixer_selem_channel_id_t channel, long  value, int dir);
  int (*set_dB_all      )(snd_mixer_elem_t* elem, long value, int dir);

  int (*has_volume      )(snd_mixer_elem_t* elem);
  int (*has_switch      )(snd_mixer_elem_t* elem);
  int (*has_channel     )(snd_mixer_elem_t* elem, snd_mixer_selem_channel_id_t channel);
};



class MixerElement
{
public:
  enum ElementType : uint8_t
  {
    invalid  = 0x0,
    playback = 0x1,
    capture  = 0x2,
    common   = 0x4,
    toggle   = 0x8,
  };

  MixerElement(snd_mixer_elem_t* obj) throw(errno_t);

  std::string string(snd_mixer_selem_channel_id_t channel = SND_MIXER_SCHN_UNKNOWN) throw(errno_t);

  std::string device_name;
  std::string name;

  ElementType type;

  long get_volume(snd_mixer_selem_channel_id_t channel = SND_MIXER_SCHN_UNKNOWN) throw(errno_t);
  void set_volume(                                      long value) throw(errno_t);
  void set_volume(snd_mixer_selem_channel_id_t channel, long value) throw(errno_t);

  long get_dB(snd_mixer_selem_channel_id_t channel = SND_MIXER_SCHN_UNKNOWN) throw(errno_t);
  void set_dB(                                      long value) throw(errno_t);
  void set_dB(snd_mixer_selem_channel_id_t channel, long value) throw(errno_t);

  long min;
  long max;

  long min_dB;
  long max_dB;

  bool native_mute;

  std::map<snd_mixer_selem_channel_id_t, std::string> channels;

private:
  snd_mixer_elem_t* m_obj;
  mixer_element_funcs funcs;
  int  unmuted_volume;

  void check_channel(snd_mixer_selem_channel_id_t channel) throw(errno_t);
};

class MixerInterface
{
public:
  MixerInterface(void) : m_is_open(false) { }
  MixerInterface(int card_num) throw(errno_t) { open(card_num); }
  //~MixerInterface(void) throw(errno_t) { close(); }

  void open (int card_num = -1) throw(errno_t); // default param: open first card
  void close(void             ) throw(errno_t);

  inline bool isOpen       (void) const { return m_is_open    ; }
  inline int  getNumber    (void) const { return m_number     ; }
  std::string getName      (void) const { return m_name       ; }
  std::string getLongName  (void) const { return m_longname   ; }
  std::string getHardwareId(void) const { return m_hardware_id; }
  std::string getDeviceName(void) const { return m_device_name; }

  std::list<MixerElement> elements;

private:
  snd_mixer_t* m_mixer;

  bool         m_is_open;
  int          m_number;
  std::string  m_name;
  std::string  m_longname;
  std::string  m_hardware_id;
  std::string  m_device_name;
};

#endif // MIXER_INTERFACE_H
