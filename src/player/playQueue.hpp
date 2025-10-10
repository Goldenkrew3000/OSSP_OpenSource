#ifndef _PLAYQUEUE_H
#define _PLAYQUEUE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int internal_OSSPQ_AppendToEnd(char* id);
char* internal_OSSPQ_PopFromFront();
int internal_OSSPQ_GetItemCount();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _PLAYQUEUE_H
