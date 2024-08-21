//
// Created by jiaming.huang on 2024/5/20.
//

#ifndef JMPLAYER_XMSGQUEUE_H
#define JMPLAYER_XMSGQUEUE_H

//#include  <SDL_mutex.h>

#include "JMLog.h"
#include <mutex>

static std::mutex mux;

typedef  struct  AVMessage {
    int what;
    int arg1;
    int arg2;
    struct AVMessage *next;

}AVMessage;

typedef  struct Node {
    AVMessage *data;
    struct Node * next;
} Node;

inline static int msg_init_msg(AVMessage *data)
{
    memset(data, 0,sizeof(AVMessage));
}

inline static Node* CreateNode(AVMessage *data)
{
    Node * newNode = (Node *)malloc(sizeof(Node));
    if (!newNode) {
        ALOGE("CreateNode is fail !!\n");
        return NULL;
    }

    newNode->data = data;
    newNode->next = NULL;

    return newNode;
}

inline static int msg_queue_put_private(Node ** head, AVMessage *data)
{
    Node * newNode = CreateNode(data);
    if (!*head) {
        *head = newNode;
        return -1;
    }

    Node *temp = *head;
    while (temp->next) {
        temp = temp->next;
    }
    temp->next = newNode;

    return 0;
}

inline static int msg_queue_put(Node ** head, AVMessage *data)
{
    mux.lock();
    msg_queue_put_private( head, data);
    mux.unlock();

    return 0;
}

inline static int msg_queue_put_simple(Node ** head, int what, int arg1,int arg2)
{
    AVMessage msg;
    msg_init_msg(&msg);
    msg.what = what;
    msg.arg1 = arg1;
    msg.arg2 = arg2;
    msg_queue_put(head, &msg);
}

inline static int msg_queue_get(Node ** head, AVMessage *data)
{
    mux.lock();
    ALOGE("msg_queue_get enter !!\n");
    if (*head == NULL) {
        mux.unlock();
        ALOGE("msg_queue_get is fail !!\n");
        return -1;
    }

    data = (*head)->data;

    Node *temp = *head;
    *head = (*head)->next;
    free(temp);

    mux.unlock();

    return 0;
}

inline static void freeList(Node * head)
{
    Node * temp;
    while(head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}






#endif //JMPLAYER_XMSGQUEUE_H
