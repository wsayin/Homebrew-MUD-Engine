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

const char *mainMenu = "\r\nMain Menu\r\n"
                        "---------\r\n"
                        "   1) Change your account password.\r\n" 
                        "   2) Read the MOTD.\r\n"
                        "   3) Play an existing character.\r\n"
                        "   4) Create a new character.\r\n"
                        "   5) Delete an existing character.\r\n"
                        "   6) Quit Legends of the Darkstone.\r\n"
                        "Which sounds good to you? (1,2,3,4,5,6)\r\n";

void handle_new_connections(D_SOCKET *dsock, char *arg)
{

   switch(dsock->state)
   {
      default:
      {
         bug("Handle_new_connections: Bad state.");
         break;
      }
      case STATE_GET_ACCOUNT:
      {
         if( !strcasecmp( arg, "new" ) )
         {
            text_to_buffer( dsock, "Please enter an e-mail address to create an account: " );
            dsock->state = STATE_NEW_ACCOUNT;
            break;
         }
         snprintf( dsock->loginName, MAX_BUFFER, "%s", arg );
         text_to_buffer( dsock, "Password: " );
         text_to_buffer(dsock, (char *) dont_echo); 
         dsock->state = STATE_ASK_PASSWORD;
         break;
      }
      case STATE_ASK_PASSWORD:
      {
         text_to_buffer(dsock, (char *) do_echo); 
         if( ( dsock->account = load_account( dsock->loginName,arg ) ) == NULL )
         {
            text_to_buffer( dsock, 
               "That username/password combination does not exist.\r\n"
               "Type 'new' to create a new account\r\nLogin: " );
            dsock->state = STATE_GET_ACCOUNT;
            break;
         }
         text_to_buffer( dsock, "Welcome!\r\n" );
         dsock->state = STATE_MAIN_MENU;
         text_to_buffer( dsock, mainMenu );
         break;
      }
      case STATE_NEW_ACCOUNT:
      {
         
         break;
      }
      case STATE_MAIN_MENU:
      {
         switch( atoi(arg) )
         {
            default:
            {
               text_to_buffer( dsock, "Please make a valid selection (1-6).\r\n" );
               text_to_buffer( dsock, mainMenu );
               break;
            }
            case 1:
            {
               text_to_buffer( dsock, "You selected change account password.\r\n" );
               break;
            }
            case 2:
            {
               text_to_buffer(dsock, motd);
               break;
            }
            case 3:
            {
               char buf[MAX_BUFFER];
               char name[MAX_BUFFER];
               char *characters = dsock->account->characters;
               int i = 0;
               text_to_buffer( dsock, "The following characters are associated with this account:\r\n" );
               while ( characters[0] != '\0' )
               {
                  characters = one_arg( characters, name, FALSE );
                  snprintf(buf,MAX_BUFFER, "%d) %-10s", 1+i, name);
                  text_to_buffer( dsock, buf );
                  
                  if ( (++i % 5) == 0 )
                     text_to_buffer( dsock, "\r\n" );
               }
                  
               if ( (i % 5) != 0 )
                  text_to_buffer(dsock, "\r\n" );

               text_to_buffer( dsock, "0) Main Menu\r\nWhich character would you like to play? (0 - %i)\r\n", i );
               dsock->state = STATE_CHOOSE_CHAR;
               
               break;
            }
            case 4:
            {
               text_to_buffer( dsock, "--== Character Creation ==--\r\n" );
               text_to_buffer( dsock, "Choose a Name\r\n"
                                      "Choosing a name is one of the most important parts of this game.\r\n"
                                      "Your choice of a name affects your character, how you can role-play,\r\n"
                                      "and, in general, how other players treat you.\r\n\r\n"
                                      "We ask that all names suit a medieval theme. Names will not be\r\n"
                                      "accepted if they are known to have come from a movie, book, or\r\n"
                                      "popular story. We also ask that you do not use common names,\r\n"
                                      "like those of people you would know in real life. Please also\r\n"
                                      "refrain from using anything you can find in a dictionary.\r\n\r\n"
                                      "If your name is not acceptable, you will be asked to change it.\r\n\r\n"
                                      "Enter the name you would like to be known by, or \"cancel\" to\r\n"
                                      "return to the main menu.\r\n\r\n>" );
               dsock->state = STATE_CHARGEN_NAME;
               break;
            }
            case 5:
            {
               text_to_buffer( dsock, "You selected delete an existing character.\r\n" );
               break;
            }
            case 6:
            {
               text_to_buffer( dsock, "\r\nCome back real soon!\r\n" );
               close_socket(dsock, FALSE);
               return;
            }
         }
         break;
      }
      case STATE_CHOOSE_CHAR:
      {
         char name[MAX_BUFFER];
         char *characters = dsock->account->characters;

         if( isdigit( arg[0] ) )
         {
            if( atoi( arg ) == 0 )
            {
               text_to_buffer( dsock, mainMenu );
               dsock->state = STATE_MAIN_MENU;
               break;
            }
            else
            {
               for( int i = 0; i != atoi( arg ); i++ )
               {
                  characters = one_arg( characters, name, FALSE );
                  //if the selection is out of range
                  if( ( name[0] == '\0' && i < atoi( arg ) ) || atoi(arg) < 0 )
                  {
                     text_to_buffer( dsock, "Please enter a valid number.\r\n" );
                     return;
                  }
               }
               if( ( dsock->player = load_player( name ) ) == NULL )
               {
                  text_to_buffer( dsock, "Your playerfile is either corrupt or missing.\r\nPlease contact an immortal immediately or email the Darkstone Team at %s.\r\n", ADMIN_EMAIL );
                  break;
               }
               log_string("Player: %s has entered the game.", dsock->player->name);
               dsock->player->socket = dsock;
               AttachToList(dsock->player, dmobile_list);
               init_events_player(dsock->player);
               strip_event_socket(dsock, EVENT_SOCKET_IDLE);
               dsock->state = STATE_PLAYING;
               text_to_buffer(dsock, motd);
               cmd_look( dsock->player, "" );
            }
         }
         break;
      }
      case STATE_CHARGEN_NAME:
      {
         if( dsock->player && dsock->player->name )
         {
            free( dsock->player->name );
            free( dsock->player );
         }
         
         if( !strcasecmp( arg, "cancel" ) )
         {
            text_to_buffer( dsock, mainMenu );
            dsock->state = STATE_MAIN_MENU;
            break;
         }
         
         if( !check_name( arg ) )
         {
            text_to_buffer( dsock, "That name is not available. Enter a different name or 'cancel' to return to the main menu.\r\n" );
            break;
         }
         
         if( ( dsock->player = malloc( sizeof( D_MOBILE ) ) ) == NULL )
         {
            bug( "handle_new_connections::STATE_CHARGEN_NAME : Unable to allocate memory for new player." );
            text_to_buffer( dsock, "Sorry, there seems to be some sort of critical memory error. We are unable to accept new characters at this time.\r\n" );
            dsock->state = STATE_CLOSED;
            break;
         }
         dsock->player->name = strdup( arg );
         //snprintf( dsock->player->name, MAX_PC_NAME_LENGTH, "%s", arg );
         text_to_buffer( dsock, "Greetings, %s!\r\n", dsock->player->name );
         text_to_buffer( dsock, "And tell me, is %s the name of a male or a female? (m/f)", dsock->player->name );
         dsock->state = STATE_CHARGEN_GENDER;
         
         break;
      }
      case STATE_CHARGEN_GENDER:
      {
         switch( arg[0] )
         {
            default:
            {
               text_to_buffer( dsock, "[M]ale or [F]emale? \"Cancel\" to quit character creation." );
               break;
            }
            case 'm':
            case 'M':
            {
               dsock->player->gender = 'm';
               break;
            }
            case 'f':
            case 'F':
            {
               dsock->player->gender = 'f';
               break;
            }
            case 'c':
            case 'C':
            {
               text_to_buffer( dsock, mainMenu );
               dsock->state = STATE_MAIN_MENU;
               return;
            }
         }
         dsock->player->level = 4;
         log_string("Player: %s has entered the game.", dsock->player->name);
         dsock->player->socket = dsock;
         AttachToList(dsock->player, dmobile_list);
         AttachToList( dsock->player, dsock->player->room->mobiles );
         init_events_player(dsock->player);
         strip_event_socket(dsock, EVENT_SOCKET_IDLE);
         dsock->state = STATE_PLAYING;
         text_to_buffer(dsock, motd);
         break;
      }
      case STATE_NEW_PASSWORD:
      {
         break;
      }
      case STATE_VERIFY_PASSWORD:
      {
         break;
      }
   }
   return;
}
