#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mimalloc.h>
#if defined(_XBOX)
#include <errno.h>
#include <malloc.h>
#include <stdint.h>
#endif
// #include <mimalloc-override.h>

#if defined(_XBOX)
static int saw_stats;

static void mi_cdecl capture_stats(const char* message, void* arg) {
  (void)arg;
  if (strstr(message, "subproc") != NULL) saw_stats = 1;
}
#endif

int main() {
  mi_version();       // ensure mimalloc library is linked
  void* p1 = malloc(78);
  #if !defined(_XBOX)
  _expand(p1, 100);
  #endif
  if (!mi_is_in_heap_region(p1)) {
     printf("p1: malloc failed to allocate in heap region\n");
     return 1;
  }

  #if defined(_XBOX)
  if (malloc_usable_size(p1) < 78) {
     printf("p1: malloc_usable_size returned an invalid size\n");
     return 1;
  }
  errno = 0;
  struct mallinfo info = mallinfo();
  if (errno != ENOTSUP || info.arena != SIZE_MAX || info.ordblks != SIZE_MAX ||
      info.smblks != SIZE_MAX || info.hblks != SIZE_MAX || info.hblkhd != SIZE_MAX ||
      info.usmblks != SIZE_MAX || info.fsmblks != SIZE_MAX || info.uordblks != SIZE_MAX ||
      info.fordblks != SIZE_MAX || info.keepcost != SIZE_MAX) {
     printf("mallinfo did not report unsupported\n");
     return 1;
  }
  mi_register_output(&capture_stats, NULL);
  saw_stats = 0;
  malloc_stats();
  if (!saw_stats) {
     printf("malloc_stats did not print mimalloc statistics\n");
     return 1;
  }
  #endif

  void* p2 = malloc(24);
  if (!mi_is_in_heap_region(p2)) {
     printf("p2: malloc failed to allocate in heap region\n");
     return 1;
  }
  free(p1);
  p1 = malloc(8);
  char* s = strdup("hello\n");
  free(p2);
  p2 = malloc(16);
  void* p3 = realloc(p1, 32); if (p3!=NULL) { p1 = p3; }
  free(p1);
  free(p2);
  free(s);
  //mi_collect(true);

  /* now test if override worked by allocating/freeing across the api's*/
  p1 = mi_malloc(32);
  free(p1);
  p2 = malloc(32);
  mi_free(p2);

  //p1 = malloc(24);
  //p2 = reallocarray(p1, 16, 16);
  //free(p2);
  //p1 = malloc(24);
  //assert(reallocarr(&p1, 16, 16) == 0);
  //free(p1);

  mi_stats_print(NULL);
  return 0;
}
