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

/*
 * Text_to_socket()
 *
 * Sends text directly to the socket,
 * will compress the data if needed.
 */
 
static char *parse_color_codes( char *msg );
static char *strip_color_codes( char *msg );
static char *parse_prompt( D_MOBILE *dMob );

bool text_to_socket( D_SOCKET *dsock, const char *txt2, ... )
{
   int iBlck, iPtr, iWrt = 0, length, control = dsock->control;
   va_list args;
   char buf[MAX_BUFFER], *txt = buf;

   va_start( args, txt2 );
   vsnprintf( buf, MAX_BUFFER, txt2, args );
   va_end( args );

   length = strlen( txt );

   /* write compressed */
   if( dsock && dsock->out_compress )
   {
      dsock->out_compress->next_in = (unsigned char *)txt;
      dsock->out_compress->avail_in = length;

      while( dsock->out_compress->avail_in )
      {
         dsock->out_compress->avail_out = COMPRESS_BUF_SIZE - ( dsock->out_compress->next_out - dsock->out_compress_buf );

         if( dsock->out_compress->avail_out )
         {
            int status = deflate( dsock->out_compress, Z_SYNC_FLUSH );

            if( status != Z_OK )
               return FALSE;
         }

         length = dsock->out_compress->next_out - dsock->out_compress_buf;
         if( length > 0 )
         {
            for( iPtr = 0; iPtr < length; iPtr += iWrt )
            {
               iBlck = UMIN( length - iPtr, 4096 );
               if( ( iWrt = write( control, dsock->out_compress_buf + iPtr, iBlck ) ) < 0 )
               {
                  perror( "Text_to_socket (compressed):" );
                  return FALSE;
               }
            }
            if( iWrt <= 0 )
               break;
            if( iPtr > 0 )
            {
               if( iPtr < length )
                  memmove( dsock->out_compress_buf, dsock->out_compress_buf + iPtr, length - iPtr );

               dsock->out_compress->next_out = dsock->out_compress_buf + length - iPtr;
            }
         }
      }
      return TRUE;
   }

   /* write uncompressed */
   for( iPtr = 0; iPtr < length; iPtr += iWrt )
   {
      iBlck = UMIN( length - iPtr, 4096 );
      if( ( iWrt = write( control, txt + iPtr, iBlck ) ) < 0 )
      {
         perror( "Text_to_socket:" );
         return FALSE;
      }
   }

   return TRUE;
}

/*
 * Text_to_buffer()
 *
 * Stores outbound text in a buffer, where it will
 * stay untill it is flushed in the gameloop.
 *
 * Will also parse ANSI colors and other tags.
 */
void text_to_buffer( D_SOCKET *dsock, const char *txt2, ... )
{
   static char output[8 * MAX_BUFFER];
   char buf[MAX_BUFFER];
   va_list args;

   va_start( args, txt2 );
   vsnprintf( buf, MAX_BUFFER, txt2, args );
   va_end( args );

   /* always start with a leading space */
   if( dsock->top_output == 0 )
   {
      dsock->outbuf[0] = '\n';
      dsock->outbuf[1] = '\r';
      dsock->top_output = 2;
   }
      snprintf( output, MAX_BUFFER * 8, "%s", buf );

   /* check to see if the socket can accept that much data */
   if( dsock->top_output + strlen(output) >= MAX_OUTPUT )
   {
      bug( "Text_to_buffer: ouput overflow on %s.", dsock->hostname );
      return;
   }

   /* add data to buffer */
   strncat( dsock->outbuf, output, MAX_OUTPUT - strlen( dsock->outbuf ) -1 );
   dsock->top_output = strlen( dsock->outbuf );
}

/*
 * Text_to_mobile()
 *
 * If the mobile has a socket, then the data will
 * be send to text_to_buffer().
 */
void text_to_mobile( D_MOBILE *dMob, const char *txt2, ... )
{
   va_list args;
   char buf[MAX_BUFFER], *txt = buf;

   va_start( args, txt2 );
   vsnprintf( buf, MAX_BUFFER, txt2, args );
   va_end( args );
   buf[0] = toupper( buf[0] ); //sentences always begin with a capital letter...

   if( dMob->socket )
   {
      text_to_buffer( dMob->socket, txt );
      dMob->socket->bust_prompt = TRUE;
   }
}

void communicate(D_MOBILE *dMob, char *txt, int range)
{
  D_MOBILE *xMob;
  ITERATOR Iter;
  char buf[MAX_BUFFER];
  char message[MAX_BUFFER];

  switch(range)
  {
    default:
      bug("Communicate: Bad Range %d.", range);
      return;
    case COMM_LOCAL:  /* everyone is in the same room for now... */
      snprintf(message, MAX_BUFFER, "&o%s says '%s'.\n\r", dMob->name, txt);
      snprintf(buf, MAX_BUFFER, "&oYou say '%s'.\n\r", txt);
      text_to_mobile(dMob, buf);
      AttachIterator(&Iter, dmobile_list);
      while ((xMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
      {
        if (xMob == dMob) continue;
        text_to_mobile(xMob, message);
      }
      DetachIterator(&Iter);
      break;
    case COMM_LOG:
      snprintf(message, MAX_BUFFER, "&p[LOG: %s]\n\r", txt);
      AttachIterator(&Iter, dmobile_list);
      while ((xMob = (D_MOBILE *) NextInList(&Iter)) != NULL)
      {
        if (!IS_ADMIN(xMob)) continue;
        text_to_mobile(xMob, message);
      }
      DetachIterator(&Iter);
      break;
  }
}

bool flush_output( D_SOCKET *dsock )
{
   /* nothing to send */
   if( dsock->outbuf[0] == '\0' && !( dsock->bust_prompt && dsock->state == STATE_PLAYING ) )
      return TRUE;

   /* bust a prompt */
   if( dsock->state == STATE_PLAYING && dsock->bust_prompt )
   {
      text_to_buffer( dsock, "\r\n\r\n%s", parse_prompt( dsock->player ) );
      dsock->bust_prompt = FALSE;
   }

   /*
    * Send the buffer, and return FALSE
    * if the write fails.
    */
   if( dsock->account && dsock->account->acceptANSI == TRUE )
      text_to_socket( dsock, "%s\r\n", parse_color_codes( dsock->outbuf ) );
   else if( dsock->account && dsock->account->acceptANSI == FALSE )
      text_to_socket( dsock, "%s\r\n",  strip_color_codes( dsock->outbuf ) );
   else
      text_to_socket( dsock, "%s\r\n",  dsock->outbuf );

   /* reset the top pointer */
   dsock->outbuf[0] = '\0';
   /* Success */
   return TRUE;
}
static char *strip_color_codes( char *msg )
{
   static char result[MAX_BUFFER*2];
   char *start;
   char *orig;
   
   start = result;
   orig  = msg;

   for( ; *orig; orig++ )
   {
      if( *orig == '&' ) //background color
      {
         orig++;
         switch( *orig )
         {
            case 'x': //xrgoybpcw
            case 'r':
            case 'g':
            case 'o':
            case 'y':
            case 'b':
            case 'p':
            case 'c':
            case 'w':
            case 'X': //xrgoybpcw
            case 'R':
            case 'G':
            case 'O':
            case 'Y':
            case 'B':
            case 'P':
            case 'C':
            case 'W':
               orig++;
               break;
            default:
               start += sprintf( start, "&%c", *orig );
               orig++;
               break;
         }
      }
      if( *orig == '^' ) //background color
      {
         orig++;
         switch( *orig )
         {
            case 'x': //xrgoybpcw
            case 'r':
            case 'g':
            case 'o':
            case 'y':
            case 'b':
            case 'p':
            case 'c':
            case 'w':
            case 'X':
            case 'R':
            case 'G':
            case 'O':
            case 'Y':
            case 'B':
            case 'P':
            case 'C':
            case 'W':
               orig++;
               break;
            default:
               start += sprintf( start, "^%c", *orig );
               orig++;
               break;
         }
      }
      else
      {
         *start = *orig;
         start++;
      }
   }
   *start = '\0';
   strcat( start, "\033[0m" );
   return result;
}
static char *parse_color_codes( char *msg )
{
   static char result[MAX_BUFFER*2];
   char *start;
   char *orig;
   
   start = result;
   orig  = msg;

   for( ; *orig; orig++ )
   {
      if( *orig == '&' ) //background color
      {
         orig++;
         switch( *orig )
         {
            case 'x': //normal/dull colors
               start += sprintf( start, "\033[0m\033[30m" );
               break;
            case 'r':
               start += sprintf( start, "\033[0m\033[31m" );
               break;
            case 'g':
               start += sprintf( start, "\033[0m\033[32m" );
               break;
            case 'o':
            case 'y':
               start += sprintf( start, "\033[0m\033[33m" );
               break;
            case 'b':
               start += sprintf( start, "\033[0m\033[34m" );
               break;
            case 'p':
               start += sprintf( start, "\033[0m\033[35m" );
               break;
            case 'c':
               start += sprintf( start, "\033[0m\033[36m" );
               break;
            case 'w':
               start += sprintf( start, "\033[0m\033[37m" );
               break;
            case 'X': //bright/light colors
               start += sprintf( start, "\033[0m\033[1;30m" );
               break;
            case 'R':
               start += sprintf( start, "\033[0m\033[1;31m" );
               break;
            case 'G':
               start += sprintf( start, "\033[0m\033[1;32m" );
               break;
            case 'O':
            case 'Y':
               start += sprintf( start, "\033[0m\033[1;33m" );
               break;
            case 'B':
               start += sprintf( start, "\033[0m\033[1;34m" );
               break;
            case 'P':
               start += sprintf( start, "\033[0m\033[1;35m" );
               break;
            case 'C':
               start += sprintf( start, "\033[0m\033[1;36m" );
               break;
            case 'W':
               start += sprintf( start, "\033[0m\033[1;37m" );
               break;
            default:
               start += sprintf( start, "&%c", *orig );
               break;
         }
      }
      else if( *orig == '^' ) //foreground color
      {
         orig++;
         switch( *orig )
         {
            case 'x': //normal/dull colors
               start += sprintf( start, "\033[0m\033[40m" );
               break;
            case 'r':
               start += sprintf( start, "\033[0m\033[41m" );
               break;
            case 'g':
               start += sprintf( start, "\033[0m\033[42m" );
               break;
            case 'o':
            case 'y':
               start += sprintf( start, "\033[0m\033[43m" );
               break;
            case 'b':
               start += sprintf( start, "\033[0m\033[44m" );
               break;
            case 'p':
               start += sprintf( start, "\033[0m\033[45m" );
               break;
            case 'c':
               start += sprintf( start, "\033[0m\033[46m" );
               break;
            case 'w':
               start += sprintf( start, "\033[0m\033[47m" );
               break;
            case 'X':  //bright/light colors
               start += sprintf( start, "\033[0m\033[1;40m" );
               break;
            case 'R':
               start += sprintf( start, "\033[0m\033[1;41m" );
               break;
            case 'G':
               start += sprintf( start, "\033[0m\033[1;42m" );
               break;
            case 'O':
            case 'Y':
               start += sprintf( start, "\033[0m\033[1;43m" );
               break;
            case 'B':
               start += sprintf( start, "\033[0m\033[1;44m" );
               break;
            case 'P':
               start += sprintf( start, "\033[0m\033[1;45m" );
               break;
            case 'C':
               start += sprintf( start, "\033[0m\033[1;46m" );
               break;
            case 'W':
               start += sprintf( start, "\033[0m\033[1;47m" );
               break;
            default:
               start += sprintf( start, "^%c", *orig );
               break;
         }
      }
      else
      {
         *start = *orig;
         start++;
      }
   }
   *start = '\0';
   strcat( start, "\033[0m" );
   return result;
}

static char *parse_prompt( D_MOBILE *dMob )
{
   static char result[MAX_BUFFER * 2 ];
   char *orig, *start;
   
   if( !dMob->prompt )
      dMob->prompt = strdup( "<&Y$h&W/&Y$Hhp &G$v&W/&G$Vmv &C$m&W/&C$Mm&W>" );
      
   start = result;
   orig  = dMob->prompt;
   
   for( ; *orig; orig++ )
   {
      if( *orig == '$' ) //background color
      {
         orig++;
         switch( *orig )
         {
            case 'h':
               start += sprintf( start, "%i", dMob->curHP );
               break;
            case 'H':
               start += sprintf( start, "%i", dMob->maxHP );
               break;
            case 'v':
               start += sprintf( start, "%i", dMob->curMV );
               break;
            case 'V':
               start += sprintf( start, "%i", dMob->maxMV );
               break;
            case 'm':
               start += sprintf( start, "%i", dMob->curMP );
               break;
            case 'M':
               start += sprintf( start, "%i", dMob->maxMP );
               break;
            default:
               start += sprintf( start, "&%c", *orig );
               break;
         }
      }
      else
      {
         *start = *orig;
         start++;
      }
   }
   *start = '\0';
   
   return result;
}
