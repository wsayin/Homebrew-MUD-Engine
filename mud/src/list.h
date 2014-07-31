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
 /* file: list.h
 *
 * Headerfile for a basic double-linked list
 */

#ifndef __LIST_HEADER
#define __LIST_HEADER

typedef struct Cell
{
  struct Cell  *_pNextCell;
  struct Cell  *_pPrevCell;
  void         *_pContent;
  int           _valid;
} _CELL;

typedef struct List
{
  _CELL  *_pFirstCell;
  _CELL  *_pLastCell;
  int    _iterators;
  int    _size;
  int    _valid;
} _LIST;

typedef struct Iterator
{
  _LIST  *_pList;
  _CELL  *_pCell;
} ITERATOR;

_LIST *AllocList          ( void );
void  AttachIterator     ( ITERATOR *pIter, _LIST *pList);
void *NextInList         ( ITERATOR *pIter );
void  AttachToList       ( void *pContent, _LIST *pList );
void  DetachFromList     ( void *pContent, _LIST *pList );
void  DetachIterator     ( ITERATOR *pIter );
void  FreeList           ( _LIST *pList );
int   SizeOfList         ( _LIST *pList );
bool  IsInList           ( void *ptr, _LIST *pList );

#endif
