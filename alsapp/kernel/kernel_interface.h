#ifndef DEVICE_IO_H
#define DEVICE_IO_H

#include <errno.h>
#include <array>

//#include <sound/asound.h>

#include <linux/types.h>
#include <sys/ioctl.h>


namespace Alsa
{
  template<typename ver_t> constexpr int build_protocol_version(ver_t major, ver_t minor, ver_t micro)
                                          { return (major << 16) | (minor << 8) | micro; }

  template<typename ver_t> constexpr ver_t protocol_major  (ver_t version) { return (version >> 16) & 0xFFFF; }
  template<typename ver_t> constexpr ver_t protocol_minor  (ver_t version) { return (version >>  8) & 0x00FF; }
  template<typename ver_t> constexpr ver_t protocol_micro  (ver_t version) { return (version      ) & 0x00FF; }

  template<typename ver_t> constexpr bool protocol_incompatible(ver_t kversion, ver_t uversion)
  {
    return protocol_major(kversion) != protocol_major(uversion) ||
           protocol_minor(kversion) != protocol_minor(uversion);
  }

  class KernelInterface
  {
  public:
    virtual const char* device_filename(void) const = 0;
    virtual int protocol_version_requirement ( void ) const = 0;

    virtual void protocol_version ( int& value ) = 0;


    template<typename data_type>
    inline void io_r(const __s8 cmd_type, const __u8 cmd, data_type& data) const throw(error_t)
    {
      if(ioctl(device_handle, _IOR(cmd_type, cmd, data_type), &data) < 0)
        throw(errno);
    }

    template<typename data_type>
    inline void io_w(const __s8 cmd_type, const __u8 cmd, const data_type& data) const throw(error_t)
    {
      if(ioctl(device_handle, _IOW(cmd_type, cmd, data_type), &data) < 0)
        throw(errno);
    }

    template<typename data_type>
    inline void io_rw(const __s8 cmd_type, const __u8 cmd, data_type& data) const throw(error_t)
    {
      if(ioctl(device_handle, _IOWR(cmd_type, cmd, data_type), &data) < 0)
        throw(errno);
    }

    inline void io(const __s8 cmd_type, const __u8 cmd) const throw(error_t)
    {
      if(ioctl(device_handle, _IO(cmd_type, cmd)) < 0)
        throw(errno);
    }

    int device_handle;
  };
}

#endif // DEVICE_IO_H
