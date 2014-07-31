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
#include <mysql.h>
#include <time.h>
#include "mud.h"


MYSQL *sql_con;

#define MRES(con,des,src) mysql_real_escape_string( con, des, src, strlen( src ) )
#define B(x) fprintf( stderr, "\n\n::ALERT:: %s\n\n", x )

char *MYSQL_SERVER_ADDRESS;
char *MYSQL_SERVER_PASS;
char *MYSQL_SERVER_LOGIN;
char *MYSQL_DEFAULT_DB;

/*
void save_account( ACCOUNT_DATA *acct )
{
   char query[MAX_INPUT_LENGTH];
   char eName[MAX_INPUT_LENGTH];
   char eChars[MAX_INPUT_LENGTH];
   char eAChars[MAX_INPUT_LENGTH];
   char eAutoLogin[MAX_INPUT_LENGTH];
   
   
   MRES( sql_con, eName, acct->email );
   MRES( sql_con, eChars, acct->characters );
   MRES( sql_con, eAChars, acct->allCharacters );
   MRES( sql_con, eAutoLogin, acct->autoLoginName_.c_str() );
   
   if( does_account_exist( eName ) == TRUE )
   {
      //Account exists, so we're updating all the fields, not creating a new one
      snprintf( query, MAX_INPUT_LENGTH, "UPDATE `darkston` `player_accounts` SET email='%s' password='%s' creationDate=%f curChars='%s' allChars='%s' flags=%i autoLogin='%s' WHERE accountID=%f",
         eName, acct->password_.c_str(), (double)acct->secCreatedDate, eChars, eAChars, acct->flags, eAutoLogin, acct->id );
   }
   else
   {
      create_account( acct->email, acct->password_.c_str(), acct->characters, acct->allCharacters, acct->flags, acct->autoLoginName_.c_str(), acct->id );
   }
   
   mysql_query( sql_con, query );
   
   if( mysql_error( sql_con ) )
   {
      fprintf( stderr, "Error saving account: %s\n", mysql_error( sql_con ) );
   }
   
   return;
}

*/
///////////////////////////////////////////////////////////////////////////////
//Pulls a help file from the mysql database.                                 //
//help files with keywords starting with '_' will print without the keyword  //
///////////////////////////////////////////////////////////////////////////////
HELP_DATA *get_help( const char *arg )
{
   char eA[MAX_STRING_LENGTH];
   char query[MAX_STRING_LENGTH];
   MYSQL_RES *result;
   MYSQL_ROW row;
   int num = 0;
   static HELP_DATA *h;
   
   if( h != NULL )
   {
      if( h->text )
         free(h->text);
      if( h->keyword )
         free(h->keyword);
      free(h);
   }
   
   MRES( sql_con, eA, arg );
   
   //snprintf( query, MAX_STRING_LENGTH, "SELECT * FROM `help_files` WHERE MATCH(keywords) AGAINST ('%s*' IN BOOLEAN MODE) AND `level` < %i", eA, ch->level );
   snprintf( query, MAX_STRING_LENGTH, "SELECT * FROM `help_files` WHERE `keywords` REGEXP '[[:<:]]%s[[:>:]]'", eA );
   mysql_query( sql_con, query );
   result = mysql_store_result( sql_con );
   if( mysql_errno(sql_con)>0 )
   {
      fprintf( stderr, "::MYSQL ERROR(%i):: %s\n", mysql_errno(sql_con), mysql_error(sql_con) );
      return NULL;
   }
   if( result )
      num = mysql_num_rows( result );
   if( num < 1   )
   {
      return NULL;
   }
   row = mysql_fetch_row( result );
   if( !row )
      return NULL;
   
   h = (HELP_DATA*)malloc(sizeof(HELP_DATA) );
   h->level = atoi(row[0]);
   h->text = strdup(row[1]);
   h->keyword = strdup(row[2]);
   
   return h;
}

void create_account( const char *login, const char *pass, const char *curChars, const char *allChars, int flags, const char *autoLogin, double id )
{
   char query[MAX_INPUT_LENGTH];
   char eLogin[MAX_INPUT_LENGTH];
   char ePass[MAX_INPUT_LENGTH];
   char eCurChars[MAX_INPUT_LENGTH];
   char eAllChars[MAX_INPUT_LENGTH];
   char eAutoLogin[MAX_INPUT_LENGTH];
   double  creationDate;
   char loginDate[MAX_INPUT_LENGTH];
   time_t rawtime;
   struct tm *timeinfo;
   time( &rawtime );
   timeinfo = localtime( &rawtime );
   
   
   MRES( sql_con, eLogin, login );
   MRES( sql_con, ePass, pass );
   MRES( sql_con, eCurChars, curChars );
   MRES( sql_con, eAllChars, allChars );
   MRES( sql_con, eAutoLogin, autoLogin );
   
   creationDate = (double)time(0);
   strftime( loginDate, MAX_INPUT_LENGTH, "%Y-%m-%d %T", timeinfo );
   
   snprintf( query, MAX_INPUT_LENGTH, "INSERT INTO `darkston`.`player_accounts` ( `email`, `password`, `creationDate`, `curChars`, `allChars`, `flags`, `autoLogin`, `id`, `name`, `dob`, `location`, `newsLetter`, `lastLogin`, `lastPlay` ) VALUES ( '%s', SHA1('%s'), '%f', '%s', '%s', '%i', '%s', '%f', 'NAME NOT SET', '1900-01-01', 'LOCATION NOT SET', '1', '%s', '%s' )", 
         eLogin, pass, creationDate, eCurChars, eAllChars, flags, eAutoLogin, id, loginDate, loginDate );
      
   mysql_query( sql_con, query );
   
   if( mysql_error( sql_con ) )
   {
      fprintf( stderr, "Error creating account: %s\n", mysql_error( sql_con ) );
   }
   
   return;
}
   
   
   

bool does_account_exist( const char *name )
{
   char query[MAX_INPUT_LENGTH];
   MYSQL_RES *result;
   char eName[MAX_INPUT_LENGTH];
   int num;
   
   MRES( sql_con, eName, name );
   
   snprintf( query, MAX_INPUT_LENGTH, "SELECT * FROM `player_accounts` WHERE email='%s'", eName );
   if( !sql_con )
      return FALSE;
   
   mysql_query( sql_con, query );
   result = mysql_store_result( sql_con );
   num = mysql_num_rows( result );
   mysql_free_result( result );
   return num == 0 ? FALSE : TRUE;
}

bool check_account_password( const char *name, const char *pass )
{
   char query[MAX_INPUT_LENGTH];
   MYSQL_RES *result;
   char eName[MAX_INPUT_LENGTH];
   char ePass[MAX_INPUT_LENGTH];
   int num;
   
   MRES( sql_con, eName, name );
   MRES( sql_con, ePass, pass );
   
   snprintf( query, MAX_INPUT_LENGTH, "SELECT * FROM `player_accounts` WHERE email='%s' AND password=SHA1('%s')", eName, ePass );
   
   if( !sql_con )
      return FALSE; 
   mysql_query( sql_con, query );
   
   result = mysql_store_result( sql_con );
   num = mysql_num_rows( result );
   
   mysql_free_result( result );
   
   return num == 0 ? FALSE : TRUE;
}

void set_account_password( const char *acct, const char *pass )
{
   char query[MAX_INPUT_LENGTH];
   char eName[MAX_INPUT_LENGTH];
   char ePass[MAX_INPUT_LENGTH];
   
   MRES( sql_con, eName, acct );
   MRES( sql_con, ePass, pass );
   
   snprintf( query, MAX_INPUT_LENGTH, "UPDATE `darkston`.`player_accounts` SET password=SHA1('%s') WHERE email='%s'", ePass, eName );
   
   if( !sql_con )
      return; 
   
   mysql_query( sql_con, query );
   fprintf( stderr, ":: %s ::\n", query );
   if( mysql_error( sql_con ) )
      fprintf( stderr, "::MYSQL ERROR (%i):: %s.\n", mysql_errno( sql_con ), mysql_error( sql_con ) );
   
   return;
}   

D_ACCOUNT *load_account( const char *name, const char *pw )
{
   char query[MAX_INPUT_LENGTH];
   MYSQL_RES *result;
   MYSQL_ROW row;
   D_ACCOUNT *acct;
   char eName[MAX_INPUT_LENGTH];
   char ePass[MAX_INPUT_LENGTH];
   
   if( !name || !pw )
   {
      return NULL;
   }
   
   mysql_real_escape_string( sql_con, eName, name, strlen( name ) );
   mysql_real_escape_string( sql_con, ePass, pw, strlen( pw ) );

   snprintf( query, MAX_INPUT_LENGTH, "SELECT * FROM `player_accounts` WHERE email='%s' AND password=SHA1('%s');", eName, ePass );
   if( !sql_con )
   {
      return NULL;
   }
   mysql_query( sql_con, query );
   result = mysql_store_result( sql_con );
   if( !result )
   {
      fprintf( stderr, "null result set in query %s\n", query );
      return NULL;
   }
   if( mysql_num_rows(result) < 1 )
   {
      mysql_free_result( result );
      return NULL;
   }

   row = mysql_fetch_row( result );

   if( ( acct = new_account() ) == NULL )
   {
      bug( "ERROR: Unable to create new account for socket." );
      return NULL;
   }
   
   acct->email = strdup( row[0] );
   acct->password = strdup( row[1] );
   acct->secCreatedDate = (time_t)atoi( row[2] );
   acct->characters = strdup( row[3] );
   acct->allCharacters = strdup( row[4] );
   acct->flags = (int)atoi( row[5] );
   acct->autoLogin = strdup( row[6] );
   acct->acceptANSI = atoi( row[14] ) == 0 ? FALSE : TRUE;
   
   mysql_free_result( result );
   return acct;
}

void close_mysql()
{
   mysql_close( sql_con );
   return;
}

void init_mysql()
{
   if( ( sql_con = mysql_init( NULL ) ) == NULL )
   {
      fprintf( stderr, "Error: Could not allocate MySQL Object: %s\n", mysql_error( sql_con ) );
      exit( 1 );
   }
   if( ( mysql_real_connect( sql_con, MYSQL_SERVER_ADDRESS,
                                      MYSQL_SERVER_LOGIN,
                                      MYSQL_SERVER_PASS,
                                      MYSQL_DEFAULT_DB,
                                      0, NULL, 0 ) ) == NULL )
   {
      fprintf( stderr, "Error: Could not connect to MySQL Server: %s\n", mysql_error( sql_con ) );
      //fprintf( stderr, "--> Using Server Address: %s\nLogin: %s\nPassword: %s\nDB: %s\n",
      //         MYSQL_SERVER_ADDRESS, MYSQL_SERVER_LOGIN, MYSQL_SERVER_PASS, MYSQL_DEFAULT_DB );
      exit( 1 );
   }
   return;
}
