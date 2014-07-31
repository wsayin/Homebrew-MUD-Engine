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
 * This file handles command interpreting
 */
#include <sys/types.h>
#include <stdio.h>

/* include main header file */
#include "mud.h"

void next_cmd_from_buffer( D_SOCKET *dsock )
{
   int size = 0, i = 0, j = 0, telopt = 0;

   /* if theres already a command ready, we return */
   if( dsock->next_command[0] != '\0' )
      return;

   /* if there is nothing pending, then return */
   if( dsock->inbuf[0] == '\0' )
      return;

   /* check how long the next command is */
   while( dsock->inbuf[size] != '\0' && dsock->inbuf[size] != '\n' && dsock->inbuf[size] != '\r' )
      size++;

   /* we only deal with real commands */
   if( dsock->inbuf[size] == '\0' )
      return;

   /* copy the next command into next_command */
   for( ; i < size; i++ )
   {
      if( dsock->inbuf[i] == (signed char)IAC )
      {
         telopt = 1;
      }
      else if( telopt == 1 && ( dsock->inbuf[i] == (signed char)DO || dsock->inbuf[i] == (signed char)DONT ) )
      {
         telopt = 2;
      }
      else if( telopt == 2 )
      {
         telopt = 0;

         if( dsock->inbuf[i] == (signed char)TELOPT_COMPRESS ) /* check for version 1 */
         {
            if( dsock->inbuf[i - 1] == (signed char)DO ) /* start compressing   */
               compressStart( dsock, TELOPT_COMPRESS );
            else if( dsock->inbuf[i - 1] == (signed char)DONT ) /* stop compressing    */
               compressEnd( dsock, TELOPT_COMPRESS, FALSE );
         }
         else if( dsock->inbuf[i] == (signed char)TELOPT_COMPRESS2 ) /* check for version 2 */
         {
            if( dsock->inbuf[i - 1] == (signed char)DO ) /* start compressing   */
               compressStart( dsock, TELOPT_COMPRESS2 );
            else if( dsock->inbuf[i - 1] == (signed char)DONT ) /* stop compressing    */
               compressEnd( dsock, TELOPT_COMPRESS2, FALSE );
         }
      }
      else if( isprint( dsock->inbuf[i] ) &&
             ( isprint( dsock->inbuf[i] ) || iscntrl( dsock->inbuf[i] ) ) )
      {
         dsock->next_command[j++] = dsock->inbuf[i];
      }
   }
   dsock->next_command[j] = '\0';

   /* skip forward to the next line */
   while( dsock->inbuf[size] == '\n' || dsock->inbuf[size] == '\r' )
   {
      dsock->bust_prompt = TRUE; /* seems like a good place to check */
      size++;
   }

   /* use i as a static pointer */
   i = size;

   /* move the context of inbuf down */
   while( dsock->inbuf[size] != '\0' )
   {
      dsock->inbuf[size - i] = dsock->inbuf[size];
      size++;
   }
   dsock->inbuf[size - i] = '\0';
}

void handle_cmd_input(D_SOCKET *dsock, char *arg)
{
   D_MOBILE *dMob;
   char command[MAX_BUFFER];
   bool found_cmd = FALSE;
   int i;

   if ((dMob = dsock->player) == NULL)
      return;

   arg = one_arg(arg, command, false);

   for (i = 0; tabCmd[i].cmd_name[0] != '\0' && !found_cmd; i++)
   {
      if (tabCmd[i].level > dMob->level) continue;

      if (is_prefix(command, tabCmd[i].cmd_name))
      {
         found_cmd = TRUE;
         (*tabCmd[i].cmd_funct)(dMob, arg);
      }
   }

   if (!found_cmd)
      text_to_mobile(dMob, "Huh?\n\r");
}

/*
 * The command table, very simple, but easy to extend.
 */
const struct typCmd tabCmd [] =
{

 /* command          function        Req. Level   */
 /* --------------------------------------------- */
  
  //movement
  { "north",         cmd_north,      LEVEL_GUEST  },
  { "south",         cmd_south,      LEVEL_GUEST  },
  { "east",          cmd_east,       LEVEL_GUEST  },
  { "west",          cmd_west,       LEVEL_GUEST  },
  { "enter",         cmd_enter,      LEVEL_GUEST  },
  { "look",          cmd_look,       LEVEL_GUEST  },
  { "say",           cmd_say,        LEVEL_GUEST  },
  { "save",          cmd_save,       LEVEL_GUEST  },
  { "commands",      cmd_commands,   LEVEL_GUEST  },
  { "compress",      cmd_compress,   LEVEL_GUEST  },
  { "copyover",      cmd_copyover,   LEVEL_GOD    },
  { "help",          cmd_help,       LEVEL_GUEST  },
  { "linkdead",      cmd_linkdead,   LEVEL_ADMIN  },
  { "shutdown",      cmd_shutdown,   LEVEL_GOD    },
  { "quit",          cmd_quit,       LEVEL_GUEST  },
  { "who",           cmd_who,        LEVEL_GUEST  },
  { "score",         cmd_score,      LEVEL_GUEST  },
  { "areas",         cmd_list_area,  LEVEL_GUEST  },
  { "rooms",         cmd_list_rooms, LEVEL_GUEST  },

  /* end of table */
  { "", 0, 0 }
};
