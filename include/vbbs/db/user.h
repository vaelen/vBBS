#ifndef VBBS_DB_USER_H
#define VBBS_DB_USER_H
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
#include <vbbs/user.h>
#include <vbbs/list.h>
#include <vbbs/map.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USER_DB_FILE "users.db"

typedef struct UserDB
{
   char *filename;
   unsigned int nextUserID;
   ArrayList *users;
} UserDB;

extern UserDB *userDB;

bool LoadUserDB(void);
bool SaveUserDB(void);
void AddUser(User *user);
void RemoveUser(unsigned int userID);
User *GetUserByID(unsigned int userID);
User *GetUserByUsername(const char *username);
int GetUserCount(void);

UserDB *NewUserDB(const char *filename);
void DestroyUserDB(UserDB *db);

bool _LoadUserDB(UserDB *db);
bool _SaveUserDB(UserDB *db);
void _AddUser(UserDB *db, User *user);
void _RemoveUser(UserDB *db, unsigned int userID);
User *_GetUserByID(UserDB *db, unsigned int userID);
User *_GetUserByUsername(UserDB *db, const char *username);
int _GetUserCount(UserDB *db);

#endif
