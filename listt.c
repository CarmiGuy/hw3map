//
// Created by Maya on 17/05/2018.
//

#include "listt.h"
#include <stdbool.h>
#include <stdlib.h>

#define INVALID_VAL -1
typedef struct node_t* Node;
struct node_t {
    int n;
    Node next;
};

typedef struct list_t* List;
struct list_t
{
    Node firstNode;
};


int ChangeArrVariable(int Val)
{
    static int oldVal = 0;
    if (Val > 0)
    {
        oldVal = Val;
    }
    return oldVal;
}

bool CalcCoolNumbers(int Value)
{
    static int arrPlace = 0;
    int newPlace = ChangeArrVariable(INVALID_VAL);
    if (newPlace != arrPlace)
    {
        arrPlace = newPlace;
    }
    if (!(Value%(arrPlace%3) + 1 ))
    {
        return true;
    }
}

List ListFilter(void (*func_ptr)(int), List oldList)
{
    List newList = malloc(sizeof(List));
    if (newList == NULL || oldList == NULL)
    {
        free(newList);
        return NULL;
    }
    int newElemNumber = 0;
    Node node = oldList->firstNode;
    Node newNode = NULL;
    while (node != NULL)
    {
        if ((*func_ptr)(node->n))
        {

            Node addNode = malloc(sizeof(Node));
            if (addNode == NULL)
            {
                destroyList(newList->firstNode);
                free(newList);
                return NULL;
            }
            if (!newElemNumber)
            {
                newList->firstNode = addNode;
                newNode = newList->firstNode;
            }
            else
            {
                newNode->next = addNode;
                newNode = newNode->next;
            }
            newNode->n = node->n;
            newNode->next = NULL;
            newElemNumber++;
        }
        node = node->next;
    }
    return newList;
}

List CoolElements(List list[], int SizeOfArr)
{
    int listIndex = 0;
    void (*func_pointer)(int);
    func_pointer = &CalcCoolNumbers;
    List newList = ListFilter(func_pointer, list[0]);
    for (int i = 1; i < SizeOfArr; i++)
    {
        Node tempNode = newList->firstNode;
        while (tempNode->next != NULL)
        {
            tempNode = tempNode->next;
        }
        ChangeArrVariable(i);
        List tempList = ListFilter(func_pointer, list[i]);
        tempNode->next = tempList->firstNode;
        free(tempList);
    }
    return newList;
}

/**
* mergeSort - sorts an array of ADT
*
* @param greaterThen - Function pointer to be used for comparing the elements, returning true if element in place
* j is bigger then the following element
* @param swapElem - Function pointer to be used for swapping the elements in the array
* @param arr - array of ADT
* @param size - the size of the array.
*/
void mergeSort(void** arr, int size , bool (*greaterThen)(void*)(void*), void (*swapElem)(void*)(void*))
{
    for (int i =0; i< size-1; i++)
    {
        for (int j =0; j< size-1-i; j++)
        {
            if (greaterThen((arr[j], arr[j+1])))
            {
                swapElem((arr[j], arr[j+1]));
            }
        }
    }
}


