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

#include <vBBS/Types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vBBS/User.h>
#include <vBBS/SHA1.h>
#include <vBBS/Log.h>

void InitUser(User *user)
{
    user->userID = 0;
    strcpy(user->username, "guest");
    strcpy(user->pwHash, "35675E68F4B5AF7B995D9205AD0FC43842F16450");
    strcpy(user->email, "user@example.com");
    user->userType = REGULAR_USER;
}

bool AuthenticateUser(User *user, const char *username, const char *password)
{
    int i;
    SHA1_CTX sha;
    uint8_t hash[20];
    char hashString[41];

    if (user == NULL || username == NULL || password == NULL)
    {
        return FALSE;
    }

    // Hash the password
    SHA1Init(&sha);
    SHA1Update(&sha, (uint8_t *) password, strlen(password));
    SHA1Final(hash, &sha);

    // Convert hash to hex string
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

bool changePassword(User *user, const char *newPassword)
{
    int i;
    SHA1_CTX sha;
    uint8_t hash[20];
    char hashString[41];

    if (strlen((const char*)newPassword) < 8)
    {
        printf("Password must be at least 8 characters long.\n");
        return FALSE;
    }

    // Hash the password
    SHA1Init(&sha);
    SHA1Update(&sha, (uint8_t *) newPassword, strlen(newPassword));
    SHA1Final(hash, &sha);

    // Convert hash to hex string
    for (i = 0; i < 20; i++)
    { 
        sprintf(&hashString[i * 2], "%02X", hash[i]);
    }
    hashString[40] = '\0';

    strcpy((char*)user->pwHash, hashString);
    printf("Password changed successfully.\n");
    return TRUE;
}
