#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <windows.h>

#include "debug.h"
#include "wfio.h"
#include "input.h"
                
#define STDIN 1

#define MAXLOOK 17   // Max lookahead
#define MAXLEX  1024 // Max lexeme size

#define BUFSIZE ( (MAXLEX * 4) + (2 * MAXLOOK))

#define DANGER ( End_buf - MAXLOOK )  // Flush buffer when next passes this address

#define END (&Start_buf[BUFSIZE])	  // Just passed last char in buff

#define NO_MORE_CHARS() (Eof_read && Next >= End_buf) 

//#define INVALID_HANDLE_VALUE ((HANDLE)(ULONG_PTR)-1)

typedef unsigned char uchar;

uchar Start_buf[BUFSIZE]; // Input Buffer
uchar *End_buf = END;     // Just past last character
uchar *Next = END;		  // Next Input Character
uchar *sMark = END;       // Start of current lexeme
uchar *eMark = END;       // End of current lexeme
uchar *pMark = END;		  // Start pf previous lexeme 
int pLineno = 1;          // line # of previous Lexeme
int pLength = 1;          // Length of previous Lexeme

//int Inp_file = STDIN;     // Input file "handle"
HANDLE Inp_file = NULL;
int Lineno = 2;			  // Current line number
int Mline = 2;            // Line # when mark_end() called
int Termchar = 0;         /* Holds the character that was
                            overritten by \0 when we null 
                            terminated the last lexeme.  */

int Eof_read = 0;         /* End of file has been read 
                             This can be true with characters
                             still in the input buffer */

HANDLE (*Openp)() = d_Open; //pointer to open funciton
int (*Closep)() = d_Close; //pointer to close funciton
int (*Readp)() = d_Read; //pointer to read funciton

void ii_io( HANDLE (*open_funct)(), int (*close_funct)(), int (*read_funct)() )
{
    /* Lets you modify the open, close, and read
    *  file functions used by the the input system */

    Openp = open_funct;
    Closep = close_funct;
    Readp = read_funct;
}

HANDLE ii_newfile(const char* name)
{
    HANDLE fh; // file descriptor

    // if ((fd = !name ? STDIN : (*Openp)(name, O_RDONLY | O_BINARY)) != -1)
     if ((fh = !name ? INVALID_HANDLE_VALUE : (*Openp)(name)) != INVALID_HANDLE_VALUE)
    {
        if (Inp_file != INVALID_HANDLE_VALUE)
            (*Closep)(Inp_file);
        else {
            return INVALID_HANDLE_VALUE;
        }

        Inp_file = fh;
        Eof_read = 0;

        Next = END;
        sMark = END;
        eMark = END;
        End_buf = END;
        Lineno = 1;
        Mline = 1;
    }
    return fh;
}

char* ii_text()  { return(sMark); }
int ii_length()  { return(eMark - sMark); }
int ii_lineno()  { return(Lineno); }
char* ii_ptext() { return(pMark); }
int ii_plength() { return(pLength); }
int ii_plineno() { return(pLineno); }

char* ii_mark_start()
{
    Mline = Lineno;
    eMark = sMark = Next;
    return(sMark);
}

char* ii_mark_end()
{
    Mline = Lineno;
    return(eMark = Next);
}

char* ii_move_start()
{
    if (sMark >= eMark)
        return NULL;
    else 
        return ++sMark;
}

char* ii_to_mark()
{
    Lineno = Mline;
    return (Next = eMark);
}

char* ii_mark_prev()
{
    // Set the pMark
    // Buffer flush wont go past pMark,
    // so dont forget to set it

    pMark = sMark;
    pLineno = Lineno;
    pLength = eMark - sMark;
    return(pMark);
}

uchar ii_advance()
{
    /* Actuall input function. Returns the next char and advances past it */
    static int been_called = 0;

    if (!been_called) 
    {
        /* Push a newline into the empty buffer so that the lex start of line
           anchor will work on the first input line 
        */
        Next = eMark = sMark = END - 1;
        *Next = '\n';
        --Lineno;
        --Mline;
        been_called = 1;
    }

    if (NO_MORE_CHARS())  // Detects end of file
        return 0;

    if (!Eof_read && ii_flush(0) < 0)  // Flushes buffer if necessary
        return -1;

    if (*Next == '\n')  // increases Line number if newline is encountered
        Lineno++;
    
    return(*Next++); 
}

int ii_flush(int force)
{
    /* Flush the input buffer. Do nothing if the current input character isn't
       in the danger zone, otherwise move all unread characters to the left end
       of the buffer and fill the remainder of the buffer */

    int copy_amt, shift_amt;
    uchar* left_edge;
    if (NO_MORE_CHARS())
        return 0;

    if (Eof_read)
        return 1;

    if (Next >= DANGER || force)
    {
        left_edge = pMark ? min(sMark, pMark) : sMark;
        shift_amt = left_edge - Start_buf;

        if (shift_amt < MAXLEX)
        {
            if (!force)
                return -1;

            left_edge = ii_mark_start();
            ii_mark_prev();
			shift_amt = left_edge - Start_buf;
        }

        copy_amt = End_buf - left_edge;
        memcpy_s(Start_buf, copy_amt, left_edge, copy_amt);

        if (!ii_fillbuf(Start_buf + copy_amt))
            printf("Error: ii_flush: buffer full, can't read\n");

        if (pMark)
            pMark -= shift_amt;

        sMark -= shift_amt;
        eMark -= shift_amt;
        Next -= shift_amt;
    }
    return 1;

}

int ii_fillbuf(uchar * starting_at)
{
    /* Fill the input buffer from starting_at to the end 
       Read in units of MAXLEX. 0 is returned if MAXLEX
       cannot be read. Else, the number of characters read
       is returned */

    register unsigned need, // number of bytes required from input
                      got;  // number of bytes actually read

    need = ((END - starting_at) / MAXLEX) * MAXLEX;

    D(printf("Reading %d bytes\n", need);)

	if (need < 0)
		printf("Error: (ii_fillbuff): Bad read-request starting addr.\n");

    if (need == 0)
        return 0;

    if ((got = (*Readp)(Inp_file, starting_at, need)) == -1)
		printf("Error (ii_fillbuff): Can't read input file.\n");

    End_buf = starting_at + got;

    if (got < need)
        Eof_read = 1;  // At end of file

    return got;
}

int ii_look(unsigned long n)
{
    /* Return the nth character of lookahead, EOF if you tury to look 
       past end of file, or 0 if you try to look bast either 
       end of buffer */

    uchar* p;

    p = Next + (n - 1);

    if (Eof_read && p >= End_buf)
        return EOF;

    return(p < Start_buf || p >= End_buf) ? 0 : *p;
}

int ii_pushback(int n)
{
    /* Push n characters back into the input 
       Can't push back past sMark */

    while (--n >= 0 && Next > sMark)
    {
        if (*--Next == '\n' || !*Next)  // Decrement Next*
            --Lineno;
    }

    if (Next < eMark)
    {
        eMark = Next;
        Mline = Lineno;
    }

    return (Next > sMark);
}

void ii_term()
{
    Termchar = *Next;
    *Next = '\0';
}

void ii_unterm()
{
    if (Termchar) {
		*Next = Termchar;
		Termchar = 0;
    }
}

int ii_input()
{
    int rval;

    if (Termchar)
    {
        ii_unterm();
        rval = ii_advance();
        ii_mark_end();
        ii_term();
    }
    else
    {
		rval = ii_advance();
		ii_mark_end();
    }
    
    return rval;
}

void ii_unput(char c)
{
    if (Termchar)
    {
        ii_unterm();
        if (ii_pushback(1))
            *Next = c;
        ii_term();
    }
    else
    {
        if (ii_pushback(1))
            *Next = c;
    }
}

int ii_lookahead(unsigned long n)
{
    return (n == 1 && Termchar) ? Termchar : ii_look(n);
}

int ii_flushbuf()
{
    if (Termchar)
        ii_unterm();

    return ii_flush(1);
}


