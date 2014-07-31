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
 * This file handles string copy/search/comparison/etc.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

/* include main header file */
#include "mud.h"

/*
 * Checks if aStr is a prefix of bStr.
 */
bool is_prefix(const char *aStr, const char *bStr)
{
  /* NULL strings never compares */
  if (aStr == NULL || bStr == NULL) return FALSE;

  /* empty strings never compares */
  if (aStr[0] == '\0' || bStr[0] == '\0') return FALSE;

  /* check if aStr is a prefix of bStr */
  while (*aStr)
  {
    if (tolower(*aStr++) != tolower(*bStr++))
      return FALSE;
  }

  /* success */
  return TRUE;
}

//pulls first word from from fStr and copies it into bStr, returning fStr without bStr
//understands quotes if quotes is TRUE, otherwise ignores quotes
char *one_arg( char *fStr, char *bStr, bool quotes )
{
   char delim = ' ';
   /* skip leading spaces */
   while (isspace(*fStr))
      fStr++; 

   if( quotes == TRUE && *fStr == '\'' )
   {
      delim = '\'';
      fStr++;
   }
   if( quotes == TRUE && *fStr == '\"' )
   {
      delim = '\"';
      fStr++;
   }
   /* copy the beginning of the string */
   while (*fStr != '\0')
   {
      /* have we reached the end of the first word ? */
      if (*fStr == delim)
      {
         fStr++;
         break;
      }

      /* copy one char */
      *bStr++ = *fStr++;
  }

   /* terminate string */
   *bStr = '\0';

   /* skip past any leftover spaces */
   while (isspace(*fStr))
      fStr++;

   /* return the leftovers */
   return fStr;
}

void capitalize(char *text)
{
   for( unsigned int i = 0; i < strlen( text ); i++ )
   {
      text[i] = toupper( text[i] );
   }
   return;
}

void uncapitalize( char *text )
{
   for( unsigned int i = 0; i < strlen( text ); i++ )
   {
      text[i] = tolower( text[i] );
   }
   return;
}

/*  
 * Create a new buffer.
 */
BUFFER *__buffer_new(int size)
{
  BUFFER *buffer;
    
  buffer = malloc(sizeof(BUFFER));
  buffer->size = size;
  buffer->data = malloc(size);
  buffer->len = 0;
  return buffer;
}

/*
 * Add a string to a buffer. Expand if necessary
 */
void __buffer_strcat(BUFFER *buffer, const char *text)  
{
  int new_size;
  int text_len;
  char *new_data;
 
  /* Adding NULL string ? */
  if (!text)
    return;

  text_len = strlen(text);
    
  /* Adding empty string ? */ 
  if (text_len == 0)
    return;

  /* Will the combined len of the added text and the current text exceed our buffer? */
  if ((text_len + buffer->len + 1) > buffer->size)
  { 
    new_size = buffer->size + text_len + 1;
   
    /* Allocate the new buffer */
    new_data = malloc(new_size);
  
    /* Copy the current buffer to the new buffer */
    memcpy(new_data, buffer->data, buffer->len);
    free(buffer->data);
    buffer->data = new_data;  
    buffer->size = new_size;
  }
  memcpy(buffer->data + buffer->len, text, text_len);
  buffer->len += text_len;
  buffer->data[buffer->len] = '\0';
}

/* free a buffer */
void buffer_free(BUFFER *buffer)
{
  /* Free data */
  free(buffer->data);
 
  /* Free buffer */
  free(buffer);
}

/* Clear a buffer's contents, but do not deallocate anything */
void buffer_clear(BUFFER *buffer)
{
  buffer->len = 0;
  buffer->data[0] = '\0';
}

/* print stuff, append to buffer. safe. */
int bprintf(BUFFER *buffer, char *fmt, ...)
{  
  char buf[MAX_BUFFER];
  va_list va;
  int res;
    
  va_start(va, fmt);
  res = vsnprintf(buf, MAX_BUFFER, fmt, va);
  va_end(va);
    
  if (res >= MAX_BUFFER - 1)  
  {
    buf[0] = '\0';
    bug("Overflow when printing string %s", fmt);
  }
  else
    buffer_strcat(buffer, buf);
   
  return res;
}

char *strdup(const char *s)
{
  char *pstr;
  int len;
  if( !s )
   return NULL;

  len = strlen(s) + 1;
  pstr = (char *) calloc(1, len);
  strcpy(pstr, s);

  return pstr;
}

char *escape_string( const char *s )
{
   static char buf[MAX_STRING_LENGTH *2 ];//so we can make sure we can fit all escape strings
   unsigned int i, j;
   if( !s )
      return NULL;
   
   for( i = 0, j = 0; i < strlen( s ); i++, j++ )
   {
      switch( s[i] )
      {
         default:
         {
            buf[j] = s[i];
            break;
         }
         case '\''://put all of the characters you want to escape with a \ here
         case '"':
         case '\\':
         {
            buf[j] = '\\';
            j++;
            buf[j] = s[i];
            break;
         }
         case '\r':
         {
            buf[j] = '\\';
            j++;
            buf[j] = 'r';
            break;
         }
         case '\n':
         {
            buf[j] = '\\';
            j++;
            buf[j] = 'n';
            break;
         }
      }
   }
   
   return buf;
}

int strcasecmp(const char *s1, const char *s2)
{
  int i = 0;

  while (s1[i] != '\0' && s2[i] != '\0' && toupper(s1[i]) == toupper(s2[i]))
    i++;

  /* if they matched, return 0 */
  if (s1[i] == '\0' && s2[i] == '\0')
    return 0;

  /* is s1 a prefix of s2? */
  if (s1[i] == '\0')
    return -110;

  /* is s2 a prefix of s1? */
  if (s2[i] == '\0')
    return 110;

  /* is s1 less than s2? */
  if (toupper(s1[i]) < toupper(s2[i]))
    return -1;

  /* s2 is less than s1 */
  return 1;
}


//word wrapping function, pretty basic so far, need to add in conditional
//checking for hyphen's (-) and paragraph breaks
char *word_wrap( char *s, unsigned int l )
{
   static char buf[MAX_OUTPUT*2] = {'\0'};
   char *pch = strtok( s, " \r\n" );
   int len = 0;
   
   while( pch != NULL )
   {
      if( len + strlen( pch ) >= l )
         strncat( buf, "\r\n", (MAX_OUTPUT*2)-strlen(buf) );
      strncat( buf, pch, (MAX_OUTPUT*2)-strlen(buf) );
      strncat( buf, " ", (MAX_OUTPUT*2)-strlen(buf) );
      len += strlen( pch );
      pch = strtok( NULL, " " );
   }
   return buf;
}
