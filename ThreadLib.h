#include <SYS/TYPES.H>
#include <KERNEL.H>

void InitSubThread(void (*threadfun)());
void ReturnToMainThread();
void RunSubThread();
void StopSubThread();
