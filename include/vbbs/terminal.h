#ifndef _VBBS_TERMINAL_H
#define _VBBS_TERMINAL_H

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
#include <vbbs/buffer.h>
#include <stdio.h>

/**
 * ANSI escape codes for terminal control.
 * 
 * Wikipedia page on ANSI escape codes:
 * https://en.wikipedia.org/wiki/ANSI_escape_code
 * 
 * VT100 escape code list:
 * https://gist.github.com/delameter/b9772a0bf19032f977b985091f0eb5c1
 * 
 * ANSI escape code list:
 * https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
 */

#define ANSI_ESCAPE_CHAR '\033'
#define ANSI_CSI_CHAR '['

#define IDENTIFY "\033[c"
#define IDENTIFY_RESPONSE "\033[?%50sc"

#define DEVICE_STATUS_REPORT "\033[5n"

#define CLEAR_TO_END_OF_LINE "\033[0K"
#define CLEAR_TO_START_OF_LINE "\033[1K"
#define CLEAR_LINE "\033[2K"

#define CLEAR_TO_END_OF_SCREEN "\033[0J"
#define CLEAR_TO_START_OF_SCREEN "\033[1J"
#define CLEAR_SCREEN "\033[2J"

#define SET_CURSOR_POS(x, y) "\033[" #y ";" #x "H"
#define SET_CURSOR_COL(x) "\033[" #x "G"
#define CURSOR_HOME "\033[H"
#define CURSOR_UP_ONE "\033[A"
#define CURSOR_UP(x) "\033[" #x "A"
#define CURSOR_DOWN_ONE "\033[B"
#define CURSOR_DOWN(x) "\033[" #x "B"
#define CURSOR_RIGHT_ONE "\033[C"
#define CURSOR_RIGHT(x) "\033[" #x "C"
#define CURSOR_LEFT_ONE "\033[D"
#define CURSOR_LEFT(x) "\033[" #x "D"
#define CURSOR_SAVE "\0337"
#define CURSOR_RESTORE "\0338"
#define CURSOR_HIDE "\033[?25l"
#define CURSOR_SHOW "\033[?25h"

#define SET_COLOR(fg, bg) "\033[" #fg ";" #bg "m"
#define SET_BOLD "\033[1m"
#define SET_FAINT "\033[2m"
#define SET_BOLD_OFF "\033[22m"
#define SET_ITALIC "\033[3m"
#define SET_ITALIC_OFF "\033[23m"
#define SET_UNDERLINE "\033[4m"
#define SET_UNDERLINE_OFF "\033[24m"
#define SET_BLINK "\033[5m"
#define SET_BLINK_OFF "\033[25m"
#define SET_REVERSE "\033[7m"
#define SET_REVERSE_OFF "\033[27m"
#define SET_CONCEAL "\033[8m"
#define SET_CONCEAL_OFF "\033[28m"
#define SET_STRIKETHROUGH "\033[9m"
#define SET_STRIKETHROUGH_OFF "\033[29m"

#define CRLF_MODE "\033[20h"
#define LF_MODE "\033[20l"

#define CURSOR_ON "\033[?25h"
#define CURSOR_OFF "\033[?25l"

#define ECHO_ON "\033[12l"
#define ECHO_OFF "\033[12h"

#define RESET_MODES "\033[0m"

#define SET_FG_DEFAULT "\033[39m"
#define SET_BG_DEFAULT "\033[49m"

#define SET_FG_BLACK "\033[30m"
#define SET_FG_RED "\033[31m"
#define SET_FG_GREEN "\033[32m"
#define SET_FG_YELLOW "\033[33m"
#define SET_FG_BLUE "\033[34m"
#define SET_FG_MAGENTA "\033[35m"
#define SET_FG_CYAN "\033[36m"
#define SET_FG_WHITE "\033[37m"
#define SET_FG_BRIGHT_BLACK "\033[90m"
#define SET_FG_BRIGHT_RED "\033[91m"
#define SET_FG_BRIGHT_GREEN "\033[92m"
#define SET_FG_BRIGHT_YELLOW "\033[93m"
#define SET_FG_BRIGHT_BLUE "\033[94m"
#define SET_FG_BRIGHT_MAGENTA "\033[95m"
#define SET_FG_BRIGHT_CYAN "\033[96m"
#define SET_FG_BRIGHT_WHITE "\033[97m"

#define SET_BG_BLACK "\033[40m"
#define SET_BG_RED "\033[41m"
#define SET_BG_GREEN "\033[42m"
#define SET_BG_YELLOW "\033[43m"
#define SET_BG_BLUE "\033[44m"
#define SET_BG_MAGENTA "\033[45m"
#define SET_BG_CYAN "\033[46m"
#define SET_BG_WHITE "\033[47m"
#define SET_BG_BRIGHT_BLACK "\033[100m"
#define SET_BG_BRIGHT_RED "\033[101m"
#define SET_BG_BRIGHT_GREEN "\033[102m"
#define SET_BG_BRIGHT_YELLOW "\033[103m"
#define SET_BG_BRIGHT_BLUE "\033[104m"
#define SET_BG_BRIGHT_MAGENTA "\033[105m"
#define SET_BG_BRIGHT_CYAN "\033[106m"
#define SET_BG_BRIGHT_WHITE "\033[107m"

#define SET_FG_RGB(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"
#define SET_BG_RGB(r, g, b) "\033[48;2;" #r ";" #g ";" #b "m"
#define SET_FG_256(n) "\033[38;5;" #n "m"
#define SET_BG_256(n) "\033[48;5;" #n "m"

/**
 * Telnet protocol commands and options.
 * See https://users.cs.cf.ac.uk/Dave.Marshall/Internet/node141.html
 * See https://datatracker.ietf.org/doc/html/rfc854 for the Telnet protocol.
 * See https://www.iana.org/assignments/telnet-options/telnet-options.xhtml
 * 
 * Request  Response    Meaning
 * WILL     DO          Sender requests option, receiver can support it
 * WILL     DONT        Sender requests option, receiver cannot support it
 * WONT     DONT        Sender does not request option, receiver acknowledges
 * DO       WILL        Sender can support option, receiver requests it
 * DO       WONT        Sender can support option, receiver doesn't request it
 * DONT     WONT        Sender cannot support option, receiver acknowledges
 * 
 * Example: The sender wants to enable ECHO:
 *   Sender: IAC DO ECHO
 *   Receiver: IAC WILL ECHO
 * 
 * Example: The sender requests the terminal type:
 *   Sender: IAC SB TERMINAL-TYPE SEND IAC SE
 *   Receiver: IAC SB TERMINAL-TYPE IS <type> IAC SE
 * 
 * Telnet Echo RFC: https://datatracker.ietf.org/doc/html/rfc857
 * Telnet Line Mode RFC: https://datatracker.ietf.org/doc/html/rfc1184
 * Telnet Terminal Type RFC: https://datatracker.ietf.org/doc/html/rfc1091
 * Telnet Window Size RFC: https://datatracker.ietf.org/doc/html/rfc1073
 */

#define TELNET_IAC  255 /* 0xFF Interpret As Command */
#define TELNET_DONT 254 /* 0xFE Receiver can't support option*/
#define TELNET_DO   253 /* 0xFD Sender asks for option support */
#define TELNET_WONT 252 /* 0xFC Receiver cannot handle option */
#define TELNET_WILL 251 /* 0xFB Sender says it can handle option*/
#define TELNET_SB   250 /* 0xFA Subnegotiation */
#define TELNET_GA   249 /* 0xF9 Go Ahead */
#define TELNET_EL   248 /* 0xF8 Erase Line */
#define TELNET_EC   247 /* 0xF7 Erase Character */
#define TELNET_AYT  246 /* 0xF6 Are You There? */
#define TELNET_AO   245 /* 0xF5 Abort Output */
#define TELNET_IP   244 /* 0xF4 Interrupt Process */
#define TELNET_BRK  243 /* 0xF3 Break */
#define TELNET_DM   242 /* 0xF2 Data Mark */
#define TELNET_NOP  241 /* 0xF1 No Operation */
#define TELNET_SE   240 /* 0xF0 Subnegotiation End */

#define TELNET_SE_IS   0
#define TELNET_SE_SEND 1

#define TELNET_OPTION_BINARY                0 /* 0x00 */
#define TELNET_OPTION_ECHO                  1 /* 0x01 */
#define TELNET_OPTION_SUPPRESS_GO_AHEAD     3 /* 0x03 */
#define TELNET_OPTION_STATUS                5 /* 0x05 */
#define TELNET_OPTION_TIMING_MARK           6 /* 0x06 */
#define TELNET_OPTION_TERMINAL_TYPE         24 /* 0x18 */
#define TELNET_OPTION_WINDOW_SIZE           31 /* 0x1F */
#define TELNET_OPTION_TERMINAL_SPEED        32 /* 0x20 */
#define TELNET_OPTION_FLOW_CONTROL          33 /* 0x21 */
#define TELNET_OPTION_LINE_MODE             34 /* 0x22 */
#define TELNET_OPTION_X_DISPLAY_LOCATION    35 /* 0x23 */
#define TELNET_OPTION_ENV                   36 /* 0x24 */
#define TELNET_OPTION_ENCRYPTION            38 /* 0x26 */
#define TELNET_OPTION_NEW_ENVIRON           39 /* 0x27 */


#define TELNET_DO_ECHO      "\xFF\xFD\x01" /* Please start echoing*/
#define TELNET_DONT_ECHO    "\xFF\xFE\x01" /* Do not echo */
#define TELNET_WILL_ECHO    "\xFF\xFB\x01" /* I will echo */
#define TELNET_WONT_ECHO    "\xFF\xFC\x01" /* I will not echo */

#define TELNET_DO_LINE_MODE_NEG     "\xFF\xFD\x22"
#define TELNET_DONT_LINE_MODE_NEG   "\xFF\xFE\x22"
#define TELNET_WILL_LINE_MODE_NEG   "\xFF\xFB\x22"
#define TELNET_WONT_LINE_MODE_NEG   "\xFF\xFC\x22"

#define TELNET_DO_SUPPRESS_GO_AHEAD     "\xFF\xFD\x03"
#define TELNET_DONT_SUPPRESS_GO_AHEAD   "\xFF\xFE\x03"
#define TELNET_WILL_SUPPRESS_GO_AHEAD   "\xFF\xFB\x03"
#define TELNET_WONT_SUPPRESS_GO_AHEAD   "\xFF\xFC\x03"

/* Disable line mode editing*/
#define TELNET_LINE_MODE_DEFAULTS1  "\xFF\xFA\x22\x01\x00\xFF\xF0"
#define TELNET_LINE_MODE_DEFAULTS2  "\xFF\xFA\x22\xFE\x02\xFF\xF0"

/**
 * Data structures and functions
 */

typedef enum
{
    RAW,
    ANSI
} TerminalType;

typedef struct
{
    char *type;
    bool isANSI;
    unsigned int width;
    unsigned int height;
} Terminal;

void InitTerminal(Terminal *terminal);
void Identify(Buffer *out);
void CheckIdentifyResponse(Buffer *out, const char *response,
    Terminal *terminal);

#endif
