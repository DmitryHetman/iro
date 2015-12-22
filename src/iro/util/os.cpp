#include <iro/util/os.hpp>

#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>

#include <string>

namespace iro
{

namespace
{

#if !HAVE_MKOSTEMP
int set_cloexec_or_close(int fd)
{
   if (fd == -1)
      return -1;

   long flags;
   if ((flags = fcntl(fd, F_GETFD)) == -1)
      goto err;

   if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
      goto err;

   return fd;

err:
   close(fd);
   return -1;
}
#endif

int create_tmpfile_cloexec(const char *tmpname)
{
   int fd;

#if HAVE_MKOSTEMP
   if ((fd = mkostemp(tmpname, O_CLOEXEC)) >= 0)
      unlink(tmpname);
#else
   if ((fd = mkstemp(const_cast<char*>(tmpname))) >= 0) {
      fd = set_cloexec_or_close(fd);
      unlink(tmpname);
   }
#endif

   return fd;
}

}

int os_create_anonymous_file(std::size_t size)
{
   static const char tmplate[] = "/wlc-shared-XXXXXX";

   std::string path = getenv("XDG_RUNTIME_DIR");
   if (path.empty())
      return -1;

	std::string name = path;
	if(path.back() != '/') name.append("/");
	name.append(tmplate);

   int fd = create_tmpfile_cloexec(name.c_str());

   if (fd < 0)
      return -1;

   int ret;
#if HAVE_POSIX_FALLOCATE
   if ((ret = posix_fallocate(fd, 0, size)) != 0) {
      close(fd);
      errno = ret;
      return -1;
   }
#else
   if ((ret = ftruncate(fd, size)) < 0) {
      close(fd);
      return -1;
   }
#endif

   return fd;
}

}
