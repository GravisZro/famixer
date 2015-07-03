#include "mixer_interface.h"

#include <iostream>

#include <algorithm>
#include <cctype>

namespace std
{
  const char newline = '\n';
}

int devicename_fix(int c)
{
  if(c == ' ')
    return '_';
  return ::tolower(c);
}

MixerElement::MixerElement(snd_mixer_elem_t* obj) throw(errno_t)
  : m_obj(obj)
{
  device_name = name = snd_mixer_selem_get_name(m_obj);

  std::transform(device_name.begin(), device_name.end(), device_name.begin(), devicename_fix);

  if(snd_mixer_selem_has_playback_volume(m_obj))
    type = playback;
  else if(snd_mixer_selem_has_capture_volume(m_obj))
    type = capture;
  else if(snd_mixer_selem_has_common_volume(m_obj))
    type = common;
  else
    type = toggle;

  switch(type)
  {
    case common:
      funcs =
      {
        snd_mixer_selem_get_playback_volume_range,
        snd_mixer_selem_get_playback_volume,
        snd_mixer_selem_set_playback_volume,
        snd_mixer_selem_set_playback_volume_all,

        snd_mixer_selem_get_playback_dB_range,
        snd_mixer_selem_get_playback_dB,
        snd_mixer_selem_set_playback_dB,
        snd_mixer_selem_set_playback_dB_all,

        snd_mixer_selem_has_common_volume,
        snd_mixer_selem_has_common_switch,
        snd_mixer_selem_has_playback_channel
      };
      break;

    case playback:
      funcs =
      {
        snd_mixer_selem_get_playback_volume_range,
        snd_mixer_selem_get_playback_volume,
        snd_mixer_selem_set_playback_volume,
        snd_mixer_selem_set_playback_volume_all,

        snd_mixer_selem_get_playback_dB_range,
        snd_mixer_selem_get_playback_dB,
        snd_mixer_selem_set_playback_dB,
        snd_mixer_selem_set_playback_dB_all,

        snd_mixer_selem_has_playback_volume,
        snd_mixer_selem_has_playback_switch,
        snd_mixer_selem_has_playback_channel
      };
      break;

    case capture:
      funcs =
      {
        snd_mixer_selem_get_capture_volume_range,
        snd_mixer_selem_get_capture_volume,
        snd_mixer_selem_set_capture_volume,
        snd_mixer_selem_set_capture_volume_all,

        snd_mixer_selem_get_capture_dB_range,
        snd_mixer_selem_get_capture_dB,
        snd_mixer_selem_set_capture_dB,
        snd_mixer_selem_set_capture_dB_all,

        snd_mixer_selem_has_capture_volume,
        snd_mixer_selem_has_capture_switch,
        snd_mixer_selem_has_capture_channel,
      };
      break;

    case toggle:
       return; // dunno what to do  ;_;
      break;
    default:
      assert(false); // invalid type! :((((
  }

  native_mute = throw_errno(funcs.has_switch(m_obj));
  throw_errno(funcs.get_volume_range(m_obj, &min, &max));
  throw_errno(funcs.get_dB_range(m_obj, &min_dB, &max_dB));

  check_channel(SND_MIXER_SCHN_FRONT_LEFT  );
  check_channel(SND_MIXER_SCHN_FRONT_RIGHT );
  check_channel(SND_MIXER_SCHN_REAR_LEFT   );
  check_channel(SND_MIXER_SCHN_REAR_RIGHT  );
  check_channel(SND_MIXER_SCHN_FRONT_CENTER);
  check_channel(SND_MIXER_SCHN_WOOFER      );
  check_channel(SND_MIXER_SCHN_SIDE_LEFT   );
  check_channel(SND_MIXER_SCHN_SIDE_RIGHT  );
  check_channel(SND_MIXER_SCHN_REAR_CENTER );
}

std::string MixerElement::string(snd_mixer_selem_channel_id_t channel) throw(errno_t)
{
  std::string output;

  output += name + std::newline;

  if(channel == SND_MIXER_SCHN_UNKNOWN)
  {
    auto pos = channels.begin();

    for(; pos != channels.end(); ++pos)
    {
      output += std::to_string(get_volume(pos->first)) + ':' +
                //std::to_string(get_dB(pos->first)) + ':' +
                pos->second + std::newline;
    }
  }
  else
  {
    output += std::to_string(get_volume(channel)) + ':' +
              //std::to_string(get_dB(channel)) + ':' +
              channels[channel] + std::newline;
  }

  output += std::to_string(min) + ':' + std::to_string(max) + std::newline;
  //output += std::to_string(min) + ':' + std::to_string(min_dB) + ":min" + std::newline;
  //output += std::to_string(max) + ':' + std::to_string(max_dB) + ":max" + std::newline;

  return output;
}

long MixerElement::get_volume(snd_mixer_selem_channel_id_t channel) throw(errno_t)
{
  long value;
  if(channel == SND_MIXER_SCHN_UNKNOWN)
    channel = channels.begin()->first;
  throw_errno(funcs.get_volume(m_obj, channel, &value));
  return value;
}

void MixerElement::set_volume(long value) throw(errno_t)
{ set_volume(SND_MIXER_SCHN_UNKNOWN, value); }

void MixerElement::set_volume(snd_mixer_selem_channel_id_t channel, long value) throw(errno_t)
{
  if(value < min)
    value = min;
  else if(value > max)
    value = max;
  if(channel == SND_MIXER_SCHN_UNKNOWN)
    throw_errno(funcs.set_volume_all(m_obj, value));
  else
    throw_errno(funcs.set_volume(m_obj, channel, value));
}


long MixerElement::get_dB(snd_mixer_selem_channel_id_t channel) throw(errno_t)
{
  long value;
  if(channel == SND_MIXER_SCHN_UNKNOWN)
    channel = channels.begin()->first;
  throw_errno(funcs.get_dB(m_obj, channel, &value));
  return value;
}

void MixerElement::set_dB(long value) throw(errno_t)
{ set_volume(SND_MIXER_SCHN_UNKNOWN, value); }

void MixerElement::set_dB(snd_mixer_selem_channel_id_t channel, long value) throw(errno_t)
{
  if(value < min_dB)
    value = min_dB;
  else if(value > max_dB)
    value = max_dB;
  if(channel == SND_MIXER_SCHN_UNKNOWN)
    throw_errno(funcs.set_dB_all(m_obj, value, 0));
  else
    throw_errno(funcs.set_dB(m_obj, channel, value, 0));
}


void MixerElement::check_channel(snd_mixer_selem_channel_id_t channel) throw(errno_t)
{
  if(throw_errno(funcs.has_channel(m_obj, channel)))
    channels[channel] = snd_mixer_selem_channel_name(channel);
}

void MixerInterface::open(int card_num) throw(errno_t)
{
  assert(card_num >= 0);
  char* tmp_str = nullptr;

  m_hardware_id = "hw:"  + std::to_string(card_num);
  m_device_name = "mixer" + std::to_string(card_num);

  m_number = card_num;
  throw_errno(snd_mixer_open(&m_mixer, 0));
  throw_errno(snd_mixer_attach(m_mixer, m_hardware_id.data()));
  throw_errno(snd_mixer_selem_register(m_mixer, nullptr, nullptr));
  throw_errno(snd_mixer_load(m_mixer));

  m_is_open = true; // card is open and loaded now! :D

  // store name
  throw_errno(snd_card_get_name(m_number, &tmp_str));
  m_name = tmp_str;
  free(tmp_str);

  // store long name
  throw_errno(snd_card_get_longname(m_number, &tmp_str));
  m_longname = tmp_str;
  free(tmp_str);
  tmp_str = nullptr;

  // iterate through all the mixer elements
  snd_mixer_elem_t* mixer_pos = snd_mixer_first_elem(m_mixer);
  for(unsigned int i = snd_mixer_get_count(m_mixer); i--; mixer_pos = snd_mixer_elem_next(mixer_pos))
  {
    try
    {
      elements.emplace_back(mixer_pos);
    }
    catch(int err)
    {
      std::cerr << "errno! " << snd_strerror(err) << std::endl;
      std::flush(std::cerr);
    }
  }
}

void MixerInterface::close(void) throw(errno_t)
{
  if(m_is_open)
  {
    throw_errno(snd_mixer_close(m_mixer));
    m_name.clear();
    m_longname.clear();
    m_number = -1;
    m_is_open = false;
  }
}
