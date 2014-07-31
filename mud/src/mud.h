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
#ifndef MUD_H
#define MUD_H

#include <zlib.h>
#include <pthread.h>
#include <arpa/telnet.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "list.h"
#include "stack.h"

//Lua Libraries
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/************************
 * Standard definitions *
 ************************/

/* define TRUE and FALSE */
#ifndef FALSE
#define FALSE   0
#endif
#ifndef TRUE
#define TRUE    1
#endif

#define eTHIN   0
#define eBOLD   1

/* A few globals */
#define UNUSED( x ) ( ( void )( x ) )
#define PULSES_PER_SECOND     4                   /* must divide 1000 : 4, 5 or 8 works */
#define MAX_BUFFER         2048                   /* seems like a decent amount         */
#define MAX_INPUT_LENGTH   2048
#define MAX_STRING_LENGTH  2048
#define MAX_OUTPUT         2048                   /* well shoot me if it isn't enough   */
#define MAX_HELP_ENTRY     4096                   /* roughly 40 lines of blocktext      */
#define MAX_PC_NAME_LENGTH   16                   /* We don't need insanely long player names */
#define FILE_TERMINATOR    "EOF"                  /* end of file marker                 */
#define COPYOVER_FILE      "../txt/copyover.dat"  /* tempfile to store copyover data    */
#define EXE_FILE           "../src/dark"          /* the name of the mud binary         */
extern const char *mainMenu;
extern char *MUD_NAME;
extern char *ADMIN_EMAIL;
extern int MUDPORT;
extern char *MYSQL_SERVER_ADDRESS;
extern char *MYSQL_SERVER_PASS;
extern char *MYSQL_SERVER_LOGIN;
extern char *MYSQL_DEFAULT_DB;
#define MAX_PC_LEVEL 65
#define MIN_PC_LEVEL  0

/* Connection states */
enum state_t { STATE_GET_ACCOUNT = 0, STATE_NEW_ACCOUNT, STATE_ASK_PASSWORD,
               STATE_MAIN_MENU, STATE_CHOOSE_CHAR, STATE_CHARGEN_NAME,
               STATE_CHARGEN_GENDER,
               STATE_NEW_NAME,
               STATE_NEW_PASSWORD, STATE_VERIFY_PASSWORD, STATE_PLAYING,
               STATE_CLOSED };

enum gender { FEMALE, MALE };

                  //v- No door     v- door, open    v- door, closed, etc
enum door_state { STATE_OPEN = 0, STATE_DOOR_OPEN, STATE_DOOR_CLOSED,
                  STATE_DOOR_CLOSED_LOCKED, STATE_DOOR_CLOSED_BROKEN,
                  STATE_DOOR_OPEN_BROKEN };

/* Thread states - please do not change the order of these states    */
#define TSTATE_LOOKUP          0  /* Socket is in host_lookup        */
#define TSTATE_DONE            1  /* The lookup is done.             */
#define TSTATE_WAIT            2  /* Closed while in thread.         */
#define TSTATE_CLOSED          3  /* Closed, ready to be recycled.   */

/* player levels */
#define LEVEL_GUEST            1  /* Dead players and actual guests  */
#define LEVEL_PLAYER           2  /* Almost everyone is this level   */
#define LEVEL_ADMIN            3  /* Any admin without shell access  */
#define LEVEL_GOD              4  /* Any admin with shell access     */

/* Communication Ranges */
#define COMM_LOCAL             0  /* same room only                  */
#define COMM_LOG              10  /* admins only                     */

/* define simple types */
typedef  short int           sh_int;
typedef  unsigned short int  usi;
typedef  unsigned int        u_int;

/******************************
 * End of standard definitons *
 ******************************/

/***********************
 * Defintion of Macros *
 ***********************/

#define UMIN(a, b)      ((a) < (b) ? (a) : (b))
#define IS_ADMIN(dMob)          ((dMob->level) > LEVEL_PLAYER ? TRUE : FALSE)
#define IREAD(sKey, sPtr)             \
{                                     \
  if (!strcasecmp(sKey, word))        \
  {                                   \
    int sValue = fread_number(fp);    \
    sPtr = sValue;                    \
    found = TRUE;                     \
    break;                            \
  }                                   \
}
#define SREAD(sKey, sPtr)             \
{                                     \
  if (!strcasecmp(sKey, word))        \
  {                                   \
    sPtr = fread_string(fp);          \
    found = TRUE;                     \
    break;                            \
  }                                   \
}

/***********************
 * End of Macros       *
 ***********************/

/******************************
 * New structures             *
 ******************************/

/* type defintions */
typedef struct  dSocket       D_SOCKET;
typedef struct  dAccount      D_ACCOUNT;
typedef struct  dMobile       D_MOBILE;
typedef struct  help_data     HELP_DATA;
typedef struct  lookup_data   LOOKUP_DATA;
typedef struct  event_data    EVENT_DATA;
typedef struct  dRoom         D_ROOM;
typedef struct  dExit         D_EXIT;
typedef struct  dArea         D_AREA;
typedef struct  dObject       D_OBJ;
typedef struct  dReset        D_RESET;
typedef struct  npcData       D_NPCDATA;

/* the actual structures */
struct dSocket
{
  D_MOBILE      * player;
  D_ACCOUNT     * account;
  _LIST         * events;
  char          * hostname;
  char            inbuf[MAX_BUFFER];
  char            outbuf[MAX_OUTPUT];
  char            next_command[MAX_BUFFER];
  char            loginName[MAX_BUFFER];
  bool            bust_prompt;
  sh_int          lookup_status;
  int             state;
  sh_int          control;
  sh_int          top_output;
  unsigned char   compressing;                 /* MCCP support */
  z_stream      * out_compress;                /* MCCP support */
  unsigned char * out_compress_buf;            /* MCCP support */
};

struct dAccount
{
   char   *email;
   char   *password;
   char   *characters;
   char   *allCharacters;
   char   *autoLogin;
   bool    acceptANSI; //do they accept ANSI color sequences?
   int     flags;
   int     id;
   time_t  secCreatedDate;
};

struct dMobile
{
   D_SOCKET * socket;
   _LIST    * events;
   D_ROOM   * room;
   
   //////////////
   // NPC Data //
   //////////////
   bool       is_npc;
   D_NPCDATA  *npc_data;
   
   ///////////
   // stats //
   ///////////
   char     * name;
   char     * race;
   char     * prompt;
   int        gender; //0 for female, 1 for male
   usi        level;  //really just if they're a regular player or immortal
   usi        st;     //strength
   usi        in;     //intelligence
   usi        co;     //constitution
   usi        pe;     //perception
   usi        de;     //dexterity
   usi        ch;     //charisma
   usi        lu;     //luck
   int        curHP;  //health points
   int        maxHP;
   int        curMP;  //mana points
   int        maxMP;
   int        curMV;  //movement points/stamina
   int        maxMV;
   
   /////////////
   //equipment//
   /////////////
   D_OBJ     *heldLeft;
   D_OBJ     *heldRight;
   
   /////////////
   //  limbs  //
   /////////////
   
};

struct npcData
{
   int         vnum;
   D_RESET   * reset;
   //more to be added later
};

struct dObject
{
   int         vnum;
   char      * name; //words you can access it by
   char      * sDesc;  //what you see when you're interacting with it
   char      * lDesc; //what you see when it's on the ground
   D_RESET   * reset;
   _LIST     * in;    //objects/mobiles in the object
   _LIST     * on;    //objects/mobiles on the object
   _LIST     * under; //objects/mobiles under the object
};

struct help_data
{
  time_t          load_time;
  char          * keyword;
  char          * text;
  usi             level;
};

struct lookup_data
{
  D_SOCKET       * dsock;   /* the socket we wish to do a hostlookup on */
  char           * buf;     /* the buffer it should be stored in        */
};

struct typCmd
{
  char      * cmd_name;
  void     (* cmd_funct)(D_MOBILE *dMOb, char *arg);
  usi         level;
};

typedef struct buffer_type
{
  char   * data;        /* The data                      */
  int      len;         /* The current len of the buffer */
  int      size;        /* The allocated size of data    */
} BUFFER;

/* here we include external structure headers */
#include "event.h"

/******************************
 * End of new structures      *
 ******************************/

/***************************
 * Global Variables        *
 ***************************/

extern  STACK       *   dsock_free;       /* the socket free list               */
extern  _LIST       *   dsock_list;       /* the linked list of active sockets  */
extern  STACK       *   dmobile_free;     /* the mobile free list               */
extern  _LIST       *   dmobile_list;     /* the mobile list of active mobiles  */
extern  _LIST       *   help_list;        /* the linked list of help files      */
extern  const struct    typCmd tabCmd[];  /* the command table                  */
extern  bool            shut_down;        /* used for shutdown                  */
extern  char        *   greeting;         /* the welcome greeting               */
extern  char        *   motd;             /* the MOTD help file                 */
extern  int             control;          /* boot control socket thingy         */
extern  time_t          current_time;     /* let's cut down on calls to time()  */
extern  _LIST       *   room_list;        /* a list of all the rooms in the MUD */
extern  _LIST       *   area_list;        /* a list of all the areas in the MUD */
extern  _LIST       *   obj_list;         /* master list of all loaded objects  */
extern  _LIST       *   npc_list;         /* master list of all loaded NPCs     */
extern  lua_State   *   globalLState;     /* global Lua state                   */
/*************************** 
 * End of Global Variables *
 ***************************/

/////////////////////////
//    MCCP support     //
/////////////////////////

extern const unsigned char compress_will[];
extern const unsigned char compress_will2[];
extern const unsigned char do_echo[];
extern const unsigned char dont_echo[];

#define TELOPT_COMPRESS        85
#define TELOPT_COMPRESS2       86
#define COMPRESS_BUF_SIZE   16384

/////////////////////////
// End of MCCP support //
/////////////////////////

/////////////////////////////////////
// Prototype function declerations //
/////////////////////////////////////

/* more compact */
#define  D_S         D_SOCKET
#define  D_M         D_MOBILE

#define  buffer_new(size)             __buffer_new     ( size)
#define  buffer_strcat(buffer,text)   __buffer_strcat  ( buffer, text )

char  *crypt                  ( const char *key, const char *salt );

////////////////////
//   accounts.c   //
////////////////////
#include "accounts.h"

////////////////////
//   socket.c     //
////////////////////
#include "socket.h"

////////////////////
//   interpret.c  //
////////////////////
void handle_cmd_input(D_SOCKET *dsock, char *arg);

////////////////////
//   io.c         //
////////////////////
#include "io.h"

////////////////////
//   strings.c    //
////////////////////
#include "strings.h"

////////////////////
//    help.c      //
////////////////////
bool  check_help              ( D_M *dMob, char *helpfile );
void  load_helps              ( void );

////////////////////
//    utils.c     //
////////////////////
#include "utils.h"

////////////////////
// action_safe.c  //
////////////////////
#include "action_safe.h"

////////////////////
//     mccp.c     //
////////////////////
bool  compressStart           ( D_S *dsock, unsigned char teleopt );
bool  compressEnd             ( D_S *dsock, unsigned char teleopt, bool forced );

////////////////////
//     save.c     //
////////////////////
void  save_player             ( D_M *dMob );
D_M  *load_player             ( const char *player );

////////////////////////
//connection_manager.c//
////////////////////////
void handle_new_connections(D_SOCKET *dsock, char *arg);

////////////////////
// mysql_utils.c  //
////////////////////
#include "handler_mysql.h"

////////////////////
// handler_lua.c  //
////////////////////
#include "handler_lua.h"

////////////////////
//     world.c     //
////////////////////
#include "world.h"

/////////////////////////////////
// End of prototype declartion //
/////////////////////////////////

#endif  /* MUD_H */
