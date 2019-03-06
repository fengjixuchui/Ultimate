#ifndef __BACKTRACE_H__
#define __BACKTRACE_H__

#include "ptrace.h"

void backtrace(struct pt_regs * regs);

#endif