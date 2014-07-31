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
#ifndef __UTILS_H__
#define __UTILS_H__
bool     check_name              ( char *name );
void     clear_mobile            ( D_M *dMob );
D_M     *new_mobile              (   );
void     free_mobile             ( D_M *dMob );
D_M     *fmbv                    ( int vnum );
void     communicate             ( D_M *dMob, char *txt, int range );
void     load_muddata            ( bool fCopyOver );
char    *get_time                ( void );
void     copyover_recover        ( void );
D_M     *check_reconnect         ( char *player );
D_ROOM  *new_room                (   );
void     free_room               ( D_ROOM *r );
D_AREA  *new_area                (   );
void     free_area               ( D_AREA *a );
D_EXIT  *new_exit                (   );
void     free_exit               ( D_EXIT *e );
D_OBJ   *new_object              (   );
void     free_object             ( D_OBJ *o  );
D_OBJ   *fobv                    ( int vnum );
D_RESET *new_reset               (   );
void     free_reset              ( D_RESET *r );
D_M     *new_npc                 (   );
void     free_npc                ( D_M *npc );
void     rseed                   (   );
int      rrange                  ( int low, int hi );
int      roll                    ( int numDie, int sizeDie );
D_OBJ   *clone_obj               ( const D_OBJ *obj );
D_M     *clone_npc               ( const D_M *mobile );
const char *list_mobiles         ( _LIST *l, const char *tab, const D_M *except );
bool     is_name                 ( char *name, char *string );
D_M     *find_mob_by_name        ( _LIST *l, char *name );
#endif //__utils_h__
