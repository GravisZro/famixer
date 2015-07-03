#ifndef FS_PERMISSIONS_H
#define FS_PERMISSIONS_H

#include <stdint.h>

template<typename A, typename B> constexpr A& to (B& b) { return *reinterpret_cast<A*>(&b); }

enum filetype_e : uint16_t
{
  type_fifo      = 0x1,
  type_chardev   = 0x2,
  type_dir       = 0x4,
  type_blockdev  = 0x6,
  type_file      = 0x8,
  type_link      = 0xA,
  type_socket    = 0xC,

  type_mask      = 0xF,
};

struct permissions_t
{
 uint16_t other_execute  : 1;
 uint16_t other_write    : 1;
 uint16_t other_read     : 1;

 uint16_t group_execute  : 1;
 uint16_t group_write    : 1;
 uint16_t group_read     : 1;

 uint16_t user_execute   : 1;
 uint16_t user_write     : 1;
 uint16_t user_read      : 1;

 uint16_t sticky_bit     : 1;
 uint16_t gid_bit        : 1;
 uint16_t uid_bit        : 1;

 filetype_e type         : 4;

 inline void setRead   (const bool value = true) { other_read    = group_read    = user_read    = (value ? 1 : 0); }
 inline void setWrite  (const bool value = true) { other_write   = group_write   = user_write   = (value ? 1 : 0); }
 inline void setExecute(const bool value = true) { other_execute = group_execute = user_execute = (value ? 1 : 0); }

 permissions_t(void) { to<uint16_t>(*this) = 0; } // start zeroed out
 permissions_t(const bool r,
               const bool w,
               const bool x,
               filetype_e t)
   : other_execute(x),
     other_write  (w),
     other_read   (r),
     group_execute(x),
     group_write  (w),
     group_read   (r),
     user_execute (x),
     user_write   (w),
     user_read    (r),
     sticky_bit   (0),
     gid_bit      (0),
     uid_bit      (0),
     type         (t) { }
};

static_assert(sizeof(permissions_t) == sizeof(uint16_t), "bad size: struct permissions_t");

#endif // FS_PERMISSIONS_H
