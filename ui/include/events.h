#ifndef _EVENTS_H_
#define _EVENTS_H_

int ev_init(void);
void ev_exit(void);
int ev_get(struct input_event *ev, unsigned dont_wait);

#endif
