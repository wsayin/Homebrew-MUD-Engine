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

lua_State *globalLState;     /* Global Lua State */

//forward declarations
static lua_CFunction stackDump( lua_State *L );

void init_lua()
{
   globalLState = luaL_newstate();
   luaL_openlibs( globalLState );
   lua_atpanic( globalLState, (lua_CFunction)stackDump );

   return;
}

void close_lua()
{
   lua_close( globalLState );
   return;
}

void load_mud_settings()
{
   lua_State *L;
   
   L = luaL_newstate();
   luaL_openlibs( L );
   lua_atpanic( L, (lua_CFunction)stackDump );
   
   //set up defaults
   MUD_NAME = strdup( "DARKMUD" );
   ADMIN_EMAIL = strdup( "NULL" );
   MUDPORT = 5432;
   
   
   
   
   if( luaL_loadfile( L, "../system/mud.dat" ) )
   {
      bug( "ERROR: Could not load MUD Data File (%s)", lua_tostring( L, -1 ) );
      return;
   }
   
   if( lua_pcall( L, 0, 1, 0 ) )
   {
      bug( "ERROR: Unable to parse MUD Data File (%s)", lua_tostring( L, -1 ) );
      return;
   }
   
   luaL_loadstring( L, "return name" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return;
   if( lua_isstring( L, -1 ) )
   {
      if( MUD_NAME )
         free( MUD_NAME );
      MUD_NAME = strdup( lua_tostring( L, -1 ) );
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return MySQL_Server" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return;
   if( lua_isstring( L, -1 ) )
   {
      if( MYSQL_SERVER_ADDRESS )
         free( MYSQL_SERVER_ADDRESS );
      MYSQL_SERVER_ADDRESS = strdup( lua_tostring( L, -1 ) );
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return MySQL_Login" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return;
   if( lua_isstring( L, -1 ) )
   {
      if( MYSQL_SERVER_LOGIN )
         free(MYSQL_SERVER_LOGIN);
      MYSQL_SERVER_LOGIN = strdup( lua_tostring( L, -1 ) );
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return MySQL_Pass" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return;
   if( lua_isstring( L, -1 ) )
   {
      if( MYSQL_SERVER_PASS )
         free( MYSQL_SERVER_PASS );
      MYSQL_SERVER_PASS = strdup( lua_tostring( L, -1 ) );
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return MySQL_Default_DB" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return;
   if( lua_isstring( L, -1 ) )
   {
      if( MYSQL_DEFAULT_DB )
         free(MYSQL_DEFAULT_DB);
      MYSQL_DEFAULT_DB = strdup( lua_tostring( L, -1 ) );
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return email" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return;
   if( lua_isstring( L, -1 ) )
   {
      if( ADMIN_EMAIL )
         free( ADMIN_EMAIL );
      ADMIN_EMAIL = strdup( lua_tostring( L, -1 ) );
   }
   lua_pop( L, 1 );
   
   luaL_loadstring( L, "return port" );
   if( lua_pcall( L, 0, 1, 0 ) )
      return;
   MUDPORT = lua_tonumber( L, -1 );
   lua_pop( L, 1 );
   
   return;
}

   
static lua_CFunction stackDump (lua_State *L) //useful for debugging purposes
{
   int i;
   int top = lua_gettop(L);
   char buf[MAX_BUFFER];
   char *place = buf;
   place += snprintf( place, MAX_BUFFER-strlen(buf), "\nLua Stack\n---------\n\tType   Data\n\t-----------\n" );
   for (i = 1; i <= top; i++)
   {  /* repeat for each level */
      int t = lua_type(L, i);
      place += snprintf( place, MAX_BUFFER-strlen(buf), "%i", i );
      switch (t)
      {
 
         case LUA_TSTRING:  /* strings */
            place += snprintf( place, MAX_BUFFER-strlen(buf),"\tString: `%s'\n", lua_tostring(L, i));
            break;
 
         case LUA_TBOOLEAN:  /* booleans */
            place += snprintf( place, MAX_BUFFER-strlen(buf),"\tBoolean: %s", lua_toboolean(L, i) ? "\ttrue\n" : "\tfalse\n");
            break;
 
         case LUA_TNUMBER:  /* numbers */
            place += snprintf( place, MAX_BUFFER-strlen(buf),"\tNumber: %g\n", lua_tonumber(L, i));
            break;
 
         default:  /* other values */
            place += snprintf( place, MAX_BUFFER-strlen(buf),"\tOther: %s\n", lua_typename(L, t));
            break;
 
     } 
   }
   bug(buf);  /* end the listing */
   return 0;
}
