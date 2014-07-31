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
#include "mud.h"
#include "event.h"

lua_State *area_loader;

//forward declarations
static void link_exits();
static D_ROOM  *load_room  ( lua_State  *L );
static D_EXIT  *load_exit  ( lua_State  *L );
static D_OBJ   *load_object( lua_State  *L );
static D_M     *load_mobile( lua_State  *L );
static D_RESET *load_reset ( lua_State  *L );
static void load_area      ( const char *name );
static bool load_area_stats         ( lua_State *L, D_AREA   *area );
static bool load_area_rooms         ( lua_State *L, D_AREA   *area );
static bool load_area_objects       ( lua_State *L, D_AREA   *area );
static bool load_area_mobiles       ( lua_State *L, D_AREA   *area );
static bool load_room_resets        ( lua_State *L, D_ROOM   *room );
static void load_room_objects_resets( lua_State *L, D_RESET  *r    );
static void load_room_mobiles_resets( lua_State *L, D_RESET  *r    );

//external declarations
extern bool event_reset_area( EVENT_DATA *event );

//The world list is just a file with a line-by-line list of all the areas
//in the game.
void load_world()
{
   FILE *fp;
   ITERATOR iter;
   D_AREA *area;
   char buf[MAX_BUFFER];
   
   area_loader = luaL_newstate();
   luaL_openlibs( area_loader );
   
   log_string( "Loading World" );
   
   if( ( fp = fopen( "../areas/list.are", "r" ) ) == NULL )
   {
      log_string( "FATAL ERROR: Area List (areas/list.are) does not exist!" );
      abort();
      return;
   }
   
   while( !feof( fp ) )
   {
      fgets( buf, MAX_BUFFER, fp );
      buf[strlen(buf)-1] = '\0'; //fgets includes the newline in the read, so we kill it here.
      if( !strcasecmp( buf, "END" ) )
         break;
      load_area( buf );
   }  
   fclose( fp );
   link_exits();
   
   AttachIterator( &iter, area_list );
   while( ( area = (D_AREA*)NextInList( &iter ) ) != NULL )
      reset_area( area );
   DetachIterator( &iter );
   
   return;
}

static void load_area( const char * name )
{
   lua_State *L = area_loader; //shortcut name
   int status;
   char filename[MAX_BUFFER];
   D_AREA *area;
   EVENT_DATA *event;
   
   log_string( "--Loading %s", name );
   snprintf( filename, MAX_BUFFER, "../areas/%s", name );

   if( ( status = luaL_loadfile( L, filename ) ) )
   {
      bug( "ERROR: Can not find area file %s(%s)", filename, lua_tostring( L, -1 ) );
      return;
   }
   
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "ERROR: Unable to parse area file (%s)", lua_tostring( L, -1 ) );
      return;
   }
   
   if( ( area = new_area() ) == NULL )
   {
      bug( "ERROR: Unable to allocate memory for area." );
      return;
   }
   //load the area stats
   if( load_area_stats( L, area ) == FALSE ) //syntax errors in the area file
   {
      log_string( "----stats (failed)" );
      return;
   }
   else
   {
      log_string( "----stats" );
   }
   
   //load the actual rooms, stored in the table 'rooms'
   if( load_area_rooms( L, area ) == FALSE )
   {
      log_string( "----rooms(failed)" ); //failing room load == fail area load
      return;
   }
   else
   {
      log_string( "----rooms" );
   }
   
   //load the objects, stored in table 'objects'
   if( load_area_objects( L, area ) == FALSE )
   {
      log_string( "----objects(failed)" );
   }
   else
   {
      log_string( "----objects" );
   }
   
   //load the NPCs, stored in table 'mobiles'
   if( load_area_mobiles( L, area ) == FALSE )
   {
      log_string( "----mobiles(failed)" );
   }
   else
   {
      log_string( "----mobiles" );
   }
   
   //enqueue its first reset
   if( ( event = alloc_event() ) == NULL )
   {
      bug( "Error: (System) Unable to allocate memory for new event!" );
      exit( 1 );
   }
   event->fun = event_reset_area;
   event->type = EVENT_RESET_AREA;
   event->owner.dArea = area;
   add_event_game( event, area->reset_time * 60 * PULSES_PER_SECOND );
   
   return;
}

static bool load_area_objects( lua_State *L, D_AREA *area )
{
   int numObj = 0, i = 0;
   D_OBJ *obj;
   
   if( !L || !area )
      return FALSE;
      
   lua_getglobal( L, "objects" );
   if( !lua_istable( L, -1 ) )
   {
      bug( "Error: 'objects' is not a valid table.\n" );
      return FALSE;
   }
   //numObj = lua_len( L, -1 );
   lua_len( L, -1 );
   numObj = luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   for( i = 1; i <= numObj; i++ )
   {
      lua_rawgeti( L, -1, i );
      if( ( obj = load_object( L ) ) == NULL )
      {
         bug( "Error: Unable to allocate memory for new object." );
         return FALSE;
      }
      AttachToList( obj, obj_list );
      AttachToList( obj, area->objects );
      lua_pop( L, 1 );
   }
   
   return TRUE;
}

static bool load_area_mobiles( lua_State *L, D_AREA *area )
{
   int numMob = 0, i = 0;
   D_MOBILE *mob;
   
   if( !L || !area )
      return FALSE;
      
   lua_getglobal( L, "mobiles" );
   if( !lua_istable( L, -1 ) )
   {
      bug( "Error: 'mobiles' is not a valid table.\n" );
      return FALSE;
   }
   //numMob = lua_len( L, -1 );
   lua_len( L, -1 );
   numMob = luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   for( i = 1; i <= numMob; i++ )
   {
      lua_rawgeti( L, -1, i );
      if( ( mob = load_mobile( L ) ) == NULL )
      {
         bug( "Error: Unable to allocate memory for new mobile." );
         return FALSE;
      }
      AttachToList( mob, npc_list );
      AttachToList( mob, area->mobiles );
      lua_pop( L, 1 );
   }
   
   return TRUE;
}

static bool load_area_rooms( lua_State *L, D_AREA *area )
{
   int numRooms, i;
   D_ROOM *room;
   
   if( !L || !area )
      return FALSE;
   
   lua_getglobal( L, "rooms" );
   if( !lua_istable( L, -1 ) )
   {
      bug( "Error: 'rooms' is not a valid table.\n" ); //if an area doesn't have rooms then fail the load
      return FALSE;
   }
   //numRooms = lua_len( L, -1 );
   lua_len( L, -1 );
   numRooms= luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   for( i = 1; i <= numRooms; i++ ) //iterate through the rooms
   {
      lua_rawgeti( L, -1, i );
      if( ( room = load_room( L ) ) == NULL )
      {
         bug( "Error: Unable to allocate memory for new room." );
         return FALSE;
      }
      AttachToList( room, area->rooms ); //attach it to the area's room list
      lua_pop( L, 1 );
   }
   return TRUE;
}

static bool load_room_resets( lua_State *L, D_ROOM *room )
{
   int num, i;
   D_RESET *r;
   
   if( !L || !room )
      return FALSE;
   
   lua_pushstring( L, "resets" );
   lua_gettable( L, -2 );
   lua_len( L, -1 );
   num = luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   for( i = 1; i <= num; i++ ) //iterate through the resets
   {
      lua_rawgeti( L, -1, i );
      if( ( r = load_reset( L ) ) == NULL )
      {
         bug( "Error: Unable to allocate memory for new reset." );
         return FALSE;
      }
      AttachToList( r, room->resets ); //attach it to the room's reset list
      lua_pop( L, 1 );
   }
   return TRUE;
}

static D_ROOM *load_room( lua_State *L )
{
   D_ROOM *r;
   D_EXIT *e;
   int i=0, numExits=0;
   
   if( !L )
      return NULL;
      
   
   if( ( r = new_room() ) == NULL )
   {
      return NULL;
   }
   
   lua_pushstring( L, "vnum" );
   lua_gettable( L, -2 );
   r->vnum = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "name" );
   lua_gettable( L, -2 );
   r->name = strdup( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "desc" );
   lua_gettable( L, -2 );
   r->desc = strdup( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "exits" );
   lua_gettable( L, -2 );
   //numExits = lua_len( L, -1 );
   lua_len( L, -1 );
   numExits = luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   for( i = 1; i <= numExits; i++ ) //iterate through the exits
   {
      lua_rawgeti( L, -1, i );
      if( ( e = load_exit( L ) ) == NULL )
      {
         bug( "Error: Unable to allocate memory for new exit." );
         continue;
      }
      AttachToList( e, r->exits ); //attach it to the room's exit list
      lua_pop( L, 1 );
   }
   lua_pop( L, 1 );

   load_room_resets( L, r );
   lua_pop( L, 1 );
   return r;
}

static D_EXIT *load_exit( lua_State *L )
{
   D_EXIT *e;
   
   if( !L )
      return NULL;
      
   if( ( e = new_exit() ) == NULL )
      return NULL;
   
   lua_pushstring( L, "name" );
   lua_gettable( L, -2 );
   e->name = strdup( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "desc" );
   lua_gettable( L, -2 );
   e->desc = strdup( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "dest" );
   lua_gettable( L, -2 );
   e->destVnum = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "state" );
   lua_gettable( L, -2 );
   const char *state = luaL_checkstring( L, -1 ); //temporary
   if( !strcasecmp( state, "STATE_OPEN" ) )
      e->state = STATE_OPEN;
   else if( !strcasecmp( state, "STATE_DOOR_OPEN" ) )
      e->state = STATE_DOOR_OPEN;
   else if( !strcasecmp( state, "STATE_DOOR_CLOSED" ) )
      e->state = STATE_DOOR_CLOSED;
   else if( !strcasecmp( state, "STATE_DOOR_CLOSED_LOCKED" ) )
      e->state = STATE_DOOR_CLOSED_LOCKED;
   else if( !strcasecmp( state, "STATE_DOOR_CLOSED_BROKEN" ) )
      e->state = STATE_DOOR_CLOSED_BROKEN;
   else if( !strcasecmp( state, "STATE_DOOR_OPEN_BROKEN" ) )
      e->state = STATE_DOOR_OPEN_BROKEN;
   else
   {
      bug( "Error: Invalid exit state \"%s\".", state );
      e->state = STATE_OPEN; //default
   }
   lua_pop( L, 1 );
   
   return e;
}

static bool load_area_stats( lua_State *L, D_AREA *area )
{
   //load all the area data
   luaL_loadstring( L, "return name" );
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isstring( L, -1 ) )
   {
      area->name = strdup( lua_tostring( L, -1 ) );
   }
   else
   {
      area->name = strdup( "Name Not Set" );
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return author" );
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isstring( L, -1 ) )
   {
      area->author = strdup( lua_tostring( L, -1 ) );
   }
   else
   {
      area->author = strdup( "Anonymous" );
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return reset_time" );
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isnumber( L, -1 ) )
   {
      area->reset_time = lua_tonumber( L, -1 );
   }
   else
   {
      area->reset_time = 15; //default to reset every 15 minutes
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return rLow" ); //low vnum of rooms
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isnumber( L, -1 ) )
   {
      area->rLow = lua_tonumber( L, -1 );
   }
   else
   {
      area->rLow = 0;
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return rHi" ); //high vnum of rooms
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isnumber( L, -1 ) )
   {
      area->rHi = lua_tonumber( L, -1 );
   }
   else
   {
      area->rHi = 0;
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return mLow" ); //low vnum of NPCs in area
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isnumber( L, -1 ) )
   {
      area->mLow = lua_tonumber( L, -1 );
   }
   else
   {
      area->mLow = 0;
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return mHi" );//high vnum of NPCs in area
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isnumber( L, -1 ) )
   {
      area->mHi = lua_tonumber( L, -1 );
   }
   else
   {
      area->mHi = 0;
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return oLow" );//low vnum of objects in area
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isnumber( L, -1 ) )
   {
      area->oLow = lua_tonumber( L, -1 );
   }
   else
   {
      area->oLow = 0;
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return oHi" );//high vnum of objects in area
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isnumber( L, -1 ) )
   {
      area->oHi = lua_tonumber( L, -1 );
   }
   else
   {
      area->oHi = 0;
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return levelLow" );//minimum allowed level of players in area
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isnumber( L, -1 ) )
   {
      area->levelLow = lua_tonumber( L, -1 );
   }
   else
   {
      area->levelLow = MIN_PC_LEVEL;
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return levelHi" );//maximum allowed level of players in area
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "Syntax Error in area file %s", area->name );
      return FALSE;
   }
   if( lua_isnumber( L, -1 ) )
   {
      area->levelHi = lua_tonumber( L, -1 );
   }
   else
   {
      area->levelLow = MAX_PC_LEVEL;
   }
   lua_pop( L, 1 );
   
   return TRUE;
}

static D_OBJ *load_object( lua_State *L )
{
   D_OBJ *o;
   
   if( !L )
      return NULL;
   
   if( ( o = new_object() ) == NULL )
      return NULL;
   
   lua_pushstring( L, "name" );
   lua_gettable( L, -2 );
   o->name = strdup( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "sDesc" );
   lua_gettable( L, -2 );
   o->sDesc = strdup( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "lDesc" );
   lua_gettable( L, -2 );
   o->lDesc = strdup( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "vnum" );
   lua_gettable( L, -2 );
   o->vnum = luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   return o;
}

static D_RESET *load_reset( lua_State *L )
{
   D_RESET *r;
   
   if( !L )
      return NULL;
   
   if( ( r = new_reset() ) == NULL )
      return NULL;
   
   lua_pushstring( L, "rtype" );
   lua_gettable( L, -2 );
   r->type = luaL_checkstring( L, -1 )[0]; //'M' for mob reset, 'O' for object
   lua_pop( L, 1 );                        //additional can be added easily
   
   lua_pushstring( L, "vnum" );
   lua_gettable( L, -2 );
   r->vnum = luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   switch( r->type )
   {
      case 'm':
      case 'M':
      {
         //attach any worn items
         break;
      }
      case 'o':
      case 'O':
      {
         //load_area_objects_resets( L, 
         break;
      }
      default:
      {
         bug( "Reset with invalid type \'%c\'.", r->type );
         free( r );
         return NULL;
         break;
      }
   }
   
   return r;
}

static D_MOBILE *load_mobile( lua_State *L )
{
   D_MOBILE *m;
   
   if( !L )
      return NULL;
   
   if( ( m = new_npc() ) == NULL )
      return NULL;
   
   lua_pushstring( L, "name" );
   lua_gettable( L, -2 );
   m->name = strdup( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "gender" );
   lua_gettable( L, -2 );
   m->gender = (int)luaL_checknumber( L, -1 ) == 0 ? FEMALE : MALE;
   lua_pop( L, 1 );
   
   lua_pushstring( L, "level" );
   lua_gettable( L, -2 );
   m->level = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   lua_pushstring( L, "vnum" );
   lua_gettable( L, -2 );
   m->npc_data->vnum = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   return m;
}
   
static void link_exits()
{
   D_ROOM *r, *d; //room and destination room
   D_EXIT *e; //the exit we're linking in the room
   ITERATOR iterRoom, iterExit; //iterators for rooms and exits
   
   if( room_list == NULL )
   {
      bug( "ERROR: Global room list does not exist. Aborting game!!" );
      exit(1);
   }
   
   AttachIterator( &iterRoom, room_list );//iterate through all the rooms in the game
   while( ( r = (D_ROOM*)NextInList( &iterRoom ) ) != NULL ) 
   {
      if( r->exits )
      {
         AttachIterator( &iterExit, r->exits ); //iterate through each room's exits
         while( ( e = (D_EXIT*)NextInList(&iterExit ) ) != NULL )
         {
            if( e->dest ) continue;//If it's already linked, skip it
            if( ( d = frbv( e->destVnum ) ) == NULL )
            {
               bug( "Error: Attempting to link exit to room that does not exist( Room: %i Exit: %s to room %i.)", r->vnum, e->name, e->destVnum );
               continue;
            }
            e->dest = d;
         }
         DetachIterator( &iterExit );
      }
      else
      {
         bug( "Error: Room created with unallocated exit list. Correcting now..." );
         r->exits = AllocList();
      }
   }
   DetachIterator( &iterRoom );
   return;
}

void reset_area( D_AREA *area )
{
   ITERATOR it,rIt; // it = room iterator, rIt = reset iterator
   D_OBJ *o;
   D_RESET *reset;
   D_MOBILE *m;
   D_ROOM *room;
   
   AttachIterator( &it, area->rooms );
   while( ( room = (D_ROOM*)NextInList( &it ) ) != NULL )
   {
      AttachIterator( &rIt, room->resets );
      while( ( reset = (D_RESET*)NextInList(&rIt) ) != NULL )
      {
         switch( reset->type )
         {
            default:
            {
               bug( "Invalid reset type %c.", reset->type );
               break;
            }
            case 'o':
            case 'O':
            {
                //if the reset has already fired once we need to see if the object
                //is still there, if its not, make a new one.
               if( reset->ptr )
               {
                  //if object is still there skip this reset
                  if( IsInList( reset->ptr, room->objects ) ) 
                     continue;
               }
               if( ( o = clone_obj( fobv( reset->vnum ) ) ) == NULL )
               {
                  bug( "Reset calls for object vnum %i which does not exist.", reset->vnum );
                  continue;
               }
               AttachToList( o, room->objects );
               //does the reset reference any subresets (contained objects)?
               if( reset->nest && SizeOfList( reset->nest ) > 0 )
               {
               }
               
               o->reset = reset;
               reset->ptr = o;
               break;
            }
            case 'm':
            case 'M':
            {
               if( reset->ptr )
               {
                  if( IsInList( reset->ptr, npc_list ) )
                     continue;
               }
               if( ( m = clone_npc( fmbv( reset->vnum ) ) ) == NULL )
               {
                  bug( "Reset calls for NPC vnum %i which does not exist.", reset->vnum );
                  continue;
               }
               AttachToList( m, room->mobiles );
               m->npc_data->reset = reset;
               reset->ptr = m;
               break;
            }
         }
      }
      DetachIterator( &rIt );
   }
   DetachIterator( &it );
   return;
}
   

void move_mob( D_MOBILE *dMob, const char *dir )
{
   D_EXIT *exit;
   
   if( !dMob || !dir )
      return;
      
   if( !dMob->room )
   {
      bug( "Error: Player (%s) without room. Sending to the void...", dMob->name );
      dMob->room = frbv( 1 );//default to the void if they don't have a room assigned to them
      if( !dMob->room ) //if the void doesn't even exist...
      {
         bug( "Error: Can Not Find The Void Room!" );
         return;
      }
   }
   
   exit = febn( dir, dMob->room->exits );
   if( !exit || !exit->dest )
   {
      if( exit && !exit->dest )
         bug( "Error: Exit with non-existent room in room %i.", dMob->room->vnum );
      text_to_mobile( dMob, "Alas, you can not go that way.\r\n" );
      return;
   }
   switch( exit->state )
   {
      case STATE_OPEN:
      case STATE_DOOR_OPEN:
      case STATE_DOOR_OPEN_BROKEN:
      {
         break;
      }
      case STATE_DOOR_CLOSED:
      case STATE_DOOR_CLOSED_BROKEN:
      case STATE_DOOR_CLOSED_LOCKED:
      {
         text_to_mobile( dMob, "The %s is closed.\r\n", exit->desc );
         return;
      }
      default:
      {
         bug( "Exit \'%s\' in invalid state in room %i.", exit->name, dMob->room->vnum );
         return;
      }
   }

   DetachFromList( dMob, dMob->room->mobiles );
   dMob->room = exit->dest;
   AttachToList( dMob, dMob->room->mobiles );
   
   cmd_look( dMob, "" );
   
   return;
}

//load details for an object's reset
#ifdef FUUUUUCK
static void load_room_objects_resets( lua_State *L, D_RESET *r )
{
   int vnum;
   int num;
   int i;
   D_OBJ *o;
   
   if( !L || !r )
   {
      bug( "Calling load_area_objects_resets with NULL arguments" );
      return;
   }
   
   lua_pushstring( L, "inside" );
   lua_gettable( L, -2 );
   if( lua_isnil( L, -1 ) )
   {
      //do nothing
   }
   else if( !lua_istable( L, -1 ) )
   {
      bug( "Error: 'inside' is not a valid table.\n" );
      return;
   }
   else
   {
      num = lua_len( L, -1 );
      for( i = 1; i <= num; i++ ) //iterate through the rooms
      {
         lua_rawgeti( L, -1, i );
         vnum = luaL_checknumber( L, -1 );
         if( ( o = fobv( vnum ) ) == NULL )
         {
            bug( "Reset references object vnum %i, which does not exist.", vnum );
            continue;
         }
         AttachToList( o, obj->in );
         lua_pop( L, 1 );//the vnum of the contained object
      }
   }
   lua_pop( L, 1 );//the 'in' table
   
   lua_getglobal( L, "on" );
   if( lua_isnil( L, -1 ) )
   {
      //do nothing
   }
   else if( !lua_istable( L, -1 ) )
   {
      bug( "Error: 'on' is not a valid table.\n" );
      return;
   }
   else
   {
      num = lua_len( L, -1 );
      for( i = 1; i <= num; i++ ) //iterate through the rooms
      {
         lua_rawgeti( L, -1, i );
         vnum = luaL_checknumber( L, -1 );
         if( ( o = fobv( vnum ) ) == NULL )
         {
            bug( "Reset references object vnum %i, which does not exist.", vnum );
            continue;
         }
         AttachToList( o, obj->on );
         lua_pop( L, 1 );//the vnum of the contained object
      }
   }
   lua_pop( L, 1 );//the 'on' table
   
   lua_getglobal( L, "under" );
   if( lua_isnil( L, -1 ) )
   {
      //do nothing
   }
   else if( !lua_istable( L, -1 ) )
   {
      bug( "Error: 'under' is not a valid table.\n" );
      return;
   }
   else
   {
      num = lua_len( L, -1 );
      for( i = 1; i <= num; i++ ) //iterate through the rooms
      {
         lua_rawgeti( L, -1, i );
         vnum = luaL_checknumber( L, -1 );
         if( ( o = fobv( vnum ) ) == NULL )
         {
            bug( "Reset references object vnum %i, which does not exist.", vnum );
            continue;
         }
         AttachToList( o, obj->under );
         lua_pop( L, 1 );//the vnum of the contained object
      }
   }
   lua_pop( L, 1 );//the 'under' table
   
   return;
}
#endif
//load details for a mobile's reset
static void load_area_mobiles_resets( lua_State *L, D_MOBILE *mob )
{
   UNUSED(L);
   UNUSED(mob);
   return;
}
