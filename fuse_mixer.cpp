#include "fuse_mixer.h"

#include <iostream>
#include <sstream>
#include <streambuf>

#include <sys/types.h>
#include <fstream>

#define HAVE_SETXATTR
#ifdef HAVE_SETXATTR
 #include <sys/xattr.h>
#endif

//static std::ofstream log("/home/gravis/famixer.log");


// cleans up using unordered_map::emplace with a new shared_ptr
template<typename T> struct mapped : std::unordered_map<std::string, std::shared_ptr<T>>
{
  inline std::shared_ptr<T>& easy_emplace(const std::string str, const permissions_t& _permissions, MixerElement* _mixer_element, MixerInterface* _mixer_interface)
    { return std::unordered_map<std::string, std::shared_ptr<T>>::emplace(str, std::shared_ptr<T>(new T(_permissions, _mixer_element, _mixer_interface))).first->second; }
};

struct FuseMixer::fs_element_t
{
  permissions_t   permissions;
  MixerElement*   mixer_element;
  MixerInterface* mixer_interface;
  mapped<fs_element_t> children;

  fs_element_t(void) { }
  fs_element_t(permissions_t   _permissions,
               MixerElement*   _mixer_element,
               MixerInterface* _mixer_interface)
    : permissions(_permissions),
      mixer_element(_mixer_element),
      mixer_interface(_mixer_interface) { }

//  ~fs_element_t(void) { for(auto pos = children.begin(); pos != children.end(); ++pos) { std::cout << "destroy> " << pos->first << std::endl; } }
};

FuseMixer::fs_element_ptr FuseMixer::find_child(const char* path) throw(errno_t)
{
  fs_element_ptr fs_pos = m_root;
  for(const char* end = ++path; *path; path = end)
  {
    while(*end && *end != '/')
      ++end;
    auto new_fs_pos = fs_pos->children.find(std::string(path, end - path));
    if(new_fs_pos == fs_pos->children.end())
      throw ERRNO_NOENT;
    fs_pos = new_fs_pos->second;
    if(*end == '/')
      ++end;
  }
  return fs_pos;
}

FuseMixer::fs_element_ptr FuseMixer::m_root (new FuseMixer::fs_element_t());
std::vector<MixerInterface> FuseMixer::m_cards;

// arbitrary assignment function
template<class A, class B> inline void assign(A& a, B& b) { a = reinterpret_cast<A>(b); }

// arbitrary conversion function
template<class A, class B> inline A& convert(A& a, B& b) { return reinterpret_cast<A>(b); }

FuseMixer::operator struct fuse_operations(void) const
{
  static struct fuse_operations FUSE_struct;

  // deprecated functions
  FUSE_struct.getdir = nullptr;
  FUSE_struct.utime  = nullptr;

  // copy to C struct
  assign( FUSE_struct.getattr    , GetAttr     );
/*
  assign( FUSE_struct.readlink   , ReadLink    );
  assign( FUSE_struct.mknod      , MkNod       );
  assign( FUSE_struct.mkdir      , MkDir       );
  assign( FUSE_struct.unlink     , Unlink      );
  assign( FUSE_struct.rmdir      , RmDir       );
  assign( FUSE_struct.symlink    , SymLink     );
  assign( FUSE_struct.rename     , Rename      );
  assign( FUSE_struct.link       , Link        );
*/
  //assign( FUSE_struct.chmod      , Chmod       );
  //assign( FUSE_struct.chown      , Chown       );
  assign( FUSE_struct.truncate   , Truncate    );

  //assign( FUSE_struct.open       , Open        );

  assign( FUSE_struct.read       , Read        );
  assign( FUSE_struct.write      , Write       );
/*
  assign( FUSE_struct.statfs     , StatFS      );
  assign( FUSE_struct.flush      , Flush       );
  assign( FUSE_struct.release    , Release     );
  assign( FUSE_struct.fsync      , Fsync       );
  assign( FUSE_struct.setxattr   , SetXAttr    );
*/
  assign( FUSE_struct.getxattr   , GetXAttr    );
  assign( FUSE_struct.listxattr  , ListXAttr   );
/*
  assign( FUSE_struct.removexattr, RemoveXAttr );
  assign( FUSE_struct.opendir    , OpenDir     );
*/
  assign( FUSE_struct.readdir    , ReadDir     );
/*
  assign( FUSE_struct.releasedir , ReleaseDir  );
  assign( FUSE_struct.fsyncdir   , FsyncDir    );
  assign( FUSE_struct.init       , Init        );

  assign( FUSE_struct.access     , Access      );
  assign( FUSE_struct.create     , Create      );

  assign( FUSE_struct.ftruncate  , FTruncate   );
  assign( FUSE_struct.fgetattr   , FGetAttr    );
  assign( FUSE_struct.lock       , Lock        );
  assign( FUSE_struct.utimens    , UTimeNS     );
  assign( FUSE_struct.bmap       , BMap        );
  assign( FUSE_struct.ioctl      , IoCtl       );
  assign( FUSE_struct.poll       , Poll        );

  assign( FUSE_struct.write_buf  , WriteBuf    );
  assign( FUSE_struct.read_buf   , ReadBuf     );

  assign( FUSE_struct.flock      , FLock       );
  assign( FUSE_struct.fallocate  , FAllocate   );
*/
  return FUSE_struct;
}

errno_t FuseMixer::GetXAttr(const char* path, const char* name, char* value, size_t size)
{
  int res = lgetxattr(path, name, value, size);
  if (res == -1)
          return (errno_t)-errno;
  return (errno_t)res;

  if(std::string(name) == "exec")
  {
    strcpy(value, "/usr/bin/X;/bin/less");
    return (errno_t)sizeof("/usr/bin/X;/bin/less");
  }
  return ERRNO_NOERROR;
}

errno_t FuseMixer::ListXAttr(const char* path, char* list, size_t size)
{
  int res = llistxattr(path, list, size);
  if (res == -1)
          return (errno_t)-errno;
  return (errno_t)res;

  strcpy(list, "system.posix_acl_access\0");
  return (errno_t)1;

  return ERRNO_NOERROR;
}


FuseMixer::FuseMixer(void)
{  
  //assert(log.is_open());

  std::cout << std::endl << "starting!" << std::endl;
  std::flush(std::cout);

  int card_num = -1;

  try
  {
    for(throw_errno(snd_card_next(&card_num));
            card_num != -1;
            throw_errno(snd_card_next(&card_num)))
    {
      m_cards.emplace_back(card_num);
    }
  }
  catch(int err)
  {
    std::cerr << "errno! " << snd_strerror(err) << std::endl;
    std::flush(std::cerr);
  }

  permissions_t chardev_permissions;
  chardev_permissions.type = type_file;
  chardev_permissions.setRead(true);
  chardev_permissions.setWrite(true);
  chardev_permissions.setExecute(false);

  permissions_t dir_permissions;
  dir_permissions.type = type_dir;
  dir_permissions.setRead(true);
  dir_permissions.setWrite(false);
  dir_permissions.setExecute(true);

  m_root->permissions = dir_permissions;
  mapped<fs_element_t>& fs_children = m_root->children;

  for(std::vector<MixerInterface>::iterator mixer_interface = m_cards.begin(); mixer_interface != m_cards.end(); ++mixer_interface)
  {
    fs_element_ptr& fs_mixer_elements = fs_children.easy_emplace(mixer_interface->getDeviceName(), dir_permissions, nullptr, &*mixer_interface);

    fs_element_ptr outputs  (new fs_element_t(dir_permissions, nullptr, &*mixer_interface));
    fs_element_ptr inputs   (new fs_element_t(dir_permissions, nullptr, &*mixer_interface));
    fs_element_ptr switches (new fs_element_t(dir_permissions, nullptr, &*mixer_interface));

    for(std::list<MixerElement>::iterator mixer_element = mixer_interface->elements.begin(); mixer_element != mixer_interface->elements.end(); ++mixer_element)
    {
      switch(mixer_element->type)
      {
        case MixerElement::common:
        case MixerElement::playback:
          outputs ->children.easy_emplace(mixer_element->device_name, chardev_permissions, &*mixer_element, &*mixer_interface);
          break;
        case MixerElement::capture:
          inputs  ->children.easy_emplace(mixer_element->device_name, chardev_permissions, &*mixer_element, &*mixer_interface);
          break;
        case MixerElement::toggle:
          switches->children.easy_emplace(mixer_element->device_name, chardev_permissions, &*mixer_element, &*mixer_interface);
          break;
        default:
          assert(false); // invalid type! :((((
      }
    }

    if(!outputs->children.empty())
      fs_mixer_elements->children.emplace("output", outputs);
    if(!inputs->children.empty())
      fs_mixer_elements->children.emplace("input" , inputs);
    if(!switches->children.empty())
      fs_mixer_elements->children.emplace("switch", switches);
  }
}


errno_t FuseMixer::GetAttr(const char* path,
                           struct stat* statbuf)
{
  try
  {
    fs_element_ptr fs_pos = find_child(path); // find_child() may throw
    to<permissions_t>(statbuf->st_mode) = fs_pos->permissions;

    if(fs_pos->mixer_element)
      statbuf->st_size = fs_pos->mixer_element->string().size();
  }
  catch(errno_t err)
  {
    return err;
  }

  return ERRNO_NOERROR;
}

errno_t FuseMixer::ReadDir(const char* path,
                           void* buf,
                           fuse_fill_dir_t filler,
                           off_t offset,
                           struct fuse_file_info* fileInfo)
{
  filler(buf, "." , nullptr, 0);
  filler(buf, "..", nullptr, 0);

  try
  {
    fs_element_ptr fs_pos = find_child(path); // find_child() may throw
    for(auto pos = fs_pos->children.begin(); pos != fs_pos->children.end(); ++pos)
      filler(buf, pos->first.data(), nullptr, offset);
  }
  catch(errno_t err)
  {
    return err;
  }

  return ERRNO_NOERROR;
}


#if 0

      std::cout << "\n\nmixer element: " << pos->name << std::endl;

      std::cout << "  playback   : " << (pos->is_playback ? "yes" : "no") << std::endl;
      std::cout << "  capture    : " << (pos->is_capture  ? "yes" : "no") << std::endl;
      std::cout << "  native mute: " << (pos->native_mute ? "yes" : "no") << std::endl;
      std::cout << "  min volume : " << pos->min << std::endl;
      std::cout << "  max volume : " << pos->max << std::endl;

      std::cout << "  channels   : " << pos->channels.size() << std::endl;

      for(auto channel_pos = pos->channels.begin(); channel_pos != pos->channels.end(); ++channel_pos)
      {
        std::cout << "    channel num : " << channel_pos->first << std::endl;
        std::cout << "    channel name: " << channel_pos->second << std::endl;
        std::cout << "    channel volume: " << pos->get_volume(channel_pos->first) << std::endl;
      }
#endif



errno_t FuseMixer::Read(const char* path,
                        char* buf,
                        size_t size,
                        off_t offset,
                        struct fuse_file_info* fileInfo)
{
  try
  {
    std::ostringstream stream;
    stream.rdbuf()->pubsetbuf(buf, size);

    fs_element_ptr fs_pos = find_child(path); // find_child() may throw
    if(fs_pos->mixer_element)
    {
      stream << fs_pos->mixer_element->string();
      size = stream.str().size();
    }
  }
  catch(errno_t err)
  {
    return err;
  }

  return (errno_t)size;
}

errno_t FuseMixer::Write(const char* path,
                         const char* buf,
                         size_t size,
                         off_t offset,
                         struct fuse_file_info* fileInfo)
{
  try
  {
    fs_element_ptr fs_pos = find_child(path); // find_child() may throw
    if(fs_pos->mixer_element)
    {
      std::stringstream stream;
      snd_mixer_selem_channel_id_t channel = SND_MIXER_SCHN_UNKNOWN;
      long value;

      stream << buf; // buffer input
      stream >> value; // extract numeric value

      if(stream) // couldnt read a number (either NaN or empty stream)
        throw(ERRNO_INVAL); // invalid, bail out!

      if(stream.peek() == '%') // value is a percentage
        value = (fs_pos->mixer_element->max - fs_pos->mixer_element->min) * value / 100;

      if(buf[0] == '+' || buf[0] == '-') // value is relative
        value += fs_pos->mixer_element->get_volume(channel);

      // todo, determine which channel(s) to modify

      fs_pos->mixer_element->set_volume(channel, value); // set value
    }
  }
  catch(errno_t err)
  {
    return err;
  }

  return (errno_t)size;
}

errno_t FuseMixer::Truncate(const char* path, off_t newSize)
{
  return ERRNO_NOERROR;
}

#if 0
errno_t FuseMixer::Open(const char* path,
                        struct fuse_file_info* fileInfo)
{
  try
  {
    fs_element_ptr fs_pos = find_child(path); // find_child() may throw
    fileInfo->fh = 0;
  }
  catch(errno_t err)
  {
    return err;
  }

  return ERRNO_NOERROR;
}

errno_t FuseMixer::Access(const char* path,
                          int mask)
{
  return ERRNO_NOERROR;
}
#endif
