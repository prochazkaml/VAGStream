#include "ThreadLib.h"

struct ToT *sysToT = (struct ToT *)0x100; // Table of tables
struct TCBH *tcbh; // Task status queue address
struct TCB *master_thp, *sub_thp; // Thread contexts

unsigned long sub_th, gp;

void InitSubThread(void (*threadfun)()) {
	tcbh = (struct TCBH *) sysToT[1].head;
	master_thp = tcbh->entry;

	gp = GetGp();
	EnterCriticalSection();
	sub_th = OpenTh(threadfun, 0x801EFFF0, gp);	// 64k below the main stack, should be fine
	ExitCriticalSection();

	sub_thp = (struct TCB *)sysToT[2].head + (sub_th & 0xffff);
	sub_thp->reg[R_SR] = 0x404;
}

void ReturnToMainThread() {
	tcbh->entry = master_thp;
}

void RunSubThread() {
	ChangeTh(sub_th);
}

void StopSubThread() {
	EnterCriticalSection();
	CloseTh(sub_th);
	ExitCriticalSection();
}
