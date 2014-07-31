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
 /* file: stack.h
 *
 * The implementation of a stack
 */

#include <stdlib.h>

#include "stack.h"

typedef struct BBGStackCell S_CELL;

struct BBGStack
{
  S_CELL *_pCells;
  int    _iSize;
};

struct BBGStackCell
{
  S_CELL  *_pNext;
  void   *_pContent;
};


STACK *AllocStack()
{
  STACK *pStack;

  pStack = malloc(sizeof(*pStack));
  pStack->_pCells = NULL;
  pStack->_iSize = 0;

  return pStack;
}

void FreeStack(STACK *pStack)
{
  S_CELL *pCell;

  while ((pCell = pStack->_pCells) != NULL)
  {
    pStack->_pCells = pCell->_pNext;
    free(pCell);
  }

  free(pStack);
}

void *PopStack(STACK *pStack)
{
  S_CELL *pCell;

  if ((pCell = pStack->_pCells) != NULL)
  {
    void *pContent = pCell->_pContent;

    pStack->_pCells = pCell->_pNext;
    pStack->_iSize--;
    free(pCell);

    return pContent;
  }

  return NULL;
}

void PushStack(void *pContent, STACK *pStack)
{
  S_CELL *pCell;

  pCell = malloc(sizeof(*pCell));
  pCell->_pNext = pStack->_pCells;
  pCell->_pContent = pContent;
  pStack->_pCells = pCell;
  pStack->_iSize++;
}

int StackSize(STACK *pStack)
{
  return pStack->_iSize;
}
