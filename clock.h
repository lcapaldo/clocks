#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
HANDLE CreateClock(DWORD dwFrequency);
BOOL StopClock(HANDLE hClock);
#ifdef __cplusplus
}
#endif
