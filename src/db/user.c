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

static void toLowerString(char *str)
{
   int i;
   int l = strlen(str);
   if (str == NULL)
   {
      return;
   }
   for (i = 0; i < l; i++)
   {
      str[i] = tolower(str[i]);
   }
}

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
   /* No destructor is provided so that removing users 
      won't free the underlying user object's memory. */
   db->userIDs = NewMap(NULL);
   if (db->userIDs == NULL)
   {
      Error("Failed to create user map");
      free(db->filename);
      free(db);
      return NULL;
   }
   /* No destructor is provided so that removing users 
      won't free the underlying user object's memory. */
   db->usernames = NewMap(NULL);
   if (db->usernames == NULL)
   {
      Error("Failed to create usernames map");
      DestroyMap(db->userIDs);
      free(db->filename);
      free(db);
      return NULL;
   }
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
   if (db->usernames)
   {
      DestroyMap(db->usernames);
   }
   if (db->userIDs)
   {
      DestroyMap(db->userIDs);
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

   /* Read the next user ID */
   fscanf(file, "%u\n", &db->nextUserID);

   /* Read users from the file */
   while (!feof(file))
   {
      user = NewUser();
      if (fscanf(file, "%u %s %s %s %d\n", &user->userID, user->username,
                  user->pwHash, user->email, &user->userType) != 5)
      {
         DestroyUser(user);
         break;
      }
      _AddUser(db, user);
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
   ArrayList *bucket;
   int i, j;
   User *user;

   if (db == NULL)
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

   /* Write the next user ID */
   fprintf(file, "%u\n", db->nextUserID);

   /* Write users to the file */
   for (i = 0; i < db->userIDs->bucketCount; i++)
   {
      bucket = db->userIDs->buckets[i];
      for (j = 0; j < bucket->size; j++)
      {
         user = (User *) GetFromArrayList(bucket, j);
         if (user == NULL)
         {
            continue;
         }
         fprintf(stderr, "%u %s %s %s %d %lu\n", user->userID, user->username,
                  user->pwHash, user->email, user->userType, 
                  (unsigned long)user->lastSeen);
         fprintf(file, "%u %s %s %s %d %lu\n", user->userID, user->username,
                  user->pwHash, user->email, user->userType, 
                  (unsigned long)user->lastSeen);
      }
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
   char userID[256];
   char username[256];
   if (db == NULL || user == NULL)
   {
      return;
   }
   if (user->userID == 0)
   {
      user->userID = db->nextUserID++;
   }
   user->lastSeen = time(NULL);
   AddToArrayList(db->users, user);
   sprintf(userID, USER_ID_FORMAT, user->userID);
   MapPut(db->userIDs, userID, user);
   strncpy(username, user->username, 
      MIN(sizeof(username), sizeof(user->username)));
   toLowerString(username);
   MapPut(db->usernames, username, user);
   Debug("Added user: %s, pw: %s, ID: %u, Seen: %lu (id: %s, username: %s)", user->username, user->pwHash, 
      user->userID, user->lastSeen, userID, username);
}

void RemoveUser(uint32_t userID)
{
   if (userDB == NULL)
   {
      return;
   }
   _RemoveUser(userDB, userID);
}

void _RemoveUser(UserDB *db, uint32_t userID)
{
   User *user;
   char key[256];
   char username[256];
   int i;
   if (db == NULL)
   {
      return;
   }
   sprintf(key, USER_ID_FORMAT, userID);
   user = MapGet(db->userIDs, key);
   if (user != NULL)
   {
      for (i = 0; i < db->users->size; i++)
      {
         if (user == GetFromArrayList(db->users, i))
         {
            RemoveFromArrayList(db->users, i);
            break;
         }
      }
      strncpy(username, user->username, 
         MIN(sizeof(username), sizeof(user->username)));
      toLowerString(username);
      MapRemove(db->usernames, username);
      MapRemove(db->userIDs, key);
   }
}

User *GetUserByID(uint32_t userID)
{
   if (userDB == NULL)
   {
      return NULL;
   }
   return _GetUserByID(userDB, userID);
}

User *_GetUserByID(UserDB *db, uint32_t userID)
{
   char key[256];
   if (db == NULL || db->userIDs == NULL)
   {
      return NULL;
   }
   sprintf(key, USER_ID_FORMAT, userID);
   return MapGet(db->userIDs, key);
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
   char key[256];
   if (db == NULL || db->usernames == NULL || username == NULL)
   {
      return NULL;
   }
   strncpy(key, username, sizeof(key));
   toLowerString(key);
   return MapGet(db->usernames, key);
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
   if (db == NULL || db->userIDs == NULL)
   {
      return 0;
   }
   return db->userIDs->size;
}
