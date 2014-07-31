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
#ifndef __WORLD_H__
#define __WORLD_H__
struct dArea
{
   char *name;
   char *filename;
   char *author;
   int   reset_time;
   int   levelLow;
   int   levelHi;
   int   mLow;
   int   mHi;
   int   oLow;
   int   oHi;
   int   rLow;
   int   rHi;
   
   _LIST *rooms;
   _LIST *objects;
   _LIST *mobiles;
};

struct dRoom
{
   D_AREA  * area;
   char    * name;       //the title of the room
   char    * desc;       //the description of the room
   int       vnum;       //the room ID #
   _LIST   * mobiles; //a list of all the characters (PC & NPC) in the room
   _LIST   * objects;    //list of all the objects in the room;
   _LIST   * exits;      //list of the exits in the room
   _LIST   * resets;
};

struct dExit
{
   char              * name; //the exits name-- what a player has to type to use it
   char              * desc; //what the player sees when they type 'look <direction>'
   enum door_state     state; //enum describing the state of the door
   int                 keyID; //the vnum of the key that unlocks the door
   D_ROOM            * dest; //the room that the exit points to.
   int                 destVnum; //the vnum the exit points to, used before all exits are linked
   bool                hidden; //does the exit show up when the player types 'look'?
};

struct dReset
{
   char       type;//O for object, M for Mobile
   int        vnum; //the vnum of the mob/object
   int        destVnum;//the room it goes in. Still trying to figure out nesting
   void     * ptr; //pointer to what was reset
   _LIST    * nest;
};

void load_world();
D_ROOM *frbv( int vnum );
D_EXIT *febn( const char *name, _LIST *exits );
void move_mob( D_MOBILE *dMob, const char *dir );
void reset_area( D_AREA *area );

#endif //__WORLD_H__
