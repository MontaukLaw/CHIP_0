#ifndef _MAIN_TASK_H_
#define _MAIN_TASK_H_

#include <stdio.h>

#define MAIN_TASK_LOG_ENABLED 1

#if MAIN_TASK_LOG_ENABLED
#define MAIN_TASK_LOG(...) ((void)printf("[MAIN_TASK] " __VA_ARGS__))
#else
#define MAIN_TASK_LOG(...) ((void)0)
#endif

void main_task(void);

void main_task_sync_test(void);

void uart1_tx_test(void);

void uart1_tx_test_without_cdc(void);

#endif /* _MAIN_TASK_H_ */
