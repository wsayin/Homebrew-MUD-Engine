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
 /*
 * This file contains all sorts of utility functions used
 * all sorts of places in the code.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

/* include main header file */
#include "mud.h"
#include "rand.h"

/*
 * Check to see if a given name is
 * legal, returning FALSE if it
 * fails our high standards...
 */
bool check_name( char *name )
{
   int size, i;
   FILE *fp;
   char fn[MAX_BUFFER];
   char upper[MAX_BUFFER];

   if ((size = strlen(name)) < 3 || size > 16)
      return FALSE;
      
   name[0] = toupper( name[0] );
   
   strcpy( upper, name );
   capitalize( upper );
   
   for (i = 0 ;i < size; i++)
      if (!isalpha(name[i])) return FALSE;
   
   if( strstr( upper, "FUCK" )   ||
       strstr( upper, "SHIT" )   ||
       strstr( upper, "BITCH" )  ||
       strstr( upper, "WHORE" )  ||
       strstr( upper, "ASS" )    ||
       strstr( upper, "DAMN" )   ||
       strstr( upper, "CUNT" )   ||
       strstr( upper, "STUPID" ) ||
       strstr( upper, "DUMB" )   ||
       strstr( upper, "OSAMA" )  ||
       strstr( upper, "OBAMA" )  ||
       strstr( upper, "PUSSY" )  ||
       strstr( upper, "DICK" )   )
       return FALSE;
   
   //If the name is already in use
   snprintf( fn, MAX_BUFFER, "../players/%c/%s.dat", name[0], name );
   if( ( fp = fopen( fn, "r" ) ) != NULL )
   {
      fclose( fp );
      return FALSE;
   }

   return TRUE;
}

D_MOBILE *new_mobile()
{
   D_MOBILE *m;
   
   if( ( m = malloc( sizeof( D_MOBILE ) ) ) == NULL )
      return NULL;
   
   clear_mobile( m );
   return m;
}

void clear_mobile(D_MOBILE *dMob)
{
  memset(dMob, 0, sizeof(*dMob));

  dMob->name         =  NULL;
  dMob->prompt       =  NULL;
  dMob->gender       =  FEMALE;
  dMob->level        =  1;
  dMob->room         =  NULL;
  dMob->events       =  AllocList();
  dMob->heldLeft     =  NULL;
  dMob->heldRight    =  NULL;
  dMob->is_npc       =  FALSE;
  
  //Defaults--feel free to change them
  dMob->curHP        =  20;
  dMob->maxHP        =  20;
  dMob->curMV        =  20;
  dMob->maxMV        =  20;
  dMob->curMP        =  20;
  dMob->maxMP        =  20;
  
  dMob->st           =  6; //Players have a total of 47 points they can spend
  dMob->in           =  6; //6 in each will total 42, which gives 5 extra points
  dMob->co           =  6; //to distribute, plus the others to subtract and
  dMob->pe           =  6; //rearrange as they see fit.
  dMob->de           =  6;
  dMob->ch           =  6;
  dMob->lu           =  6;
}

void free_mobile(D_MOBILE *dMob)
{
   EVENT_DATA *pEvent;
   ITERATOR Iter;

   DetachFromList(dMob, dmobile_list);

   if (dMob->socket) dMob->socket->player = NULL;

   AttachIterator(&Iter, dMob->events);
   while ((pEvent = (EVENT_DATA *) NextInList(&Iter)) != NULL)
      dequeue_event(pEvent);
   DetachIterator(&Iter);
   FreeList(dMob->events);

   /* free allocated memory */
   if( dMob->name )
      free(dMob->name);
   if( dMob->prompt )
      free( dMob->prompt );
   if( dMob->room )
      DetachFromList( dMob, dMob->room->mobiles );

   PushStack(dMob, dmobile_free);
}

/*
 * Loading of help files, areas, etc, at boot time.
 */
void load_muddata(bool fCopyOver)
{  
   load_helps();
   load_world();
   /* copyover */
   if (fCopyOver)
      copyover_recover();
}

char *get_time()
{
  static char buf[16];
  char *strtime;
  int i;

  strtime = ctime(&current_time);
  for (i = 0; i < 15; i++)   
    buf[i] = strtime[i + 4];
  buf[15] = '\0';

  return buf;
}

/* Recover from a copyover - load players */
void copyover_recover()
{     
  D_MOBILE *dMob;
  D_SOCKET *dsock;
  FILE *fp;
  char name [100];
  char host[MAX_BUFFER];
  int desc;
      
  log_string("Copyover recovery initiated");
   
  if ((fp = fopen(COPYOVER_FILE, "r")) == NULL)
  {  
    log_string("Copyover file not found. Exitting.");
    exit (1);
  }
      
  /* In case something crashes - doesn't prevent reading */
  unlink(COPYOVER_FILE);
    
  for (;;)
  {  
    fscanf(fp, "%d %s %s\n", &desc, name, host);
    if (desc == -1)
      break;

    dsock = malloc(sizeof(*dsock));
    clear_socket(dsock, desc);
  
    dsock->hostname     =  strdup(host);
    AttachToList(dsock, dsock_list);
 
    /* load player data */
    if ((dMob = load_player(name)) != NULL)
    {
      /* attach to socket */
      dMob->socket     =  dsock;
      dsock->player    =  dMob;
  
      /* attach to mobile list */
      AttachToList(dMob, dmobile_list);

      /* initialize events on the player */
      init_events_player(dMob);
    }
    else /* ah bugger */
    {
      close_socket(dsock, FALSE);
      continue;
    }
   
    /* Write something, and check if it goes error-free */
    if (!text_to_socket(dsock, "\n\r <*>  And before you know it, everything has changed  <*>\n\r"))
    { 
      close_socket(dsock, FALSE);
      continue;
    }
  
    /* make sure the socket can be used */
    dsock->bust_prompt    =  TRUE;
    dsock->lookup_status  =  TSTATE_DONE;
    dsock->state          =  STATE_PLAYING;

    /* negotiate compression */
    text_to_buffer(dsock, (char *) compress_will2);
    text_to_buffer(dsock, (char *) compress_will);
  }
  fclose(fp);
}     

D_MOBILE *check_reconnect(char *player)
{
  D_MOBILE *dMob;
  ITERATOR Iter;

  AttachIterator(&Iter, dmobile_list);
  while ((dMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
  {
    if (!strcasecmp(dMob->name, player))
    {
      if (dMob->socket)
        close_socket(dMob->socket, TRUE);

      break;
    }
  }
  DetachIterator(&Iter);

  return dMob;
}

D_ROOM *new_room()
{
   D_ROOM *r;
   
   if( ( r = malloc( sizeof( D_ROOM ) ) ) == NULL )
      return NULL;
   
   r->area = NULL;
   r->name = NULL;
   r->desc = NULL;
   r->vnum = 0;
   r->mobiles = AllocList();
   r->objects = AllocList();
   r->exits = AllocList();
   r->resets = AllocList();
   
   AttachToList( r, room_list );
   
   return r;
}

void free_room( D_ROOM *room )
{
   if( !room )
      return;
   if( room->name )
      free( room->name );
   if( room->desc )
      free( room->desc );
   if( room->exits )
      FreeList( room->exits );
   if( room->objects )
      FreeList( room->objects );
   if( room->mobiles )
      FreeList( room->mobiles );
   free( room );
   return;
}

D_AREA *new_area()
{
   D_AREA *a;
   
   if( ( a = malloc( sizeof( D_AREA ) ) ) == NULL )
      return NULL;
   
   a->name = NULL;
   a->filename = NULL;
   a->author = NULL;
   a->levelLow = 0;
   a->levelHi = 0;
   a->mLow = 0;
   a->mHi  = 0;
   a->rLow = 0;
   a->rHi  = 0;
   a->oLow = 0;
   a->oHi  = 0;
   a->rooms   = AllocList();
   a->objects = AllocList();
   a->mobiles = AllocList();
   
   AttachToList( a, area_list );
   
   return a;
}

void free_area( D_AREA *area )
{
   if( !area )
      return;
   if( area->name )
      free( area->name );
   if( area->filename )
      free( area->filename );
   if( area->author )
      free( area->author );
   if( area->rooms )
      FreeList( area->rooms );
   if( area->objects )
      FreeList( area->objects );
   if( area->mobiles )
      FreeList( area->mobiles );
   free( area );
   area = NULL;
   return;
}

D_EXIT *new_exit()
{
   D_EXIT *exit;
   if( ( exit = malloc( sizeof( D_EXIT ) ) ) == NULL )
      return NULL;
   exit->name = NULL;
   exit->desc = NULL;
   exit->state = STATE_OPEN;
   exit->keyID = -1;
   exit->dest = NULL;
   exit->hidden = FALSE;
   return exit;
}

void free_exit( D_EXIT *exit )
{
   if( !exit )
      return;
   if( exit->name )
      free( exit->name );
   if( exit->desc )
      free( exit->desc );
   //we don't worry about freeing exit->dest because its destination will eventually be freed by free_area
   exit = NULL;
   return;
}

D_OBJ *new_object()
{
   D_OBJ *o;
   if( ( o = malloc( ( sizeof( D_OBJ ) ) ) ) == NULL ) 
      return NULL;
      
   o->name = NULL;
   o->sDesc = NULL;
   o->lDesc = NULL;
   o->vnum  = 0;
   
   o->in = AllocList();
   o->on = AllocList();
   o->under = AllocList();
   
   return o;
}

void free_object( D_OBJ *o )
{
   if( !o )
      return;
   
   if( o->name ) 
      free( o->name );
   if( o->sDesc )
      free( o->sDesc );
   if( o->lDesc )
      free( o->lDesc );
   if( o->reset )
      o->reset->ptr = NULL;
   if( o )
      free( o );
   
   o = NULL;
   return;
}

D_RESET *new_reset()
{
   D_RESET *r;
   if( ( r = malloc( ( sizeof( D_RESET ) ) ) ) == NULL )
      return NULL;
   
   r->type = 0;
   r->vnum = 0;
   r->destVnum = 0;
   r->nest = AllocList();
   
   return r;
}

void free_reset( D_RESET *r )
{
   if( r )
      free( r );
   r = NULL;
   return;
}

D_MOBILE *fmbv( int vnum ) //find NPC by vnum
{
   ITERATOR iter;
   D_MOBILE *m;
   
   AttachIterator( &iter, npc_list );
   while( ( m = (D_MOBILE*) NextInList( &iter ) ) != NULL )
   {
      if( m->npc_data && m->npc_data->vnum == vnum )
         return m;
   }
   
   return NULL;
}

D_OBJ *fobv( int vnum ) //find object by vnum
{
   ITERATOR iter;
   D_OBJ *o;
   
   AttachIterator( &iter, obj_list );
   while( ( o = (D_OBJ*) NextInList( &iter ) ) != NULL )
   {
      if( o->vnum == vnum )
         return o;
   }
   
   return NULL;
}

D_ROOM *frbv( int vnum ) //find room by vnum
{
   ITERATOR iter;
   D_ROOM *room;
   
   AttachIterator( &iter, room_list );
   while( (room = (D_ROOM *) NextInList(&iter) ) != NULL )
   {
      if( room->vnum == vnum )
         break;
   }
   DetachIterator( &iter );
   return room;
}

D_EXIT *febn( const char *name, _LIST *exits ) //find exit by name ("north" etc )
{
   ITERATOR iter;
   D_EXIT *exit = NULL;
   
   if( !exits )
   {
      bug( "Error: Function febn requires a valid exits list." );
      return NULL;
   }
   if( !name )
      return NULL;
   
   AttachIterator( &iter, exits );
   while( ( exit = (D_EXIT *) NextInList( &iter ) ) != NULL )
   {
      if( exit->name && !strcasecmp( exit->name, name ) )
         break;
   }
   DetachIterator( &iter );
   return exit;
}

void rseed() //seed the RNG
{
   FILE *fp;
   uint32_t seed;
   
   if( ( fp = fopen( "/dev/random", "r" ) ) == NULL )
   {
      bug( "Warning: Unable to open /dev/random, time() for rand seed" );
      init_rand( (uint32_t)time(NULL) );
      return;
   }
   
   fread( &seed, sizeof(uint32_t), 1, fp ); //read a random number from /dev/random
   init_rand( (uint32_t)seed ); //use it to seed the random number generator
   return;
}

//returns random number in range low to hi
int rrange( int low, int hi )
{
   return ( (int)rand_cmwc() % hi + low );
}

//roll numDie number of size sizeDie die
int roll( int numDie, int sizeDie )
{
   int val = 0, i = 0;
   
   for( i = 0; i < numDie; i++ )
   {
      val += (int)rand_cmwc() % sizeDie + 1;
   }
   
   return val;
}

//random percent from 1 to 100
int rpercent()
{
   return rrange( 1, 100 );
}

D_OBJ *clone_obj( const D_OBJ *obj )
{
   D_OBJ *clone;
   
   if( !obj )
      return NULL;
   
   if( ( clone = new_object() ) == NULL )
      return NULL;
   clone->vnum = obj->vnum;
   clone->name = strdup( obj->name );
   clone->sDesc = strdup( obj->sDesc );
   clone->lDesc = strdup( obj->lDesc );
   
   return clone;
}

D_MOBILE *clone_npc( const D_MOBILE *m )
{
   D_MOBILE *clone;
   
   if( !m )
      return NULL;
   if( m->is_npc == FALSE )
   {
      bug( "Attempting to clone a playing character!" );
      return NULL;
   }
   if( !m->npc_data )
   {
      bug( "NPC has no npc_data!" );
      return NULL;
   }
   
   if( ( clone = new_npc() ) == NULL )
      return NULL;
   memcpy( clone, m, sizeof( D_MOBILE ) );
   clone->name = strdup( m->name );
   clone->race = strdup( m->race );
   clone->is_npc = TRUE;
   clone->npc_data->vnum = m->npc_data->vnum;
   
   return clone;
}

//print the list with the preceding tab, excluding the mobile pointed to by except
const char *list_mobiles( _LIST *l, const char *tab, const D_M *except )
{
   ITERATOR i;
   D_MOBILE *m;
   static char buf[MAX_BUFFER];
   char *point = buf;
   
   *point = '\0'; //for some reason the buffer kept holding on to data from other strings
   
   AttachIterator( &i, l );
   while( ( m = (D_MOBILE*)NextInList(&i) ) != NULL )
   {
      if( m == except )
         continue;
      point += snprintf( point, MAX_BUFFER - strlen(buf), "%s%s\r\n", tab ? tab : "\0", m->name );
   }
   DetachIterator( &i );
   
   return buf;
}

static D_NPCDATA *new_npcdata()
{
   D_NPCDATA *data;
   if( ( data = malloc( sizeof( D_NPCDATA ) ) ) == NULL )
      return NULL;
   data->reset = NULL;
   return data;
}

static void free_npc_data( D_NPCDATA *data )
{
   if( data->reset )
      data->reset->ptr = NULL;
   free( data );
   return;
}


D_MOBILE *new_npc()
{
   D_MOBILE *npc;
   D_NPCDATA *data;
   
   if( ( data = new_npcdata() ) == NULL )
      return NULL;
   if( ( npc = new_mobile() ) == NULL )
   {
      free_npc_data( data );
      return NULL;
   }
   npc->npc_data = data;
   npc->is_npc = TRUE;
   AttachToList( npc, npc_list );
   
   return npc;
}

void free_npc( D_MOBILE *npc )
{
   if( !npc )
   {
      bug( "Attempting to free NULL NPC!" );
      return;
   }
   if( npc->is_npc == FALSE )
   {
      bug( "Attempting to call free_npc on a player character." );
      return;
   }
   if( !npc->npc_data )
      bug( "Calling free_npc on NPC with no npc_data." );
   
   free_npc_data( npc->npc_data );
   free_mobile( npc );
   return;
}

D_MOBILE *find_mob_by_name( _LIST *l, char *name )
{
   ITERATOR iter;
   D_MOBILE *m;
   
   if( !l || !name )
      return NULL;
      
   AttachIterator( &iter, l );
   while( ( m = (D_MOBILE*)NextInList( &iter ) ) != NULL )
   {
      if( is_name( name, m->name ) )
         return m;
   }
   DetachIterator( &iter );
   return NULL;
}

bool is_name( char *name, char *string )
{
   char try[MAX_BUFFER];
   
   while( string[0] != '\0' )
   {
      string = one_arg( string, try, TRUE );
      if( is_prefix( name, try ) )
         return true;
   }
   return false;
}
