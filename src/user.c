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

#include <stdlib.h>
#include <string.h>
#include <vbbs/user.h>
#include <vbbs/sha1.h>
#include <vbbs/log.h>

User *NewUser(void)
{
    User *user = (User *) malloc(sizeof(User));
    if (user == NULL)
    {
        return NULL;
    }
    user->userID = 0;
    memset(user->username, 0, sizeof(user->username));
    memset(user->pwHash, 0, sizeof(user->pwHash));
    memset(user->email, 0, sizeof(user->email));
    user->lastSeen = 0;
    user->userType = REGULAR_USER;
    return user;
}

void DestroyUser(User *user)
{
    if (user == NULL)
    {
        return;
    }
    free(user);
}

User *CopyUser(const User *src)
{
    User *user = (User *) malloc(sizeof(User));
    if (user == NULL)
    {
        return NULL;
    }
    user->userID = src->userID;
    strcpy(user->username, src->username);
    strcpy(user->pwHash, src->pwHash);
    strcpy(user->email, src->email);
    user->userType = src->userType;
    return user;
}

bool AuthenticateUser(User *user, const char *username, const char *password)
{
    int i;
    SHA1_CTX sha;
    uint8_t hash[20];
    char hashString[41];

    Debug("Authenticating user: '%s'", username);

    if (user == NULL || username == NULL || password == NULL)
    {
        return FALSE;
    }

    /* Hash the password */
    SHA1Init(&sha);
    SHA1Update(&sha, (uint8_t *) password, strlen(password));
    SHA1Final(hash, &sha);

    /* Convert hash to hex string */
    for (i = 0; i < 20; i++)
    {
        sprintf(&hashString[i * 2], "%02X", hash[i]);
    }
    hashString[40] = '\0';

    Debug("Hash: %s", hashString);

    if (strcmp(user->username, username) == 0 && 
        strcmp(user->pwHash, hashString) == 0)
    {
        return TRUE;
    }
    return FALSE;
}

bool ChangePassword(User *user, const char *newPassword)
{
    int i;
    SHA1_CTX sha;
    uint8_t hash[20];
    char hashString[41];

    /* Hash the password */
    SHA1Init(&sha);
    SHA1Update(&sha, (uint8_t *) newPassword, strlen(newPassword));
    SHA1Final(hash, &sha);

    /* Convert hash to hex string */
    for (i = 0; i < 20; i++)
    { 
        sprintf(&hashString[i * 2], "%02X", hash[i]);
    }
    hashString[40] = '\0';

    strncpy((char*)user->pwHash, hashString, sizeof(user->pwHash) - 1);
    user->pwHash[sizeof(user->pwHash) - 1] = '\0';
    return TRUE;
}
