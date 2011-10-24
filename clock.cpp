#include "clock.h"
#include <map>
#include <memory>

namespace {
struct ClockRecord
{
  bool go;
  DWORD dwFrequency;
  HANDLE clock;
  HANDLE hThread;
};
struct ClockTable {
  CRITICAL_SECTION lck_clocks;
  std::map<intptr_t, ClockRecord*> s_clocks;

  ClockTable() 
  {
    InitializeCriticalSection(&lck_clocks);
  }

  ~ClockTable()
  {
    DeleteCriticalSection(&lck_clocks);
  }
} g_clocks;

DWORD WINAPI SleeperProc(LPVOID p)
{
  std::auto_ptr<ClockRecord> r((ClockRecord*)p);
  while(r->go)
  {
    SleepEx(r->dwFrequency, TRUE);
    if(r->go)
      SetEvent(r->clock);
  }
  return 0;
}

void CALLBACK WakerProc(ULONG_PTR p)
{
  ((ClockRecord*)p)->go = false;
}

}
HANDLE CreateClock(DWORD dwFrequency)
{
  std::auto_ptr<ClockRecord> r( new ClockRecord() );
  r->clock = CreateEvent(NULL, FALSE, FALSE, NULL);
  if(!r->clock)
    return NULL;
  r->go = true;
  r->dwFrequency = dwFrequency; 
  r->hThread = CreateThread(NULL, 0, &SleeperProc, r.get(), 0, NULL);  
  if( r->hThread ) {
    EnterCriticalSection(&g_clocks.lck_clocks);
    g_clocks.s_clocks[reinterpret_cast<intptr_t>(r->clock)] = r.get();
    LeaveCriticalSection(&g_clocks.lck_clocks);
    return r.release()->clock;
  }
  return NULL;
}
 
BOOL StopClock(HANDLE clock)
{
  ClockRecord *r = NULL;
  EnterCriticalSection(&g_clocks.lck_clocks);
  std::map<intptr_t, ClockRecord*>::iterator i = g_clocks.s_clocks.find(reinterpret_cast<intptr_t>(clock));
  if(i != g_clocks.s_clocks.end() )
   {
     r = i->second;
     g_clocks.s_clocks.erase(i);
   }
   LeaveCriticalSection(&g_clocks.lck_clocks);
   if( r )
   {
     HANDLE thread = r->hThread;
     QueueUserAPC(WakerProc, thread, (ULONG_PTR)r);
     WaitForSingleObject(thread, INFINITE);
     CloseHandle(thread);
     CloseHandle(clock);
     return TRUE;
   }
   return FALSE;
}

    
   



