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
 * This file handles non-fighting player actions.
 */
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* include main header file */
#include "mud.h"

void cmd_say(D_MOBILE *dMob, char *arg)
{
  if (arg[0] == '\0')
  {
    text_to_mobile(dMob, "Say what?\n\r");
    return;
  }
  communicate(dMob, arg, COMM_LOCAL);
}

void cmd_quit(D_MOBILE *dMob, char *arg)
{
   char buf[MAX_BUFFER];

   /* log the attempt */
   snprintf(buf, MAX_BUFFER, "%s has left the game.", dMob->name);
   text_to_mobile( dMob, "\r\nWe await your return...\r\n" );
   log_string(buf);

   save_player(dMob);

   dMob->socket->player = NULL;
  
  
   if( !strcasecmp( arg, "all" ) )
   {
      close_socket(dMob->socket, FALSE);
      dMob->socket->state = STATE_CLOSED;
      free_mobile(dMob);
      return;
   }
   dMob->socket->state = STATE_MAIN_MENU;
   text_to_buffer( dMob->socket, mainMenu );
   free_mobile(dMob);
   return;
}

void cmd_shutdown(D_MOBILE *dMob, char *arg)
{
   text_to_mobile( dMob, "Ok.\r\n" );
   if( arg[0] != '\0' )
     log_string( arg );
   shut_down = TRUE;
}

void cmd_commands(D_MOBILE *dMob, char *arg)
{
   BUFFER *buf = buffer_new(MAX_BUFFER);
   int i, col = 0;
   int level = atoi( arg );
   if( level < 0 || level > dMob->level )
      level = dMob->level;
   if( arg[0] == '\0' )
      level = dMob->level;

  bprintf(buf, "    - - - - ----==== The full command list ====---- - - - -\n\n\r");
  for (i = 0; tabCmd[i].cmd_name[0] != '\0'; i++)
  {
    if (dMob->level < tabCmd[i].level || tabCmd[i].level > level ) continue;

    bprintf(buf, " %-16.16s", tabCmd[i].cmd_name);
    if (!(++col % 4)) bprintf(buf, "\n\r");
  }
  if (col % 4) bprintf(buf, "\n\r");
  text_to_mobile(dMob, buf->data);
  buffer_free(buf);
}

void cmd_who(D_MOBILE *dMob, char *arg)
{
  UNUSED(arg);
  D_MOBILE *xMob;
  D_SOCKET *dsock;
  ITERATOR Iter;
  text_to_mobile( dMob,
"+-----------------------------------------------------------------------+\r\n"
"|                              WHO LIST                                 |\r\n"
"+-----------------------------------------------------------------------+\r\n" );

  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
  {
    if (dsock->state != STATE_PLAYING) continue;
    if ((xMob = dsock->player) == NULL) continue;

    text_to_mobile(dMob, "|  %-12s                                                         |\n\r", xMob->name );
  }
  DetachIterator(&Iter);

  text_to_mobile( dMob, "+-----------------------------------------------------------------------+\r\n" );

  return;
}

void cmd_help(D_MOBILE *dMob, char *arg)
{
  if (arg[0] == '\0')
  {
    HELP_DATA *pHelp;
    ITERATOR Iter;
    BUFFER *buf = buffer_new(MAX_BUFFER);
    int col = 0;

    bprintf(buf, "      - - - - - ----====//// HELP FILES  \\\\\\\\====---- - - - - -\n\n\r");

    AttachIterator(&Iter, help_list);
    while ((pHelp = (HELP_DATA *) NextInList(&Iter)) != NULL)
    {
      bprintf(buf, " %-19.18s", pHelp->keyword);
      if (!(++col % 4)) bprintf(buf, "\n\r");
    }
    DetachIterator(&Iter);

    if (col % 4) bprintf(buf, "\n\r");
    bprintf(buf, "\n\r Syntax: help <topic>\n\r");
    text_to_mobile(dMob, buf->data);
    buffer_free(buf);

    return;
  }

  if (!check_help(dMob, arg))
    text_to_mobile(dMob, "Sorry, no such helpfile.\n\r");
}

void cmd_compress(D_MOBILE *dMob, char *arg)
{
   UNUSED(arg);
  /* no socket, no compression */
  if (!dMob->socket)
    return;

  /* enable compression */
  if (!dMob->socket->out_compress)
  {
    text_to_mobile(dMob, "Trying compression.\n\r");
    text_to_buffer(dMob->socket, (char *) compress_will2);
    text_to_buffer(dMob->socket, (char *) compress_will);
  }
  else /* disable compression */
  {
    if (!compressEnd(dMob->socket, dMob->socket->compressing, FALSE))
    {
      text_to_mobile(dMob, "Failed.\n\r");
      return;
    }
    text_to_mobile(dMob, "Compression disabled.\n\r");
  }
}

void cmd_save(D_MOBILE *dMob, char *arg)
{
  UNUSED(arg);
  save_player(dMob);
  text_to_mobile(dMob, "Saved.\n\r");
}

void cmd_copyover(D_MOBILE *dMob, char *arg)
{
  UNUSED(arg); 
  FILE *fp;
  ITERATOR Iter;
  D_SOCKET *dsock;
  char buf[MAX_BUFFER];
  
  if ((fp = fopen(COPYOVER_FILE, "w")) == NULL)
  {
    text_to_mobile(dMob, "Copyover file not writeable, aborted.\n\r");
    return;
  }

  strncpy(buf, "\n\r <*>            The world starts spinning             <*>\n\r", MAX_BUFFER);

  /* For each playing descriptor, save its state */
  AttachIterator(&Iter, dsock_list);
  while ((dsock = (D_SOCKET *) NextInList(&Iter)) != NULL)
  {
    compressEnd(dsock, dsock->compressing, FALSE);

    if (dsock->state != STATE_PLAYING)
    {
      text_to_socket(dsock, "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r");
      close_socket(dsock, FALSE);
    }
    else
    {
      fprintf(fp, "%d %s %s\n",
        dsock->control, dsock->player->name, dsock->hostname);

      /* save the player */
      save_player(dsock->player);

      text_to_socket(dsock, buf);
    }
  }
  DetachIterator(&Iter);

  fprintf (fp, "-1\n");
  fclose (fp);

  /* close any pending sockets */
  recycle_sockets();
  
  /*
   * feel free to add any additional arguments between the 2nd and 3rd,
   * that is "SocketMud" and buf, but leave the last three in that order,
   * to ensure that the main() function can parse the input correctly.
   */
  snprintf(buf, MAX_BUFFER, "%d", control);
  execl(EXE_FILE, "SocketMud", buf, "copyover", (char *) NULL);

  /* Failed - sucessful exec will not return */
  text_to_mobile(dMob, "Copyover FAILED!\n\r");
}

void cmd_linkdead(D_MOBILE *dMob, char *arg)
{
  UNUSED(arg); 
  D_MOBILE *xMob;
  ITERATOR Iter;
  char buf[MAX_BUFFER];
  bool found = FALSE;

  AttachIterator(&Iter, dmobile_list);
  while ((xMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
  {
    if (!xMob->socket)
    {
      snprintf(buf, MAX_BUFFER, "%s is linkdead.\n\r", xMob->name);
      text_to_mobile(dMob, buf);
      found = TRUE;
    }
  }
  DetachIterator(&Iter);

  if (!found)
    text_to_mobile(dMob, "Noone is currently linkdead.\n\r");
}

void cmd_score( D_MOBILE *dMob, char *arg )
{
  UNUSED(arg); 
   char buf[MAX_BUFFER];
   
   snprintf( buf, MAX_BUFFER,
            "-- You are %s, level %i %s %s. --\r\n"
            "\r\n"
            "Str: %i\r\n"
            "Int: %i\r\n"
            "Per: %i\r\n"
            "Dex: %i\r\n"
            "Con: %i\r\n"
            "Cha: %i\r\n"
            "Lck: %i\r\n"
            "\r\n"
            "HP: %4i/%-4i  MV: %4i/%-4i  MP: %4i/%-4i\r\n",
            dMob->name, dMob->level,dMob->race,
            dMob->gender == FEMALE ? "woman" : "man", dMob->st,
            dMob->in, dMob->pe, dMob->de, dMob->co, dMob->ch, dMob->lu,
            dMob->curHP, dMob->maxHP, dMob->curMV, dMob->maxMV, dMob->curMP,
            dMob->maxMP );
   
   text_to_mobile( dMob, buf );
   return;
}

void cmd_list_rooms( D_MOBILE *dMob, char *arg )
{
  UNUSED(arg); 
   ITERATOR iter;
   D_ROOM *r;
   
   AttachIterator( &iter, room_list );
   while( ( r = (D_ROOM*)NextInList(&iter) ) != NULL )
   {
      text_to_mobile( dMob, "Room Name: %s\r\nRoom vNum: %i\r\n---\r\n", r->name, r->vnum );
   }
   DetachIterator( &iter );
   
   return;
}

void cmd_list_area( D_MOBILE *dMob, char *arg )
{
  UNUSED(arg); 
   ITERATOR iter;
   D_AREA *area;
   int count = 0;
   
   text_to_mobile( dMob, "Area Name          Author            Low  High\r\n" );
   AttachIterator( &iter, area_list );
   
   while( ( area = (D_AREA *)NextInList( &iter ) ) != NULL )
   {
      text_to_mobile( dMob, "%-16s   %-16s  %-3i  %-3i\r\n",
                      area->name, area->author, area->levelLow, area->levelHi );
      count++;
   }
   
   text_to_mobile( dMob, "Total Areas: %i\r\n", count );
   
   DetachIterator( &iter );
   
   return;
}

void cmd_look( D_MOBILE *dMob, char *arg )
{
   ITERATOR iter;
   D_EXIT *exit;
   D_OBJ  *obj;
   D_MOBILE *vic;
   int count = 0;
   char arg1[MAX_BUFFER];
   
   if( arg[0] == '\0' && dMob->room ) //'look' with no arguments
   {
      text_to_mobile( dMob, "&W%s\r\n&O%s\r\n&WExits:", 
                      dMob->room->name ? dMob->room->name : "UNK",
                      dMob->room->desc ? dMob->room->desc : "UNK" );
      if( !dMob->room->exits )
         text_to_mobile( dMob, "None.\r\n" );
      else
      {
         AttachIterator( &iter, dMob->room->exits );
         while( ( exit = (D_EXIT *) NextInList( &iter ) ) != NULL )
         {
            if( exit->hidden == TRUE )
               continue;
            switch( exit->state )
            {
               default:
               {
                  text_to_mobile( dMob, " %s", exit->name );
                  bug( "Exit \'%s\' has invalid state in room %i.", exit->name, dMob->room->vnum );
                  break;
               }
               case STATE_DOOR_OPEN:
               {
                  text_to_mobile( dMob, " ]%s[", exit->name );
                  break;
               }
               case STATE_DOOR_CLOSED:
               case STATE_DOOR_CLOSED_LOCKED:
               {
                  text_to_mobile( dMob, " [%s]", exit->name );
                  break;
               }
               case STATE_DOOR_OPEN_BROKEN:
               {
                  text_to_mobile( dMob, " }%s{", exit->name );
                  break;
               }
               case STATE_DOOR_CLOSED_BROKEN:
               {
                  text_to_mobile( dMob, " {%s}", exit->name );
                  break;
               }
               case STATE_OPEN:
               {
                  text_to_mobile( dMob, " %s", exit->name );
                  break;
               }
            }
            count++;
         }
      }
      if( count == 0 )
         text_to_mobile( dMob, "  None.\r\n" );
      else
         text_to_mobile( dMob, "\r\n" );
      
      if( dMob->room->objects )
      {
         AttachIterator( &iter, dMob->room->objects );
         while( ( obj = (D_OBJ*)NextInList( &iter ) ) != NULL )
         {
            text_to_mobile( dMob, "%s\r\n", obj->lDesc );
         }
         DetachIterator( &iter );
      }
      
      if( dMob->room->mobiles )
         text_to_mobile( dMob, "%s", list_mobiles( dMob->room->mobiles, "", dMob ) );
      return;
   }// end 'look' with no arguments
   
   arg = one_arg( arg, arg1, TRUE );
   if( !strcasecmp( arg1, "in" ) )
   {
      if( arg[0] == '\0' )
      {
         text_to_mobile( dMob, "Look in what?\r\n" );
      }
      text_to_mobile( dMob, "You look in ..." );
      return;
   }
   
   //'look' with an argument, check to see if it's a mobile in the room
   if( ( vic = find_mob_by_name( dMob->room->mobiles, arg1 ) ) )
   {
      text_to_mobile( dMob, "You look at %s.\r\n", vic->name );
      return;
   }//end 'look' for mobile

   //'look with an argument, check to see if it's an object in the room
   
   text_to_mobile( dMob, "You do not see that here.\r\n" );
   return;
}

void cmd_north( D_MOBILE *dMob, char *arg )
{
  UNUSED(arg); 
   move_mob( dMob, "north" );
}

void cmd_south( D_MOBILE *dMob, char *arg )
{
  UNUSED(arg); 
   move_mob( dMob, "south" );
   return;
}
void cmd_east( D_MOBILE *dMob, char *arg )
{
  UNUSED(arg); 
   move_mob( dMob, "east" );
   return;
}
void cmd_west( D_MOBILE *dMob, char *arg )
{
  UNUSED(arg); 
   move_mob( dMob, "west" );
   return;
}
void cmd_enter( D_MOBILE *dMob, char *arg )
{
  UNUSED(arg); 
   move_mob( dMob, arg );
   return;
}
