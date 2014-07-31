/******************************************************************************
 *                                                                            *
 *    ###      ###  ##      ##  ########    ##########                        *
 *    #####  #####  ##      ##  ##     ##   ##                                *
 *    ##  ####  ##  ##      ##  ##      ##  ##                                *
 *    ##   ##   ##  ##      ##  ##      ##  ######     ## #####    #######    *
 *    ##        ##  ##      ##  ##      ##  ##          ##    ##  ##     ##   *
 *    ##        ##  ##      ##  ##      ##  ##          ##    ##  ##     ##   *
 *    ##        ##   ##    ##   ##     ##   ##          ##    ##  ##     ##   *
 *    ##        ##    ######    ########    ##########  ##    ##   ########   *
 *                                                                       ##   *
 *                                                                ##     ##   *
 *                                                                 ##    ##   *
 *                                                                  ######    *
 * Developed by William Sayin                                                 *
 * www.github.com/wsayin                                                      *
 * ************************************************************************** *
 * This Source Code Form is subject to the terms of the Mozilla Public        *
 * License, v. 2.0. If a copy of the MPL was not distributed with this        *
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.                   *
 *                                                                            *
 ******************************************************************************/
 /* file: list.c
 *
 * The implementation of a basic double-linked list
 */

#include <stdlib.h>
#include <stdbool.h>

#include "list.h"

/* local procedures */
static void  FreeCell        ( _CELL *pCell, _LIST *pList );
static _CELL *AllocCell       ( void );
static void  InvalidateCell  ( _CELL *pCell );

_LIST *AllocList()
{
   _LIST *pList;

   pList = malloc( sizeof(_LIST) );
   if( !pList )
      return NULL;
   pList->_pFirstCell = NULL;
   pList->_pLastCell = NULL;
   pList->_iterators = 0;
   pList->_valid = 1;
   pList->_size = 0;

   return pList;
}

void AttachIterator(ITERATOR *pIter, _LIST *pList)
{
   if( !pIter || !pList )
      return;
   
   pIter->_pList = pList;

   if (pList != NULL)
   {
      pList->_iterators++;
      pIter->_pCell = pList->_pFirstCell;
   }
   else
      pIter->_pCell = NULL;
   
   return;
}

static _CELL *AllocCell()
{
   _CELL *pCell;

   pCell = malloc(sizeof(*pCell));
   if( !pCell )
      return NULL;
   pCell->_pNextCell = NULL;
   pCell->_pPrevCell = NULL;
   pCell->_pContent = NULL;
   pCell->_valid = 1;

   return pCell;
}

void AttachToList(void *pContent, _LIST *pList)
{
   _CELL *pCell;
   if( !pContent || !pList )
     return;

   /* do not attach to list if already here */
   if( IsInList( pContent, pList ) )
      return;

   pCell = AllocCell();
   pCell->_pContent = pContent;
   pCell->_pNextCell = pList->_pFirstCell;

   if (pList->_pFirstCell != NULL)
      pList->_pFirstCell->_pPrevCell = pCell;
   if (pList->_pLastCell == NULL)
      pList->_pLastCell = pCell;

   pList->_pFirstCell = pCell;

   pList->_size++;
}

void DetachFromList(void *pContent, _LIST *pList)
{
   _CELL *pCell;
   
   if( !pContent || !pList )
      return;

   for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pCell->_pNextCell)
   {
      if (pCell->_pContent && pCell->_pContent == pContent)
      {
         if (pList->_iterators > 0)
            InvalidateCell(pCell);
         else
            FreeCell(pCell, pList);

         pList->_size--;
         return;
      }
   }
}

void DetachIterator(ITERATOR *pIter)
{
  _LIST *pList = pIter->_pList;

  if (pList != NULL)
  {
    pList->_iterators--;

    /* if this is the only iterator, free all invalid cells */
    if (pList->_iterators <= 0)
    {
      _CELL *pCell, *pNextCell;

      for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pNextCell)
      {
        pNextCell = pCell->_pNextCell;

        if (!pCell->_valid)
          FreeCell(pCell, pList);
      }

      if (!pList->_valid)
        FreeList(pList);
    }
  }
}

void FreeList(_LIST *pList)
{
  _CELL *pCell, *pNextCell;
  
  if( !pList )
   return;

  /* if we have any unfinished iterators, wait for later */
  if (pList->_iterators > 0)
  {
    pList->_valid = 0;
    return;
  }

  for (pCell = pList->_pFirstCell; pCell != NULL; pCell = pNextCell)
  {
    pNextCell = pCell->_pNextCell;

    FreeCell(pCell, pList);
  }

  free(pList);
}

static void FreeCell(_CELL *pCell, _LIST *pList)
{
  if (pList->_pFirstCell == pCell)
    pList->_pFirstCell = pCell->_pNextCell;

  if (pList->_pLastCell == pCell)
    pList->_pLastCell = pCell->_pPrevCell;

  if (pCell->_pPrevCell != NULL)
    pCell->_pPrevCell->_pNextCell = pCell->_pNextCell;

  if (pCell->_pNextCell != NULL) 
    pCell->_pNextCell->_pPrevCell = pCell->_pPrevCell;

  free(pCell);
}

static void InvalidateCell(_CELL *pCell)
{
  pCell->_valid = 0;
}

void *NextInList(ITERATOR *pIter)
{
  void *pContent = NULL;

  /* skip invalid cells */
  while (pIter->_pCell != NULL && !pIter->_pCell->_valid)
  {
    pIter->_pCell = pIter->_pCell->_pNextCell;
  }

  if (pIter->_pCell != NULL)
  {
    pContent = pIter->_pCell->_pContent;
    pIter->_pCell = pIter->_pCell->_pNextCell;
  }

  return pContent;
}

int SizeOfList(_LIST *pList)
{
  return pList->_size;
}

bool IsInList( void *datum, _LIST *pList )
{
   void *ptr;
   ITERATOR iter;
   
   if( !datum || !pList )
      return false;
   
   AttachIterator( &iter, pList );
   while( ( ptr = NextInList( &iter ) ) != NULL )
   {
      if( ptr == datum )
         return true;
   }
   DetachIterator( &iter );
   return false;
}
