/**
 * Copyright (C) 2024 wwhai
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef QUEUE_H
#define QUEUE_H

#include <libavformat/avformat.h>

#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    AVFrame *frame;
} QueueData;

// Forward declare the QueueNode struct
typedef struct QueueNode
{
    QueueData data;
    struct QueueNode *next;
} QueueNode;

typedef struct
{
    QueueNode *front;
    QueueNode *rear;
    int size;
} Queue;

// Function to create a new queue
Queue *NewQueue()
{
    Queue *q = (Queue *)malloc(sizeof(Queue));
    if (q == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for queue\n");
        return NULL;
    }
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
    return q;
}

// Function to create a new queue node
QueueNode *createQueueNode(QueueData data)
{
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    if (node == NULL)
    {
        fprintf(stderr, "Failed to allocate memory for queue node\n");
        return NULL;
    }
    node->data = data;
    node->next = NULL;
    return node;
}

// Function to enqueue data into the queue
void enqueue(Queue *q, QueueData data)
{
    QueueNode *node = createQueueNode(data);
    if (node == NULL)
    {
        return;
    }

    if (q->rear == NULL)
    {
        // Queue is empty
        q->front = q->rear = node;
    }
    else
    {
        // Add the new node at the end of the queue and update the rear
        q->rear->next = node;
        q->rear = node;
    }
    q->size++;
}

// Function to dequeue data from the queue
QueueData dequeue(Queue *q)
{
    QueueData data = {NULL};

    if (q->front == NULL)
    {
        fprintf(stderr, "Queue underflow\n");
        return data;
    }

    QueueNode *temp = q->front;
    data = temp->data;
    q->front = q->front->next;

    if (q->front == NULL)
    {
        q->rear = NULL;
    }

    free(temp);
    q->size--;
    return data;
}

// Function to check if the queue is empty
int isQueueEmpty(Queue *q)
{
    return q->front == NULL;
}

// Function to get the size of the queue
int queueSize(Queue *q)
{
    return q->size;
}

// Function to clear the queue and free memory
void clearQueue(Queue *q)
{
    while (!isQueueEmpty(q))
    {
        QueueData data = dequeue(q);
        // If using AVFrame, make sure to free the frame
        if (data.frame)
        {
            av_frame_free(&data.frame);
        }
    }
    free(q);
}

#endif
