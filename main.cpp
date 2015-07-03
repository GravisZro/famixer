#include "fuse_mixer.h"

int main(int argc, char *argv[])
{
  FuseMixer iface;
  fuse_operations ops = iface;
  return fuse_main_real(argc, argv, &ops, sizeof(ops), nullptr);
//return 0;
}
