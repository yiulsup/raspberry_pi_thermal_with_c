#include <stdio.h>
#include <stdlib.h>

#include "cqueue.h"

struct ArrayQueue* Queue(int size);
int IsEmptyQueue(struct ArrayQueue *Q);
int IsFullQueue(struct ArrayQueue *Q);
int QueueSize(struct ArrayQueue *Q);
void ResizeQueue(struct ArrayQueue *Q);
void EnQueue(struct ArrayQueue *Q, unsigned char data);
unsigned char DeQueue(struct ArrayQueue *Q);
void DeleteQueue(struct ArrayQueue *Q);

struct ArrayQueue* Queue(int size)
{
	struct ArrayQueue *Q = (struct ArrayQueue *)malloc(sizeof(struct ArrayQueue));
	if(!Q) return NULL;

	Q->capacity = size;
	Q->front = -1;
	Q->rear = -1;
	Q->array = (unsigned char *)malloc(Q->capacity * sizeof(unsigned char));
	if(!Q->array) return NULL;
	return Q;
}

int IsEmptyQueue(struct ArrayQueue *Q)
{
	return (Q->front == -1);
}

int IsFullQueue(struct ArrayQueue *Q)
{
	return ((Q->rear+1)%Q->capacity==Q->front);
}

int QueueSize(struct ArrayQueue *Q)
{
	return (Q->capacity - (Q->front - Q->rear)+1) % Q->capacity;
}

void ResizeQueue(struct ArrayQueue *Q)
{
	int size = Q->capacity;
	Q->capacity = Q->capacity*2;
	Q->array = (unsigned char *)realloc(Q->array, Q->capacity);
	if(!Q->array)
	{
		printf("Memory Error");
		return;
	}
	if(Q->front > Q->rear)
	{
		for(int i = 0; i < Q->front; i++)
		{
			Q->array[i+size] = Q->array[i];
		}
		Q->rear = Q->rear+size;
	}
}

void EnQueue(struct ArrayQueue *Q, unsigned char data)
{
	if(IsFullQueue(Q))
	{
		printf("resized\n");
		ResizeQueue(Q);
	}

	Q->rear = (Q->rear+1) % Q->capacity;
	Q->array[Q->rear] = data;
	if(IsEmptyQueue(Q))
	{
		Q->front = Q->rear;
	}
}

unsigned char DeQueue(struct ArrayQueue *Q)
{
	unsigned char data = 0;
	if(IsEmptyQueue(Q))
	{
		//printf("Queue is Empty\n");
		return 0;
	}
	else
	{
		data = Q->array[Q->front];
		if(Q->front == Q->rear)
			Q->front = Q->rear = -1;
		else
		{
			Q->front = (Q->front+1) % Q->capacity;
		}
	}
	return data;
}

void DeleteQueue(struct ArrayQueue *Q)
{
	if(Q)
	{
		if(Q->array)
			free(Q->array);
		free(Q);
	}
}

#if 0
int main(int argc, const char *argv[])
{
	struct ArrayQueue *Q = Queue(10000);
	EnQueue(Q, 1);
	EnQueue(Q, 2);
	EnQueue(Q, 3);
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	EnQueue(Q, 4);
	EnQueue(Q, 5);
	EnQueue(Q, 6);
	EnQueue(Q, 7);
	EnQueue(Q, 8);
	EnQueue(Q, 9);
	EnQueue(Q, 0);

	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));
	printf("%d\n", DeQueue(Q));

	return 0;
}
#endif
