#ifndef __CQUEUE_H__
#define __CQUEUE_H__

struct ArrayQueue
{
        int front, rear;
        int capacity;
        unsigned char *array;
};

extern struct ArrayQueue* Queue(int size);
extern int IsEmptyQueue(struct ArrayQueue *Q);
extern int IsFullQueue(struct ArrayQueue *Q);
extern int QueueSize(struct ArrayQueue *Q);
extern void ResizeQueue(struct ArrayQueue *Q);
extern void EnQueue(struct ArrayQueue *Q, unsigned char data);
extern unsigned char DeQueue(struct ArrayQueue *Q);
extern void DeleteQueue(struct ArrayQueue *Q);

#endif	/* __CQUEUE_H__ */
