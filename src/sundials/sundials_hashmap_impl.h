/* -----------------------------------------------------------------
 * Programmer: Cody J. Balos @ LLNL
 * -----------------------------------------------------------------
 * SUNDIALS Copyright Start
 * Copyright (c) 2002-2024, Lawrence Livermore National Security
 * and Southern Methodist University.
 * All rights reserved.
 *
 * See the top-level LICENSE and NOTICE files for details.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * SUNDIALS Copyright End
 * -----------------------------------------------------------------
 * A simple hashmap implementation for char* keys and
 * void* values. Uses linear probing to resolve collisions.
 * The values can be anything, but will be freed by
 * the hash map upon its destruction.
 * -----------------------------------------------------------------*/

#ifndef _SUNDIALS_HASHMAP_IMPL_H
#define _SUNDIALS_HASHMAP_IMPL_H

#include <stdlib.h>
#include <sundials/sundials_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SUNHashMapKeyValue_s* SUNHashMapKeyValue;

struct SUNHashMapKeyValue_s
{
  const char* key;
  void* value;
};

#define TTYPE SUNHashMapKeyValue
#include "sundials_arraylist.h"

typedef struct SUNHashMap_s* SUNHashMap;

struct SUNHashMap_s
{
  size_t capacity; /* max number of entries */
  SUNArrayList_SUNHashMapKeyValue buckets;
};

SUNErrCode SUNHashMap_New(size_t init_capacity, SUNHashMap* map);
size_t SUNHashMap_Capacity(SUNHashMap map);
SUNErrCode SUNHashMap_Destroy(SUNHashMap* map, void (*freevalue)(void* ptr));

size_t SUNHashMap_Iterate(SUNHashMap map, int start,
                          int (*yieldfn)(int, SUNHashMapKeyValue, void*), void* ctx);
int SUNHashMap_Insert(SUNHashMap map, const char* key, void* value);
int SUNHashMap_GetValue(SUNHashMap map, const char* key, void** value);
int SUNHashMap_Remove(SUNHashMap map, const char* key, void** value);
SUNErrCode SUNHashMap_Sort(SUNHashMap map, SUNHashMapKeyValue** sorted,
                           int (*compar)(const void*, const void*));

#if SUNDIALS_MPI_ENABLED
SUNErrCode SUNHashMap_Values(SUNHashMap map, void*** values, size_t value_size);
#endif

#ifdef __cplusplus
}
#endif

#endif
