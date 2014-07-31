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

D_ACCOUNT *new_account()
{
   D_ACCOUNT *acct;
   
   if( ( acct = malloc( sizeof( D_ACCOUNT ) ) ) == NULL )
   {
      bug( "ERROR: Unable to allocate memory for new account!" );
      return NULL;
   }
   
   acct->email         = NULL;
   acct->password      = NULL;
   acct->characters    = NULL;
   acct->allCharacters = NULL;
   acct->autoLogin     = NULL;
   acct->acceptANSI    = FALSE;
   
   return acct;
}

void free_account( D_ACCOUNT *acct )
{
   if( !acct )
   {
      bug( "ERROR: Attempting to free null account." );
      return;
   }
   if( acct->email )
      free( acct->email );
   if( acct->password )
      free( acct->password );
   if( acct->characters )
      free( acct->characters );
   if( acct->allCharacters )
      free( acct->allCharacters );
   if( acct->autoLogin )
      free( acct->autoLogin );
   
   free( acct );
   
   return;
}
