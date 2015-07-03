#ifndef FUSE_MIXER_H
#define FUSE_MIXER_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>

#define FUSE_USE_VERSION 30
#include <fuse.h>

#include <stdint.h>
#include <linux/limits.h>
#include "errno_t.h"

#include <alsapp/fs_permissions.h>

#include "mixer_interface.h"


static_assert(sizeof(errno_t) == sizeof(int), "incorrect size for \"errno_t\"");

class FuseMixer
{
public:
  // stock
  FuseMixer(void);
  operator struct fuse_operations(void) const;

  static errno_t GetAttr     (const char* path, struct stat* statbuf);
  static errno_t ReadLink    (const char* path, char* link, size_t size);
  static errno_t MkNod       (const char* path, mode_t mode, dev_t dev);
  static errno_t MkDir       (const char* path, mode_t mode);
  static errno_t Unlink      (const char* path);
  static errno_t RmDir       (const char* path);
  static errno_t SymLink     (const char* path, const char* link);
  static errno_t Rename      (const char* path, const char* newpath);
  static errno_t Link        (const char* path, const char* newpath);
  static errno_t Chmod       (const char* path, mode_t mode);
  static errno_t Chown       (const char* path, uid_t uid, gid_t gid);
  static errno_t Truncate    (const char* path, off_t newSize);
  static errno_t Open        (const char* path, struct fuse_file_info* fileInfo);
  static errno_t Read        (const char* path,       char* buf, size_t size, off_t offset, struct fuse_file_info* fileInfo);
  static errno_t Write       (const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fileInfo);
  static errno_t StatFS      (const char* path, struct statvfs* statInfo);
  static errno_t Flush       (const char* path, struct fuse_file_info* fileInfo);
  static errno_t Release     (const char* path, struct fuse_file_info* fileInfo);
  static errno_t Fsync       (const char* path, int datasync, struct fuse_file_info* fi);
  static errno_t SetXAttr    (const char* path, const char* name, const char* value, size_t size, int flags);
  static errno_t GetXAttr    (const char* path, const char* name, char* value, size_t size);
  static errno_t ListXAttr   (const char* path, char* list, size_t size);
  static errno_t RemoveXAttr (const char* path, const char* name);
  static errno_t OpenDir     (const char* path, struct fuse_file_info* fileInfo);
  static errno_t ReadDir     (const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fileInfo);
  static errno_t ReleaseDir  (const char* path, struct fuse_file_info* fileInfo);
  static errno_t FsyncDir    (const char* path, int datasync, struct fuse_file_info* fileInfo);
  static void*   Init        (struct fuse_conn_info* conn);
  static errno_t Access      (const char* path, int mask);
  static errno_t Create      (const char* path, mode_t mode, struct fuse_file_info* fileInfo);
  static errno_t FTruncate   (const char* path, off_t offset, struct fuse_file_info* fileInfo);
  static errno_t FGetAttr    (const char* path, struct stat* statInfo, struct fuse_file_info* fileInfo);
  static errno_t Lock        (const char* path, struct fuse_file_info* fileInfo, int cmd, struct flock* locks);
  static errno_t UTimeNS     (const char* path, const struct timespec tv[2]);
  static errno_t BMap        (const char* path, size_t blocksize, uint64_t* idx);
  static errno_t IoCtl       (const char* path, int cmd, void* arg, struct fuse_file_info* fileInfo, unsigned int flags, void* data);
  static errno_t Poll        (const char* path, struct fuse_file_info* fileInfo, struct fuse_pollhandle* ph, unsigned* reventsp);
  static errno_t WriteBuf    (const char* path, struct fuse_bufvec* buf, off_t offset, struct fuse_file_info* fileInfo);
  static errno_t ReadBuf     (const char* path, struct fuse_bufvec** bufp, size_t size, off_t offset, struct fuse_file_info* fileInfo);
  static errno_t FLock       (const char* path, struct fuse_file_info* fileInfo, int operation);
  static errno_t FAllocate   (const char* path, int mode, off_t offset, off_t length, struct fuse_file_info* fileInfo);

private:
  class fs_element_t;
  typedef std::shared_ptr<fs_element_t> fs_element_ptr;

  static fs_element_ptr find_child(const char* path) throw(errno_t);
  static fs_element_ptr m_root;
  static std::vector<MixerInterface> m_cards;
};


#endif // FUSE_MIXER_H
