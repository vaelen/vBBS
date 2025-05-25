/*
Copyright (c) 2025, Andrew Young

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <vbbs/types.h>
#include <vbbs/db.h>
#include <vbbs/log.h>
#include <vbbs/db/user.h>
#include <vbbs/user.h>
#include <ctype.h>
#include <time.h>

#define USER_ID_FORMAT "%u"

UserDB *userDB = NULL;

UserDB *NewUserDB(const char *filename)
{
   UserDB *db = (UserDB *) malloc(sizeof(UserDB));
   if (db == NULL)
   {
      Error("Failed to allocate memory for user database");
      return NULL;
   }
   db->filename = strdup(filename);
   db->nextUserID = 1;
   /* This list owns the user objects in memory. */
   db->users = NewArrayList(10, (ListItemDestructor)DestroyUser);
   return db;
}

void DestroyUserDB(UserDB *db)
{
   if (db == NULL)
   {
      return;
   }
   if (db->filename)
   {
      free(db->filename);
   }
   if (db->users)
   {
      DestroyArrayList(db->users);
   }
   free(db);
}

bool LoadUserDB(void)
{
   if (userDB == NULL)
   {
      userDB = NewUserDB(USER_DB_FILE);
      if (userDB == NULL)
      {
         Error("Failed to create user database");
         return FALSE;
      }
   }
   return _LoadUserDB(userDB);
}

bool _LoadUserDB(UserDB *db)
{
   FILE *file;
   User *user;
   char value[256];
   int fieldNum, i;
   char c;

   if (db == NULL)
   {
      return FALSE;
   }
   
   /* Open the file for reading */
   file= fopen(db->filename, "r");
   if (file == NULL)
   {
      Error("Failed to open user database file: %s", db->filename);
      return FALSE;
   }

   memset(value, 0, sizeof(value));
   fieldNum = 0;
   i = 0;
   user = NULL;

   /* Read users from the file */
   while (!feof(file))
   {
      c = fgetc(file);
      
      /* Start of a new line */
      if (user == NULL)
      {
         user = NewUser();
         if (user == NULL)
         {
            Error("Failed to allocate memory for user");
            fclose(file);
            return FALSE;
         }
         memset(value, 0, sizeof(value));
         i = 0;
      }
      
      if (c == '\r')
      {
         /* Ignore carriage return */
         continue;
      }

      if (c == '\t' || c == '\n')
      {
         /* End of a field, process the value */
         switch (fieldNum)
         {
            case 0:
               user->userID = atoi(value);
               break;
            case 1:
               strncpy(user->username, value, sizeof(user->username) - 1);
               user->username[sizeof(user->username) - 1] = '\0';
               break;
            case 2:
               strncpy(user->email, value, sizeof(user->email) - 1);
               user->email[sizeof(user->email) - 1] = '\0';
               break;
            case 3:
               user->userType = atoi(value);
               break;
            case 4:
               strncpy(user->pwHash, value, sizeof(user->pwHash) - 1);
               user->pwHash[sizeof(user->pwHash) - 1] = '\0';
               break;
            case 5:
               user->lastSeen = strtoul(value, NULL, 10);
               break;
         }
         fieldNum++;
         memset(value, 0, sizeof(value));
         i = 0;
      }
      else
      {
         value[i++] = c;
      }

      if (c == '\n')
      {
         /* End of a user record. */
         if (user != NULL && user->userID != 0)
         {
            _AddUser(db, user);
            user = NULL;
         }
         fieldNum = 0;
         continue;
      }
   }

   if (user != NULL && user->userID != 0)
   {
      /* Add the last user if it exists */
      _AddUser(db, user);
      user = NULL;
   }

   if (user != NULL)
   {
      /* If we end up with an unused empty record, clean it up. */
      DestroyUser(user);
   }

   fclose(file);
   return TRUE;
}

bool SaveUserDB(void)
{
   if (userDB == NULL)
   {
      return FALSE;
   }
   return _SaveUserDB(userDB);
}

bool _SaveUserDB(UserDB *db)
{
   FILE *file;
   int i;
   User *user;

   if (db == NULL || db->filename == NULL || db->users == NULL)
   {
      return FALSE;
   }

   /* Open the file for writing */
   /* If the file doesn't exist, it will be created  */
   file = fopen(db->filename, "w");
   if (file == NULL)
   {
      Error("Failed to open user database file for writing: %s", db->filename);
      return FALSE;
   }

   /* Write users to the file */
   for (i = 0; i < db->users->size; i++)
   {
      user = (User *) GetFromArrayList(db->users, i);
      if (user == NULL)
      {
         continue;
      }
      Debug("Saving User: %lu\t%s\t%s\t%s\t%d\t%lu\n", user->userID, user->username,
               user->pwHash, user->email, user->userType, user->lastSeen);
      
      fprintf(file, "%u\t", user->userID);
      fprintf(file, "%s\t", user->username);
      fprintf(file, "%s\t", user->email);
      fprintf(file, "%d\t", user->userType);
      fprintf(file, "%s\t", user->pwHash);
      fprintf(file, "%lu\n", user->lastSeen);
   }

   fflush(file);
   fclose(file);
   return TRUE;
}

void AddUser(User *user)
{
   if (userDB == NULL)
   {
      return;
   }
   _AddUser(userDB, user);
}

void _AddUser(UserDB *db, User *user)
{
   if (db == NULL || user == NULL)
   {
      return;
   }
   if (user->userID == 0)
   {
      user->userID = db->nextUserID++;
   }
   if (user->userID >= db->nextUserID)
   {
      db->nextUserID = user->userID + 1;
   }
   if (user->lastSeen == 0)
   {
      user->lastSeen = time(NULL);
   }
   AddToArrayList(db->users, user);
   Debug("Added user: %s, pw: %s, ID: %u, Seen: %lu", user->username, 
      user->pwHash, user->userID, user->lastSeen);
}

void RemoveUser(unsigned int userID)
{
   if (userDB == NULL)
   {
      return;
   }
   _RemoveUser(userDB, userID);
}

void _RemoveUser(UserDB *db, unsigned int userID)
{
   User *user;
   int i;
   if (db == NULL)
   {
      return;
   }
   for (i = 0; i < db->users->size; i++)
   {
      user = (User *) GetFromArrayList(db->users, i);
      if (user == NULL)
      {
         continue;
      }
      if (user->userID == userID)
      {
         Debug("Removing user: %s, ID: %u", user->username, user->userID);
         RemoveFromArrayList(db->users, i);
         break;
      }
   }
}

User *GetUserByID(unsigned int userID)
{
   if (userDB == NULL)
   {
      return NULL;
   }
   return _GetUserByID(userDB, userID);
}

User *_GetUserByID(UserDB *db, unsigned int userID)
{
   int i;
   User *user;
   if (db == NULL || db->users == NULL)
   {
      return NULL;
   }
      for (i = 0; i < db->users->size; i++)
   {
      user = (User *) GetFromArrayList(db->users, i);
      if (user == NULL)
      {
         continue;
      }
      if (user->userID == userID)
      {
         return user;
      }
   }
   return NULL;
}

User *GetUserByUsername(const char *username)
{
   if (userDB == NULL)
   {
      return NULL;
   }
   return _GetUserByUsername(userDB, username);
}

User *_GetUserByUsername(UserDB *db, const char *username)
{
   int i;
   User *user;
   if (db == NULL || db->users == NULL || username == NULL)
   {
      return NULL;
   }
   for (i = 0; i < db->users->size; i++)
   {
      user = (User *) GetFromArrayList(db->users, i);
      if (user == NULL)
      {
         continue;
      }
      if (strcasecmp(username, user->username) == 0)
      {
         return user;
      }
   }
   return NULL;
}

int GetUserCount(void)
{
   if (userDB == NULL)
   {
      return 0;
   }
   return _GetUserCount(userDB);
}

int _GetUserCount(UserDB *db)
{
   if (db == NULL || db->users == NULL)
   {
      return 0;
   }
   return db->users->size;
}
