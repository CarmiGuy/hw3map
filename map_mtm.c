#include "map_mtm.h"
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>

#define KEYS_ARE_EQUAL 0

typedef struct Node_t
{
	MapKeyElement key;
	MapDataElement data;
	struct Node_t* next;
}Node;

struct Map_t 
{
	Node* iterator;
	Node* content;
	copyMapDataElements dataCopyF;
	copyMapKeyElements keyCopyF;
	freeMapDataElements dataFreeF;
	freeMapKeyElements keyFreeF;
	compareMapKeyElements compareF;
};

/**
* mapCreate: Allocates a new empty map.
*
* @param copyDataElement - Function pointer to be used for copying data elements into
*  	the map or when copying the map.
* @param copyKeyElement - Function pointer to be used for copying key elements into
*  	the map or when copying the map.
* @param freeDataElement - Function pointer to be used for removing data elements from
* 		the map
* @param freeKeyElement - Function pointer to be used for removing key elements from
* 		the map
* @param compareKeyElements - Function pointer to be used for comparing key elements
* 		inside the map. Used to check if new elements already exist in the map.
* @return
* 	NULL - if one of the parameters is NULL or allocations failed.
* 	A new Map in case of success.
*/
Map mapCreate(copyMapDataElements copyDataElement, copyMapKeyElements copyKeyElement,
	freeMapDataElements freeDataElement, freeMapKeyElements freeKeyElement,
	compareMapKeyElements compareKeyElements)
{
	if (copyDataElement == NULL || copyKeyElement == NULL || freeDataElement == NULL || freeKeyElement == NULL || compareKeyElements == NULL)
	{
		return NULL;
	}
	Map newMap = malloc(sizeof(*newMap));
	if (newMap == NULL)
	{
		return NULL;
	}
	newMap->dataCopyF = copyDataElement;
	newMap->keyCopyF = copyKeyElement;
	newMap->dataFreeF = freeDataElement;
	newMap->keyFreeF = freeKeyElement;
	newMap->compareF = compareKeyElements;
	newMap->content = NULL;
	newMap->iterator = NULL;
	return newMap;
}

/**
* mapDestroy: Deallocates an existing map. Clears all elements by using the
* stored free functions.
*
* @param map - Target map to be deallocated. If map is NULL nothing will be
* 		done
*/
void mapDestroy(Map map)
{
	if (map == NULL)
	{
		return;
	}
	mapClear(map);
	free(map);
}

/**
* mapCopy: Creates a copy of target map.
* Iterator values for both maps is undefined after this operation.
*
* @param map - Target map.
* @return
* 	NULL if a NULL was sent or a memory allocation failed.
* 	A Map containing the same elements as map otherwise.
*/
Map mapCopy(Map map)
{
	Map newMap = malloc(sizeof(*newMap));
	if (map == NULL || newMap == NULL)
	{
		return NULL;
	}
	newMap->dataCopyF = map->dataCopyF;
	newMap->keyCopyF = map->keyCopyF;
	newMap->dataFreeF = map->dataFreeF;
	newMap->keyFreeF = map->keyFreeF;
	newMap->compareF = map->compareF;
	newMap->content = NULL;
	newMap->iterator = NULL;
	MapKeyElement currKey = mapGetFirst(map);
	MapDataElement currData = mapGet(map, currKey);
	while (currKey != NULL)
	{
		if (mapPut(newMap, currKey, currData) == MAP_OUT_OF_MEMORY)
		{
			mapDestroy(newMap);
			return NULL;
		}
		currKey = mapGetNext(map);
		currData = mapGet(map, currKey);
	}
	return newMap;
}

/**
* mapGetSize: Returns the number of elements in a map
* @param map - The map which size is requested
* @return
* 	-1 if a NULL pointer was sent.
* 	Otherwise the number of elements in the map.
*/
int mapGetSize(Map map)
{
	if (map == NULL)
	{
		return -1;
	}
	int count = 0;
	MapKeyElement currElement = mapGetFirst(map);
	while (currElement != NULL)
	{
		currElement = mapGetNext(map);
		count++;
	}
	return count;
}

/**
* mapContains: Checks if a key element exists in the map. The key element will be
* considered in the map if one of the key elements in the map it determined equal
* using the comparison function used to initialize the map.
*
* @param map - The map to search in
* @param element - The element to look for. Will be compared using the
* 		comparison function.
* @return
* 	false - if one or more of the inputs is null, or if the key element was not found.
* 	true - if the key element was found in the map.
*/
bool mapContains(Map map, MapKeyElement element)
{
	if (map == NULL || element == NULL)
	{
		return false;
	}
	MapKeyElement currElement = mapGetFirst(map);
	while (currElement != NULL)
	{
		if (map->compareF(currElement, element) == KEYS_ARE_EQUAL)
		{
			return true;
		}
		currElement = mapGetNext(map);
	}
	return false;
}

/**
*	mapPut: Gives a specified key a specific value.
*  Iterator's value is undefined after this operation.
*
* @param map - The map for which to reassign the data element
* @param keyElement - The key element which need to be reassigned
* @param dataElement - The new data element to associate with the given key.
*      A copy of the element will be inserted as supplied by the copying function
*      which is given at initialization and old data memory would be
*      deleted using the free function given at initialization.
* @return
* 	MAP_NULL_ARGUMENT if a NULL was sent as map
* 	MAP_OUT_OF_MEMORY if an allocation failed (Meaning the function for copying
* 	an element failed)
* 	MAP_SUCCESS the paired elements had been inserted successfully
*/
MapResult mapPut(Map map, MapKeyElement keyElement, MapDataElement dataElement)
{
	if (map == NULL || keyElement == NULL || dataElement == NULL)
	{
		return MAP_NULL_ARGUMENT;
	}
	if (mapGetSize(map) == 0)
	{
		Node* newElement = malloc(sizeof(*newElement));
		if (newElement == NULL)
		{
			return MAP_OUT_OF_MEMORY;
		}
		newElement->key = map->keyCopyF(keyElement);
		newElement->data = map->dataCopyF(dataElement);
		newElement->next = NULL;
		if (newElement->key == NULL || newElement->data == NULL)
		{
			map->keyFreeF(keyElement);
			map->dataFreeF(dataElement);
			free(newElement);
			return MAP_OUT_OF_MEMORY;
		}
		map->content = newElement;
	}
	else if (mapContains(map, keyElement))
	{
		MapKeyElement currElement = mapGetFirst(map);
		while (currElement != NULL)
		{
			if (map->compareF(currElement, keyElement) == KEYS_ARE_EQUAL)
			{
				map->iterator->data = map->dataCopyF(dataElement);
				if (map->iterator->data == NULL)
				{
					return MAP_OUT_OF_MEMORY;
				}
			}
			currElement = mapGetNext(map);
		}
	}
	else
	{
		for (MapKeyElement currElement = mapGetFirst(map); currElement != NULL; currElement = mapGetNext(map))
		{
			if (map->compareF(currElement, keyElement) > 0)
			{
				Node* newElement = malloc(sizeof(*newElement));
				if (newElement == NULL)
				{
					return MAP_OUT_OF_MEMORY;
				}
				newElement->key = map->keyCopyF(keyElement);
				newElement->data = map->dataCopyF(dataElement);
				if (newElement->key == NULL || newElement->data == NULL)
				{
					map->keyFreeF(keyElement);
					map->dataFreeF(dataElement);
					free(newElement);
					return MAP_OUT_OF_MEMORY;
				}
				if (map->compareF(currElement, map->content->key) == 0)
				{
					Node* tmp = map->iterator;
					map->content = newElement;
					map->iterator = newElement;
					map->iterator->next = tmp;
				}
				else
				{
					Node* tmp = map->iterator;
					map->iterator = newElement;
					map->iterator->next = tmp;
				}
				map->iterator = NULL;
				return MAP_SUCCESS;
			}
			if (map->iterator->next == NULL)
			{
				Node* newElement = malloc(sizeof(*newElement));
				if (newElement == NULL)
				{
					return MAP_OUT_OF_MEMORY;
				}
				newElement->key = map->keyCopyF(keyElement);
				newElement->data = map->dataCopyF(dataElement);
				newElement->next = NULL;
				if (newElement->key == NULL || newElement->data == NULL)
				{
					map->keyFreeF(keyElement);
					map->dataFreeF(dataElement);
					free(newElement);
					return MAP_OUT_OF_MEMORY;
				}
				map->iterator->next = newElement;
				map->iterator = NULL;
				return MAP_SUCCESS;
			}
		}
	}
	map->iterator = NULL;
	return MAP_SUCCESS;
}

/**
*	mapGet: Returns the data associated with a specific key in the map.
*			Iterator status unchanged
*
* @param map - The map for which to get the data element from.
* @param keyElement - The key element which need to be found and whos data
we want to get.
* @return
*  NULL if a NULL pointer was sent or if the map does not contain the requested key.
* 	The data element associated with the key otherwise.
*/
MapDataElement mapGet(Map map, MapKeyElement keyElement)
{
	if (map == NULL || keyElement == NULL || !mapContains(map, keyElement))
	{
		return NULL;
	}
	MapKeyElement currElement = mapGetFirst(map);
	while (currElement != NULL)
	{
		if (map->compareF(currElement, keyElement) == KEYS_ARE_EQUAL)
		{
			return map->iterator->data;
		}
		currElement = mapGetNext(map);
	}
	return NULL;
}

/**
* 	mapRemove: Removes a pair of key and data elements from the map. The elements
*  are found using the comparison function given at initialization. Once found,
*  the elements are removed and deallocated using the free functions
*  supplied at initialzation.
*  Iterator's value is undefined after this operation.
*
* @param map -
* 	The map to remove the elements from.
* @param keyElement
* 	The key element to find and remove from the map. The element will be freed using the
* 	free function given at initialization. The data element associated with this key
*  will also be freed using the free function given at initialization.
* @return
* 	MAP_NULL_ARGUMENT if a NULL was sent to the function
*  MAP_ITEM_DOES_NOT_EXIST if an equal key item does not already exists in the map
* 	MAP_SUCCESS the paired elements had been removed successfully
*/
MapResult mapRemove(Map map, MapKeyElement keyElement)
{
	if (map == NULL || keyElement == NULL)
	{
		return MAP_NULL_ARGUMENT;
	}
	if (mapGetSize(map) == 0 || !mapContains(map, keyElement))
	{
		return MAP_ITEM_DOES_NOT_EXIST;
	}
	else if (mapGetSize(map) == 1 && mapContains(map, keyElement))
	{
		MapKeyElement currElement = mapGetFirst(map);
		MapKeyElement keyToDelete = currElement;
		MapDataElement dataToDelete = mapGet(map, keyToDelete);
		map->content = map->iterator->next;
		map->keyFreeF(keyToDelete);
		map->dataFreeF(dataToDelete);
	}
	else
	{
		MapKeyElement currElement = mapGetFirst(map);
		Node* prev = NULL;
		while (currElement != NULL)
		{
			if (map->compareF(currElement, keyElement) == KEYS_ARE_EQUAL)
			{
				Node* tmp = map->iterator->next;
				MapKeyElement keyToDelete = currElement;
				MapDataElement dataToDelete = mapGet(map, keyToDelete);
				map->keyFreeF(keyToDelete);
				map->dataFreeF(dataToDelete);
				prev->next = tmp;
				break;
			}
			prev = map->iterator;
			currElement = mapGetNext(map);
		}
	}
	return MAP_SUCCESS;
}

/**
*	mapGetFirst: Sets the internal iterator (also called current key element) to
*	the first key element in the map. There doesn't need to be an internal order
*  of the keys so the "first" key element is any key element.
*	Use this to start iteraing over the map.
*	To continue iteration use mapGetNext
*
* @param map - The map for which to set the iterator and return the first
* 		key element.
* @return
* 	NULL if a NULL pointer was sent or the map is empty.
* 	The first key element of the map otherwise
*/
MapKeyElement mapGetFirst(Map map)
{
	if (map == NULL || map->content == NULL)
	{
		return NULL;
	}
	map->iterator = map->content;
	return map->iterator->key;
}

/**
*	mapGetNext: Advances the map iterator to the next key element and returns it.
*	The next key element is any key element not previously returned by the iterator.
* @param map - The map for which to advance the iterator
* @return
* 	NULL if reached the end of the map, or the iterator is at an invalid state
* 	or a NULL sent as argument
* 	The next key element on the map in case of success
*/
MapKeyElement mapGetNext(Map map)
{
	if (map == NULL || map->iterator == NULL || map->iterator->next == NULL)
	{
		return NULL;
	}
	map->iterator = map->iterator->next;
	return map->iterator->key;
}


/**
* mapClear: Removes all key and data elements from target map.
* The elements are deallocated using the stored free functions.
* @param map
* 	Target map to remove all element from.
* @return
* 	MAP_NULL_ARGUMENT - if a NULL pointer was sent.
* 	MAP_SUCCESS - Otherwise.
*/
MapResult mapClear(Map map)
{
	if (map == NULL)
	{
		return MAP_NULL_ARGUMENT;
	}
	MapKeyElement currKey = mapGetFirst(map);
	while (currKey != NULL)
	{
		MapKeyElement keyToDelete = currKey;
		MapDataElement dataToDelete = mapGet(map, keyToDelete);
		currKey = mapGetNext(map);
		map->keyFreeF(keyToDelete);
		map->dataFreeF(dataToDelete);
	}
	map->content = NULL;
	return MAP_SUCCESS;
}
