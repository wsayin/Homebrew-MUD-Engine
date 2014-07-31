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
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* main header file */
#include "mud.h"

D_MOBILE *load_player( const char *name )
{
   lua_State *L;       /* Lua state used for loading player files */
   int status, vnum;
   char filename[MAX_STRING_LENGTH];
   D_MOBILE *dMob;
   
   snprintf( filename, MAX_STRING_LENGTH, "../players/%c/%s.dat", toupper(name[0]), name );
   
   L = luaL_newstate();
   luaL_openlibs( L );
   if( ( status = luaL_loadfile( L, filename ) ) )
   {
      bug( "ERROR: (Lua) Can not load player file %s(%s)", filename, lua_tostring( L, -1 ) );
      return NULL;
   }
   
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "ERROR: (Lua) Unable to parse player file (%s)", lua_tostring( L, -1 ) );
      return NULL;
   }
   
   if( ( dMob = malloc( sizeof( D_MOBILE ) ) ) == NULL )
   {
      bug( "ERROR: (System) Unable to allocate memory for new mobile." );
      return NULL;
   }
   clear_mobile( dMob );
   dMob->events = AllocList();
   
   luaL_loadstring( L, "return name" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->name = strdup( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return prompt" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->prompt = strdup( luaL_checkstring( L, -1 ) );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return level" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->level = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return gender" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->gender = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return st" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->st = (int)luaL_checknumber( L, -1 );
   lua_pop(L, 1 );
   
   luaL_loadstring( L, "return inte" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->in = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return pe" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->pe = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return de" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->de = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return co" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->co = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return ch" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->ch = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return lu" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->lu = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return curHP" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->curHP = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return maxHP" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->maxHP = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return curMV" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->curMV = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return maxMV" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->maxMV = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return curMP" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->curMP = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return maxMP" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   dMob->maxMP = (int)luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return room" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return NULL;
   vnum = (int) luaL_checknumber( L, -1 );
   lua_pop( L, 1 );
   
   dMob->room = frbv( vnum );
   
   lua_close( L );
   return dMob;
}

void save_player( D_MOBILE *dMob )
{
   FILE *fp;
   char  filename[MAX_BUFFER];
   
   snprintf( filename, MAX_BUFFER, "../players/%c/%s.dat", toupper(dMob->name[0]), dMob->name );
   if( ( fp = fopen( filename, "w" ) ) == NULL )
   {
      bug( "Error: unable to allocate memory to save player file %s.", dMob->name );
      return;
   }
   
   fprintf( fp, "name   = \"%s\"\n", dMob->name   );
   fprintf( fp, "prompt = \"%s\"\n", dMob->prompt );
   fprintf( fp, "level  = %i\n",     dMob->level  );
   fprintf( fp, "gender = %i\n",     dMob->gender );
   fprintf( fp, "st = %i\n",         dMob->st     );
   fprintf( fp, "inte = %i\n",         dMob->in     );
   fprintf( fp, "co = %i\n",         dMob->co     );
   fprintf( fp, "pe = %i\n",         dMob->pe     );
   fprintf( fp, "de = %i\n",         dMob->de     );
   fprintf( fp, "ch = %i\n",         dMob->ch     );
   fprintf( fp, "lu = %i\n",         dMob->lu     );
   fprintf( fp, "curHP = %i\n",      dMob->curHP  );
   fprintf( fp, "maxHP = %i\n",      dMob->maxHP  );
   fprintf( fp, "curMP = %i\n",      dMob->curMP  );
   fprintf( fp, "maxMP = %i\n",      dMob->maxMP  );
   fprintf( fp, "curMV = %i\n",      dMob->curMV  );
   fprintf( fp, "maxMV = %i\n",      dMob->maxMV  );
   fprintf( fp, "room = %i\n",       dMob->room ? dMob->room->vnum : 1 );
   
   fclose( fp );
   
   return;
}
