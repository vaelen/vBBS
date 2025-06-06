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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vbbs/log.h>
#include <vbbs/terminal.h>
#include <vbbs/rb.h>

void InitTerminal(Terminal *terminal)
{
    SetTerminalType(terminal, "Unknown");
    terminal->isANSI = TRUE;
    terminal->width = 80;
    terminal->height = 24;
}

void Identify(Buffer *out)
{
    WriteStringToBuffer(out, "[Press Enter to Continue]\n");
    WriteStringToBuffer(out, SET_CONCEAL);
    WriteStringToBuffer(out, TELNET_DO_SUPPRESS_GO_AHEAD);
    WriteStringToBuffer(out, TELNET_WILL_SUPPRESS_GO_AHEAD);
    //WriteStringToBuffer(out, TELNET_DO_BINARY);
    //WriteStringToBuffer(out, TELNET_WILL_BINARY);
    WriteStringToBuffer(out, TELNET_DONT_ECHO);
    WriteStringToBuffer(out, TELNET_WILL_ECHO);
    WriteStringToBuffer(out, TELNET_DO_TERMINAL_TYPE);
    WriteStringToBuffer(out, TELNET_REQ_TERMINAL_TYPE);
    WriteStringToBuffer(out, TELNET_DO_WINDOW_SIZE);
    WriteStringToBuffer(out, TELNET_REQ_WINDOW_SIZE);
    WriteStringToBuffer(out, TELNET_DO_TERMINAL_SPEED);
    WriteStringToBuffer(out, TELNET_REQ_TERMINAL_SPEED);
    WriteStringToBuffer(out, IDENTIFY);
}

void CheckIdentifyResponse(Buffer *out, const char *response,
    Terminal *terminal)
{
    char da[51], attr[51];
    int i = 0, j = 0, n = 0;
    int attrs[25]; /* each attribute is at least 2 bytes */

    if (out == NULL || response == NULL || terminal == NULL)
    {
        return;
    }

    WriteStringToBuffer(out, SET_CONCEAL_OFF);

    i = sscanf(response, IDENTIFY_RESPONSE, da);
    WriteStringToBuffer(out, SET_CONCEAL_OFF);

    if (i == 1)
    {
        terminal->isANSI = TRUE;

        /* Parse the response */
        n = 0;
        j = 0;
        for (i = 0; i < (int)strlen(da); i++)
        {
            if (da[i] == ';')
            {
                attr[j] = '\0';
                attrs[n] = atoi(attr);
                n++;
                j = 0;
            } 
            else 
            {
                attr[j] = da[i];
                j++;
            }
        }

        switch(attrs[0])
        {
            case 1:
                Debug("Terminal type: VT100");
                SetTerminalType(terminal, "VT100");
                break;
            case 4:
                Debug("Terminal type: VT132");
                SetTerminalType(terminal, "VT132");
                break;
            case 6:
                Debug("Terminal type: VT102");
                SetTerminalType(terminal, "VT102");
                break;
            case 7:
                Debug("Terminal type: VT131");
                SetTerminalType(terminal, "VT131");
                break;
            case 12:
                Debug("Terminal type: VT125");
                SetTerminalType(terminal, "VT125");
                break;
            case 61:
                Debug("Terminal type: Gnome Terminal?");
                SetTerminalType(terminal, "Gnome Terminal?");
                break;
            case 62:
                Debug("Terminal type: VT220");
                SetTerminalType(terminal, "VT220");
                break;
            case 63:
                Debug("Terminal type: VT320");
                SetTerminalType(terminal, "VT320");
                break;
            case 64:
                Debug("Terminal type: VT420");
                SetTerminalType(terminal, "VT420");
                break;
            case 65:
                Debug("Terminal type: VT520");
                SetTerminalType(terminal, "VT520");
                break;               
            default:
                Debug("Unknown terminal type: %d (%s)", attrs[0], da);
                break;
        }
    } /* Got ident response */

    if(terminal->isANSI)
    {
        WriteStringToBuffer(out, "ANSI escape codes enabled.\n");
    }
    else
    {
        WriteStringToBuffer(out, "ANSI escape codes disabled.\n");
    }
}

void SetTerminalType(Terminal *terminal, const char *type)
{
    if (terminal == NULL || type == NULL)
    {
        return;
    }
    strncpy(terminal->type, type, sizeof(terminal->type) - 1);
    terminal->type[sizeof(terminal->type) - 1] = '\0';
}
