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
#include <vBBS/Log.h>
#include <vBBS/Terminal.h>

void InitTerminal(Terminal *terminal)
{
    terminal->type = "Unknown";
    terminal->isANSI = FALSE;
    terminal->width = 80;
    terminal->height = 24;
}

void NegotiateTerminal(FILE *in, FILE *out, Terminal *terminal)
{
    char da[51], attr[51];
    int i = 0, j = 0, n = 0;
    int attrs[25]; /* each attribute is at least 2 bytes */

    fprintf(out, "[Press Enter to Continue]\n");
    fflush(out);

    fprintf(out, "%s", SET_CONSEAL);
    fprintf(out, "%s", IDENTIFY);
    fflush(out);

    i = fscanf(in, IDENTIFY_RESPONSE, da);
    fprintf(out, "%s", SET_CONSEAL_OFF);
    fflush(out);

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
                terminal->type = "VT100";
                break;
            case 4:
                Debug("Terminal type: VT132");
                terminal->type = "VT132";
                break;
            case 6:
                Debug("Terminal type: VT102");
                terminal->type = "VT102";
                break;
            case 7:
                Debug("Terminal type: VT131");
                terminal->type = "VT131";
                break;
            case 12:
                Debug("Terminal type: VT125");
                terminal->type = "VT125";
                break;
            case 61:
                Debug("Terminal type: Gnome Terminal?");
                terminal->type = "Gnome Terminal?";
                break;
            case 62:
                Debug("Terminal type: VT220");
                terminal->type = "VT220";
                break;
            case 63:
                Debug("Terminal type: VT320");
                terminal->type = "VT320";
                break;
            case 64:
                Debug("Terminal type: VT420");
                terminal->type = "VT420";
                break;
            case 65:
                Debug("Terminal type: VT520");
                terminal->type = "VT520";
                break;               
            default:
                Debug("Unknown terminal type: %d (%s)", attrs[0], da);
                break;
        }
    } /* Got ident response */

    if (!terminal->isANSI)
    {
        fprintf(out, "Does your terminal support ANSI escape codes? (y/n): ");
        fflush(out);
        do
        {
            i = fgetc(in);
            switch (i)
            {
                case 'y':
                case 'Y':
                    terminal->isANSI = TRUE;
                    i = EOF;
                    break;
                case 'n':
                case 'N':
                    terminal->isANSI = FALSE;
                    i = EOF;
                    break;
            }
        } while (i != EOF);
    } /* is not ANSI */

    if(terminal->isANSI)
    {
        fprintf(out, "ANSI escape codes enabled.\n");
    }
    else
    {
        fprintf(out, "ANSI escape codes disabled.\n");
    }
    fflush(out);
}

int StripANSI(const char *str, FILE *output)
{
    int bytesWritten = 0;
    bool inEscape = FALSE;
    bool inCSI = FALSE;
    while (*str)
    {
        if (inEscape)
        {
            if (inCSI)
            {
                if ((*str >= 'A' && *str <= 'Z') || 
                   (*str >= 'a' && *str <= 'z'))
                {
                    /* end of CSI */
                    inCSI = FALSE;
                    inEscape = FALSE;
                }
                /* skip all characters in the CSI */
            }
            else if (*str == ANSI_CSI_CHAR)
            {
                /** start of CSI */
                inCSI = TRUE;
            }
            else {
                /* single character escape */
                inEscape = FALSE;
            }
        }
        else if (*str == ANSI_ESCAPE_CHAR)
        {
            /** start of escape */
            inEscape = TRUE;
        }
        else
        {
            if (fputc(*str, output) == EOF)
            {
                return EOF;
            }
            bytesWritten++;
        }
        str++;
    }
    return bytesWritten;
}
