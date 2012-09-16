#ifndef LIBUTL_H
#define LIBUTL_H
#line 1 "utl.h"
/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the BSD license:
**   http://creativecommons.org/licenses/BSD/
**
*/

#ifndef UTL_H
#define UTL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string.h>
#include <stdarg.h>
#include <stddef.h>

#include <setjmp.h>

/* .% Overview
** ===========
** .v
**                           ___   __
**                       ___/  (_ /  )
**                ___  _(__   ___)  /
**               /  / /  )/  /  /  /
**              /  /_/  //  (__/  /
**             (____,__/(_____(__/
** ..
**
**   This file ('|utl.h|) provide the following basic elements:
**
**  [Logging]      To print logging traces during program execution.
**                 It offers multilevel logging similar to '|log4j| 
**  [Unit Testing] A simple framework to create unit tests. Tests output
**                 is compliant with the TAP '(Test Anything Protocol) standard.
**  [Error handling] Simple handling for serious errors.
** ..   
**   It is part of the '|libutl| toolset but it can also be used on its own 
** by simply including it, with no need to link to other code. See next section
** for the details.
*/

/* .% Variables
** ============
**
**  A set of variable of generic use is provided by '|utl.h| for convenience.
**
**  .[utlEmptyFun]  A pointer to a do-nothing function that can be used
**                  as a generic placeholder (or NULL indicator) for
**                  function pointers. This can be useful as the C standard
**                  doesn't guarantee that one could mix pointers to objects
**                  with pointers to function (even if in reality this is
**                  often the case).  
**  ..
*/

void utlEmptyFun(void);

/*  .[utlEmptyString]  A pointer to the empty string "" that provides unique
**                     representation for the empty string.  
**  ..
*/

extern char *utlEmptyString;

/*   .[utlZero]  Is a constant whose value is 0 and that is to be used in
**               idioms like '|do { ... } while (utlZero)|.
**               This is useful to avoid compilers warning about testing
**               constant values.
**   ..
*/
  
#ifdef __GNUC__
#define utlZero 0
#else
extern const int utlZero;
#endif

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

/* .% Messages Output
** ==================
**
** Log messages and unit tests output normally go on '{stderr}.
** This could be not suitable for applications that are not attached to 
** a console (e.g. background servers, GUI applications).
** 
** Functions and variables in this section allows you to define where to
** redirect those messages.
**
**   '{=utl_stderr} is a global variable holding the current file to be
** used when printing a debug or test message.   It's not intended be used
** directly, use the macro '{=utlStderr} instead if you need to refer to 
** the file.
**
**   Actually I see no reason why you should need it, nut just in case ... 
*/

extern FILE *utl_stderr;
#define utlStderr (utl_stderr? utl_stderr: stderr)

/*   The '/public/ function to set the output file, is '{=utlSetOutput()}
** that takes the filename as input, open the file and and sets '{utl_stderr}
** to it.  If a previous file had been opened, it will be closed. 
**
**   Passing '|NULL| as argument closes the current output file (if there
** was one opened) and sets '{utl_stderr} to '|NULL|, which implies that the
** default '|stderr| will be used from that moment on.
** 
*/
#define utlSetOutput(fname) ((utl_stderr ? fclose(utl_stderr) : 0),\
                             (utl_stderr = (fname? fopen(fname,"w"): NULL)))


/* .% Errors Handling
** ==================
**  Functions to handle serious errors.  
**  The error number is used to differentiate between errors:
**
**   Error codes are decimal numbers between 0 and 9999. The most significant
**  dgit determines which handler is invoked. 
*/

/* '{=utlErrInternal} is provided to avoid repeating the string over
** and over again in the code.
*/

extern char *utlErrInternal; 

#define utl_MAXTRY 8
extern jmp_buf utl_jmp_lst[utl_MAXTRY];
extern int     utl_jmp_cnt;
extern int     utlErr;

/* .%% Try/Catch
** ~~~~~~~~~~~~~~~~~~~~~~~
**  Simple implementation of try/catch. Up to utl_MAXTRY (8) level of nested
** try/catch are allowed but if you use so many level you have have most
** probably big problems. Try to stay simple!
**
**   utlTry {
**      ... code ...
**      if (something_failed) utlThrow(execption_num)  // must be > 0 
**      some_other_func(); // you can trhow exceptions from other functions 
**      ... code ...
**   }  
**   utlCatch(3) {
**      ... code ...
**   }
**   utlCatch(4) {
**      ... code ...
**   }
**   utlCatchAny {  // None of the above! If not present the exception
**                  // will be thrown again at the upper level
**                  // if no handler is found the program exits
**      ... code ...
**   }
**   utlTryEnd ;
** 
*/ 

#define utl_jmp_next ((utl_jmp_cnt < utl_MAXTRY)? utl_jmp_cnt++ : utl_jmp_cnt)
#define utl_jmp_prev ((utl_jmp_cnt > 0)         ? utl_jmp_cnt-- : utl_jmp_cnt)

#define utlTry      do{ int caught = 0; \
                        switch (setjmp(utl_jmp_lst[utl_jmp_cnt])) {  \
                          case 0: if (utl_jmp_cnt<utl_MAXTRY)        \
                                    utl_jmp_cnt++;}                  \
                                  utlErr = 0; caught = 0;
                                  
                              
#define utlCatch(e)               break;                         \
                          case e: caught = 1;
                        
#define utlCatchAny                break;                        \
                          default: caught = 1;
                        
#define utlTryEnd     }                                          \
                      if (caught) utlErr = 0;                    \
                      else if (utlErr) utlThrow(utlErr);         \
                      else if (utl_jmp_cnt > 0) utl_jmp_cnt--;   \
                    } while (utlZero)
                      

/* 
**  The function '{utlError} jumps out of the current function
** and executes the error handler function. If no handler has 
** been defined, it exits
**
*/

#define utlThrow(e) (utlErr=e, (utl_jmp_cnt && utlErr? longjmp(utl_jmp_lst[--utl_jmp_cnt], utlErr) : exit(utlErr)))

#define utlError(e,m)  (logError("ERR: %d %s",e,m), utlThrow(e))


/* .% UnitTest
** ===========
**
**   These macros will help you writing unit tests.  The log produced
** is '<TAP 1.3=http://testanything.org> compatible.
**
**   They are available only if the symbol |UTL_UNITTEST| is defined before
** including the |utl.h| header.
** 
**   '{UTL_UNITTEST} implies '{DEBUG}
*/

#ifdef UTL_UNITTEST

#ifndef DEBUG
#define DEBUG
#endif

#define TSTWRITE(...) (fprintf(utlStderr,__VA_ARGS__),fflush(utlStderr))

#define TSTTITLE(s) TSTWRITE("# ** %s **\nTAP version 13\n\n",s)

/* Tests are divided in sections introduced by '{=TSTSECTION} macro.
** The macro reset the appropriate counters and prints the section header 
*/

#define TSTSECTION(s) if ((TSTSTAT(), TSTGRP = 0, TSTSEC++, TSTPASS=0, \
                       TSTWRITE("#\n# * %d. %s (%s:%d)\n",TSTSEC, s,__FILE__, __LINE__)),!utlZero)

/* to disable an entire test section, just prepend ''|_|' or ''|X|'*/
 
#define _TSTSECTION(s) if (utlZero)  
#define XTSTSECTION(s) if (utlZero)  

/* In each section, tests can be logically grouped so that different aspects
** of related functions can be tested.
*/

#define TSTGROUP(s) if ( TSTNUM=0, \
                     TSTWRITE("#\n# *   %d.%d %s\n", TSTSEC, ++TSTGRP, s),\
                     TSTGRP+1)
                     
/* to disable an intere test group , just prepend ''|_|' or '|X|' */
#define _TSTGROUP(s) if (utlZero)  
#define XTSTGROUP(s) if (utlZero)  

/* You may want to disable just a block of instructions */
#define TSTBLOCK if (!utlZero) 
#define _TSTBLOCK if (utlZero) 
#define XTSTBLOCK if (utlZero) 
                     
/* The single test is defined  with the '|TST(s,x)| macro.
**   .['|s|] is a short string that identifies the test
**    ['|x|] an assertion that has to be true for the test to succeed.
**   ..
*/
#define XTST(s,x)

#define TST(s,x)    (TST_DO(s,(TSTSKP != NULL? 1 : (x))),\
                     (TSTSKP != NULL? TSTWRITE(" # SKIP %s",TSTSKP):0),\
                     TSTWRITE("\n"),TSTRES)

#define TST_DO(s,x) (TSTRES = (x), TSTGTT++, TSTTOT++, TSTNUM++,\
                     TSTWRITE("%s %4d - %s (:%d)",\
                              (TSTRES? (TSTGPAS++,TSTPASS++,TSTOK) : TSTKO),\
                              TSTGTT, s, __LINE__))

#define TSTTODO(s,x,r) (TST_DO(s,x), TSTWRITE(" # TODO %s\n",r), TSTRES)

#define TSTTD TSTTODO

#define TSTFAILED  (!TSTRES)

/* You can skip a set of tests if a condition is not met.
** Nested skips are not supported!
*/
#define TSTSKIP(x,r) (TSTSKP = ((x)? r : NULL))

#define TSTENDSKIP  (TSTSKP = NULL)

#define TSTNOTE(...) (TSTWRITE("      "), TSTWRITE(__VA_ARGS__), TSTWRITE("\n"))

#define TSTONFAIL(...) (TSTRES? 0 : (TSTNOTE(__VA_ARGS__)))

#define TSTBAILOUT(x,r) (TSTRES? TSTRES = 1 : (TSTWRITE("Bail out! %s",r), exit(1)))

/* At the end of a section, the accumulated stats can be printed out */
#define TSTSTAT() \
          (TSTTOT == 0 ? 0 : (\
           TSTWRITE("#\n# SECTION %d PASSED: %d/%d\n",TSTSEC,TSTPASS,TSTTOT),\
           TSTTOT = 0))

/* At the end of all the tests, the accumulated stats can be printed out */
#define TSTDONE() if (TSTGTT > 0) { TSTSTAT(); \
                    TSTWRITE("#\n# TOTAL PASSED: %d/%d\n",TSTGPAS,TSTGTT);\
                    TSTWRITE("#\n# END OF TESTS\n1..%d\n",TSTGTT),fflush(utlStderr);\
                  }

/* Execute a statement if a test succeeded */
#define TSTIF_OK  if (TSTRES)

/* Execute a statement if a test failed */
#define TSTIF_NOTOK if (!TSTRES)

static int TSTRES  = 0;  /* Result of the last performed '|TST()| */
static int TSTNUM  = 0;  /* Last test number */
static int TSTGRP  = 0;  /* Current test group */
static int TSTSEC  = 0;  /* Current test SECTION*/
static int TSTTOT  = 0;  /* Number of tests executed */
static int TSTGTT  = 0;  /* Number of tests executed (Grand Total) */
static int TSTGPAS = 0;  /* Number of tests passed (Grand Total) */
static int TSTPASS = 0;  /* Number of passed tests */

static char       *TSTSKP = NULL;
static const char *TSTOK  = "ok    ";
static const char *TSTKO  = "not ok";

#else /* UTL_UNITTEST */


#define TSTWRITE(...) 

#define TSTTITLE(s) 

#define  TSTSECTION(s) if (utlZero)  
#define _TSTSECTION(s) if (utlZero)  
#define XTSTSECTION(s) if (utlZero)  

#define  TSTGROUP(s) if (utlZero)  
#define _TSTGROUP(s) if (utlZero)  
#define XTSTGROUP(s) if (utlZero)  

#define  TSTBLOCK if (utlZero) 
#define _TSTBLOCK if (utlZero) 
#define XTSTBLOCK if (utlZero) 
                     
#define TST(s,x) 
#define TSTTODO(s,x,r)
#define TSTFAILED  (1)
#define TSTSKIP(x,r)
#define TSTENDSKIP

#define TSTNOTE(...)
#define TSTONFAIL(...)

#define TSTBAILOUT(x,r)

#define TSTDONE() 

#define TSTIF_OK  if (utlZero)
#define TSTIF_NOTOK if (utlZero)

#endif /* UTL_UNITTEST */

/* .% Logging
** ==========
**
** While the debugging functions are meant to be used during the
** development process, logging functions are intended to be used
** in production, after a stable  version of the application has
** been released.
** 
** These functions are modeled after the widely used log4j
** framework but only offers logging to local files.
*/

/* Logging functions are available only if the symbol '{=UTL_LOGGING}
** has been defined before including '|utl.h|.
*/

#define log_All    0
#define log_Debug  1
#define log_Info   2
#define log_Warn   3
#define log_Error  4
#define log_Fatal  5
#define log_Msg    6
#define log_Off    7

extern int log_level;

#ifndef UTL_NOLOGGING
#include <time.h>
#include <ctype.h>
extern char log_timestr[32];
extern time_t log_time;
extern char const *log_abbrev[];

/* .%% Logging levels
** ~~~~~~~~~~~~~~~~~~
**
**   Logging levels are hierarchical and structured as in log4j. The variable 
** '{=log_level} contains the current logging level.  Logging is off by default.
**
**   Use '{=logLevel()} to set the desired level of logging.
*/

#define logLevel(level)     (log_level = (log_##level))
#define logLevelEnv(var,level)  (log_level = log_levelenv(var,log_##level))

int log_levelenv(const char *var, int level);

/*
** The table below shows whether a message of a certain level will be
** printed (Y) or not (N) given the current level of logging.
** .v
**                       message level 
**                    DBG INF WRN ERR FTL
**               DBG   Y   Y   Y   Y   Y
**               INF   N   Y   Y   Y   Y
**      current  WRN   N   N   Y   Y   Y
**      logging  ERR   N   N   N   Y   Y
**       level   FTL   N   N   N   N   Y
**               ALL   Y   Y   Y   Y   Y
**               OFF   N   N   N   N   N
** ..
*/

/* .%% Logging format
** ~~~~~~~~~~~~~~~~~~
** 
** Log files have the following format:
** .v
**     <date> <time> <level> <message>
** ..
**
**  For example:
** .v
**     2009-01-29 13:46:02 ERR An error!
**     2009-01-29 13:46:02 FTL An unrecoverable error
** ..
**
*/

/* .%% Setting the log file
** ~~~~~~~~~~~~~~~~~~~~~~~~
**
**   By default, logging messages will be sent to stderr. The internal
** variable '{=log_file} holds the file descriptor of the file the log
** messages should be sent to. 
**   Don't use it in your program, if you need a pointer to the log file,
** you should use the '{=logFile} macro.
*/

extern FILE *log_file;
#define logFile         (log_file? log_file: stderr) 

/*    Log files can be opened in "write" or "append" mode as any normal file 
** using the '{=logOpen()} function.
** For example:
** .v  
**   logOpen("file1.log","w") // Delete old log file and create a new one
**   ...
**   logOpen(NULL,NULL); // Close log file, send next messages to stderr
**   ... 
**   logOpen("file1.log","a") // Append to previous log file
** .. 
*/

#define logOpen(fname,mode) ((log_file? fclose(log_file) : 0),\
                                (log_file = fname? fopen(fname,mode) : NULL))

#define logClose() logOpen(NULL,NULL)

/*   To actually write a message on the log file, use the '{=logWrite()}
** function as if it was a '|printf()| with the exception that the first
** paratmeter is the level of the message.
** Example:
** .v
**    logWrite(log_Info,"Something weird at %d is happening", counter);
** ..
*/

#define logWrite(lvl,...) \
       (lvl >= log_level  \
          ? (time(&log_time),\
             strftime(log_timestr,32,"%Y-%m-%d %X",localtime(&log_time)),\
             fprintf(logFile,"\n%s %s ",log_timestr, log_abbrev[lvl]),\
             fprintf(logFile,__VA_ARGS__),\
             fflush(logFile)\
            )\
          : 0)

/* You can also use one of the following functions that won't require you 
** to pass the message level as parameter:
*/          
#define logDebug(...)    logWrite(log_Debug, __VA_ARGS__)
#define logInfo(...)     logWrite(log_Info,  __VA_ARGS__)
#define logWarn(...)     logWrite(log_Warn,  __VA_ARGS__)
#define logError(...)    logWrite(log_Error, __VA_ARGS__)
#define logFatal(...)    logWrite(log_Fatal, __VA_ARGS__)
#define logMessage(...)  logWrite(log_Msg,   __VA_ARGS__)

/* If you want to add something to the log file without creating a new entry
** in the log file, you can use the '{=logMessage()} function. 
*/
#define logNote(...)  (fprintf(logFile,__VA_ARGS__), fflush(logFile))

#define logIndent     "\n                        "    
#define logContinue(...)  (fputs(logIndent,logFile),logNote(__VA_ARGS__))                       

/*   To ease text alignment in the log, the string '{=logIndent} contains 
** the spaces needed to pass the date, time and type field.
** For example:
** .v
**   logError("Too many items at counter %d (%d)",numcounter,numitems);
**   logContinue("Occured %d times",times++);
** ..
** will produce:
** .v
**     2009-01-29 13:46:02 ERR Too many items at counter 9 (5)
**                             Occured 3 times
** ..
*/

#define logIf(lvl) if (log_level >= log_##lvl)

#else

#define logLevel(level)         (log_level=log_Off)
#define logLevelEnv(v,l)        (log_level=log_Off)
#define logFile stderr          (log_level=log_Off)
#define logOpen(fname,mode)     (log_level=log_Off)
#define logClose()              (log_level=log_Off)
#define logWrite(lvl,...)       (log_level=log_Off)
#define logDebug(...)           (log_level=log_Off)
#define logInfo(...)            (log_level=log_Off)
#define logWarn(...)            (log_level=log_Off)
#define logError(...)           (log_level=log_Off)
#define logFatal(...)           (log_level=log_Off)
#define logMessage(...)         (log_level=log_Off)

#define logIf(x) if (utlZero)

#endif /*- UTL_LOGGING */

#ifdef NDEBUG
#undef logDebug
#define logDebug(...)
#endif

#define _logDebug(...)

/* dbgmsg shouldn't be used */
#define  dbgmsg  logDebug
#define _dbgmsg _logDebug

/*  .% Finite state machine
**  =======================
**
**  Macros to embed Finite state machine into C programs.
**
** .v
**      fsmStart(FSM_NAME) {
**        fsmState(x) { ...
**                   if (c == 0) fsmGoto(z);
**                   fsmGoto(y);
**        }
**
**        fsmState(y) { ...
**                   fsmExit(FSM_NAME);  // exit from the FSM
**        }
*
**        fsmState(z) { ...
**                   fsmGosub(t);
**        }
**
**        fsmState(t) { ...
**                   fsmReturn;  // Go back to the caller state
**        }
**      }
**      fsmEnd(FSM_NAME);
** ..
**
**   It's a good practice to include a graphic of the FSM in the technical
** documentation (e.g including the GraphViz description in comments).
*/

#define fsmStart(x) for(;;){\
          int utl_fsmcur; \
          int utl_fsmcnt; \
          int utl_fsmret; \
          int utl_fsmrets[16]; \
          for(utl_fsmcnt=0;utl_fsmcnt<16;utl_fsmcnt++) \
            utl_fsmrets[utl_fsmcnt] = 0;\
          for(utl_fsmret=0,utl_fsmcnt=0;;) { \
            goto fsm_##x##_s;\
            fsm_##x##_s : utl_fsmcur = __LINE__ ;  \
                 if (!utl_fsmret || (utl_fsmret == utl_fsmcur && !(utl_fsmret=0)))
                               


#define fsmState(x) \
   fsm_##x##_s : utl_fsmcur = __LINE__ ;  \
                 if (!utl_fsmret || (utl_fsmret == utl_fsmcur && !(utl_fsmret=0)))
                               
#define fsmGoto(x)  goto fsm_##x##_s
#define fsmGosub(x) do { \
                      if (utl_fsmcnt < utl_fsmmax) \
                        utl_fsmrets[utl_fsmcnt++] = utl_fsmcur; \
                      goto fsm_##x##_s; \
                    } while (utlZero) 
                  
#define fsmReturn do { \
                    utl_fsmret = (utl_fsmcnt > 0? utl_fsmrets[--utl_fsmcnt] : 0);\
                    continue; \
                  } while (utlZero) 

#define fsmExit(x) goto fsm_##x##_e 

#define fsmEnd(x) fsm_##x##_e: utl_fsmcnt=0; utl_fsmrets[0]+=0; break;} break;} 


/*  .% Traced memory check
**  ======================
*/

#ifdef UTL_NOMEMCHECK
#ifdef UTL_MEMCHECK
#undef UTL_MEMCHECK
#endif
#endif

void *utl_malloc  (size_t size, char *file, int line );
void *utl_calloc  (size_t num, size_t size, char *file, int line);
void *utl_realloc (void *ptr, size_t size, char *file, int line);
void  utl_free    (void *ptr, char *file, int line );
void *utl_strdup  (void *ptr, char *file, int line);

#define utlMemInvalid    -2
#define utlMemOverflow   -1
#define utlMemValid       0
#define utlMemNull        1

int utl_check(void *ptr,char *file, int line);

#ifdef UTL_MEMCHECK

#ifdef malloc
#undef malloc
#endif 
#define malloc(n) utl_malloc(n,__FILE__,__LINE__)

#ifdef calloc
#undef calloc
#endif 
#define calloc(n,s) utl_calloc(n,s,__FILE__,__LINE__)

#ifdef realloc
#undef realloc
#endif 
#define realloc(p,n) utl_realloc(p,n,__FILE__,__LINE__)

#ifdef free
#undef free
#endif 
#define free(p) utl_free(p,__FILE__,__LINE__)

#ifdef strdup
#undef strdup
#endif 
#define strdup(p) utl_strdup(p,__FILE__,__LINE__)

#define utlMemCheck(p) utl_check(p,__FILE__, __LINE__)

#else /* UTL_MEMCHECK */

#define utlMemCheck(p) utlMemValid

#endif /* UTL_MEMCHECK */


#endif /* UTL_H */

#line 1 "pmx.h"
/* 
** .: (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the BSD license:
**   http://creativecommons.org/licenses/BSD/
**   http://opensource.org/licenses/bsd-license.php
**
** ..
*/


#ifndef PMX_H
#define PMX_H

#include <stddef.h>

/*
.v
                __ __  __ __ __ ___  ___ 
               |  '_ \|  '  '  |\  \/  / 
               |  |_) |  |  |  | /    /  
               |  ___/|__/__/__|/__/\__\ 
               |  |
               |__)
..
  
*/
#define pmxCaptMax 10

typedef size_t pmxMatches[pmxCaptMax+2][2];
typedef pmxMatches *pmx_t;

extern int pmx_capt_cur;

#define pmxCaptStkSize 8
#define pmxMatchesPush()  (pmx_capt_cur = (pmx_capt_cur +1) & 0x07)
#define pmxMatchesPop()   (pmx_capt_cur = (pmx_capt_cur + pmxCaptStkSize -1) & 0x07)

pmx_t pmx_matchstr(char *txt, char *ptrn, size_t offset);
#define pmxMatchStr(t, p) pmx_matchstr(t,p,0)

int           pmxMatched (pmx_t mtc);

size_t        pmxLen     (pmx_t mtc, unsigned char n);
size_t        pmxStart   (pmx_t mtc, unsigned char n);
size_t        pmxEnd     (pmx_t mtc, unsigned char n);

char *        pmxStartP  (pmx_t mtc, unsigned char n);
char *        pmxEndP    (pmx_t mtc, unsigned char n);

unsigned char pmxToken(pmx_t mtc);

#define pmxTokStart(x) (pmx_tmpstr+pmxStart(pmx_tmpmtc,x))
#define pmxTokEnd(x)   (pmx_tmpstr+pmxEnd(pmx_tmpmtc,x))
#define pmxTokLen(x)   pmxLen(pmx_tmpmtc,x)

#define pmxTokNONE    x00
#define pmxTokEOF     x7F
#define pmxTokIGNORE  x80
#define pmxTokERROR   xFF

#define pmxTok_defcase(y) 0##y 
#define pmxTok_defstr(y)  #y 
#define pmxTokCase(y) case pmxTok_defcase(y)

// tested with MS Visual C++ 2008 Express and 2010 Express
#if defined(_MSC_VER)
#define pmxTok_defstr_msvc(x,y) pmxTok_defstr(x##y)
#define pmxTokSet(y,x) "&|" x pmxTok_defstr_msvc(&\,y)
#else 
#define pmxTokSet(y,x) "&|" x pmxTok_defstr(&\y)
#endif


#define pmxTokGroupSet(x)     (pmx_group = (x))
#define pmxTokGroupGet()       pmx_group
#define pmxTokGroupMain        0
#define pmxTokGroupBegin    ( (pmx_group == 0) ? 
#define pmxTokGroup(n)      : (pmx_group == (n)) ?   
#define pmxTokGroupEnd      : "")

#define pmxSwitch(s,p) \
 do {\
    char *pmx_tmpstr;\
    pmx_t pmx_tmpmtc;\
    switch ( ((pmx_tmpstr = s) && *s ) \
                 ? (pmx_tmpmtc = pmxMatchStr(pmx_tmpstr,p), \
                           s += pmxLen(pmx_tmpmtc,0), pmxToken(pmx_tmpmtc))\
                 : 0x7F )
             
#define pmxSwitchEnd   } while(0)

typedef int (*pmxScanFun_t)(char *txt, pmx_t mtc);

int pmxScanStr(char* text, char *ptrn, pmxScanFun_t f);

#define pmxScannerBegin(s) \
 do {\
    char *pmx_tmpstr;\
    char *pmx_tmpptrn;\
    int   pmx_group = 0;\
    pmx_t pmx_tmpmtc;\
    if (s == NULL || *s == (char)pmx_group ) break;\
    pmx_tmpmtc = NULL;\
    pmx_tmpstr = s;\
    for(;;) {\
      if (pmx_tmpmtc) {\
        pmx_tmpstr += pmxLen(pmx_tmpmtc,0);  \
        pmxMatchesPop();\
        pmx_tmpmtc = NULL;\
      }\
      pmx_tmpptrn = 
      
#define pmxScannerLoop \
      ;\
      
#define pmxScannerSwitch \
      ;\
      pmx_tmpmtc = pmxMatchStr(pmx_tmpstr, pmx_tmpptrn);\
      pmxMatchesPush();\
      if (pmx_tmpmtc) {\
        switch (pmxToken(pmx_tmpmtc)) {\

#define pmxScannerEnd  \
        } \
      }  break; \
    }     \
    pmxMatchesPop(); \
  } while (0)


#define pmxScanner(s,p,c) pmxScannerBegin(s)\
                            p \
                          pmxScannerSwitch \
                            c \
                          pmxScannerEnd;

#endif
#line 1 "chs.h"
/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This sofwtare is distributed under the terms of the BSD license:
**   http://creativecommons.org/licenses/BSD/
**   http://opensource.org/licenses/bsd-license.php 
*/

#ifndef CHS_H
#define CHS_H   

/*
.v
                     __        
                    |  |        
                 ___|_ |__  ____
                /  __/ __ \/ ___)
               (  (_|  || |\___ \
                \___/__||_|/____/
..

         ===============================
.T             Character strings 
         ===============================
            
.A       Remo Dentato (rdentato@gmail.com)


   .% Overview
   ===========

 C strings have two major drawbacks:
 
  .- They cannot easily resized 
   - They are zero-terminated (meaning that getting the
     lenght of a strings costs O(n))
  ..

 This functions help handling variable length string. 

 
   .% Naming Conventions
   =====================
   
  Not all the functions, macros and variables exposed in this include file
are for general use. To distinguish names in the public API from names of
objects reserved for internal use, the following convention is used:
.>
    Public names are written in CamelCase
..


   .% The "block"
   ==================
  
  The following diagram shows the structure of a '|chs| string.

.v 
    __ this is the start of memory block
   |   allocated for each chs string
   |
   |                __ used when a chs is stored
   v               |   in a container
  +------+------+--v---+----------------------------
  | size | len  |  up  |  <--- string ... ---> \0
  +--A---+--A---+------+----------------------------
     |      |           A
     |      |           |__ this is the "string" pointer that is
     |      |               returned and by the chs functions
     |      |
     |      |__ used space (length of the string)
     |
     |__ allocated space     
..

as implemented by the '{=chs_blk_t} structure below.

  The '/string/ part at the end is incremented in steps of '{=chs_blk_inc} bytes.
*/
 
 
#define chs_blk_inc 16

typedef struct {
  long   size;
  long   len;
  void   **up;
  char chs[chs_blk_inc];
} chs_blk_t;

/*
  All the '|chs| functions return a pointer to the '{chs[]} array within the 
'{chs_blk_t} structure. This allows '|chs| strings to be passed to the standard
C string functions.  

  Given a '|chs| string, to get back the associated information the 
macro '{=chs_blk} is used. 

*/

#define chs_blk(s) ((chs_blk_t *)(((char*)(s)) - offsetof(chs_blk_t,chs)))

/* .% Types
   =========

.['|chs_t|]  This is the type of a '|chs| string. It is equivalent to a 
             '|char *| so that it can be passed to any function accepting
             standard C strings.
             Of course the opposite is not true, passing a '|char *| to
             a function expecting a '|chs_t| will most likely result in
             a memory corruption and program crash.  
..
*/

typedef char *chs_t ;

/* .% Auxiliary variables
   ======================
   
   The following defines and variables are used within the '|chs| macros
   and defined in the '<chs.c> file.  
*/


/*
.% API
=========

  All public functions have a CamelCase name, the only exception is the
type '{=chs_t}.  Every other identifier that contains an underscore (''_|')
is to be considered as reserved for internal use.

  A key concept to bear in mind is that the address of a '|chs| string may
change when the string is modified.  This makes address of '|chs| strings 
unsuitable to being stored in two different variable at the same time.

  The creation/modification functions take the chs string as first parameter
and can modify its value
*/

/*  .%% Creating/disposing strings
    ''''''''''''''''''''''''''''''
  .['|chsNew(s)|]   Set s to a freshly created, empty '|chs| string and
                    returns it.
  
   ['|chsDup(s)|]   Returns a freshly created '|chs| string, filled with
                    the content of the string '|s|.
                    '|s| can be a '|chs| string or a regular C string.
   
   ['|chsFree(s)|]  Frees the memory allocated for the string '|s| and
                    returns NULL. '|s| is reset to NULL and avoid the
                    risk of accessing de-allocated memory.
  .. 
*/

chs_t chs_setsize(chs_t s, long ndx);
#define chsNew(s) (s = chs_setsize(NULL,0)) 

#define chsDup(s)     chs_Cpy(NULL,s)
#define chsDupL(s,l)  chs_CpyL(NULL,s,l)

chs_t chs_Free(chs_t s);
#define chsFree(s) (s=chs_Free(s))

long chsLen(chs_t s);
long chsSize(chs_t s);

char   chsChrAt  (chs_t s, long ndx);

chs_t chs_Set(chs_t s, long ndx, char c);
#define chsSetChr(s, n, c) (s = chs_Set(s,n,c))

/*  .%% Modifying strings
    '''''''''''''''''''''
  Functions that add/remove content from a string. Whenever used, indexes
can be negative meaning they start at the end of the string (i.e index '|-1|
is the last character of the string).  
    
  .['|chsCpy(d,s)|]
        Copies the content of the string s in the '|chs| string d.
        
   ['|chsCpyL(d,s,l)|]
        Copies the content of the string s in the '|chs| string d
        up to l characters. 
        
   ['|chsAddChr(d,c)|]
        Appends a character to the '|chs| string d. 
        
   ['|chsAddStr(d,s)|]
        Appends a string to the '|chs| string d. 
        
   ['|chsAddStrL(d,s,l)|]
        Appends a string to the '|chs| string d up to l characters. 
        
   ['|chsInsChr(d,n,c)|]
        Inserts a character in the '|chs| string d at the specified index n. 
        
   ['|chsInsStr(d,n,s)|]
        Inserts a string to the '|chs| string d at the specified index n. 
        
   ['|chsInsStrL(d,n,s,l)|]
        Inserts a string to the '|chs| string d at the specified index n
        up to l characters.
         
   ['|chsCpyFmt(d,f,...)|]
        Format a string according to '|f| (same sintax as the C '|printf()|)
        and copies it into d. 
        
   ['|chsAddFmt(d,f,...)|]
        Format a string according to '|f| (same sintax as the C '|printf()|)
        and appends it at the end of d. 
        
   ['|chsInsFmt(d,n,f,...)|]
        Format a string according to '|f| (same sintax as the C '|printf()|)
        and insert it into d at the speficied index n. 
        
   ['|chsDel(d,b,e)|]
        Deletes the content of the string from the index '|b| to the index '|e|.
         
   ['|chsChrAt(s,n)|]  
        Returns the character at the position n.

   ['|chsSetChr(s,n,c)|]  
        Writes the character '|c| at the position '|n|. 
  ..    
*/

chs_t chs_CpyL (chs_t dst, char *src, long len) ;
#define chsCpyL(d, s, l) (d = chs_CpyL(d,s,l))

chs_t chs_Cpy(chs_t dst, char *src) ;
#define chsCpy(d, s) (d = chs_Cpy(d,s))

chs_t chs_AddChr(chs_t dst, char c) ;
#define chsAddChr(d, c) (d = chs_AddChr(d,c))

chs_t chs_AddStrL(chs_t dst, char *src, long len) ;
#define chsAddStrL(d, s, l) (d = chs_AddStrL(d,s,l))
#define chsCatL chsAddStrL

chs_t chs_AddStr(chs_t dst, char *src) ;
#define chsAddStr(d, s) (d = chs_AddStr(d,s))
#define chsCat chsAddStr

chs_t chs_InsChr(chs_t dst, long ndx, char c) ;
#define chsInsChr(d, n, c) (d = chs_InsChr(d,n,c))

chs_t chs_InsStrL(chs_t dst, long ndx, char *src, long len) ;
#define chsInsStrL(d, n , s, l) (d = chs_InsStrL(d,n,s,l))

chs_t chs_InsStr(chs_t dst, long ndx, char *src) ;
#define chsInsStr(d, n, s) (d = chs_InsStr(d,n,s))

#define chsPrintf(d,f,...)      \
  do {                          \
    long n = snprintf(NULL, 0, f, __VA_ARGS__ );\
    chsSetChr(d,n,'\0');        \
    sprintf(d,f,__VA_ARGS__);   \
  } while (utlZero)
 
chs_t chs_Del (chs_t dst, long from, long to) ;
#define chsDel(d, f, t) (d = chs_Del(d,f,t))

/*  .%% Reading from files
    ''''''''''''''''''''''
  This functions read content of a '|FILE *| into a string.

   .['|chsCpyFile(s,f)|]  Copies the content of the file from the current
                          position to the end of file into the string s.
   
    ['|chsAddFile(s,f)|]  Append to the string s the content of the file
                          from the current position to the end of file.
                          
    ['|chsCpyLine(s,f)|]  Copies the content of the file from the current
                          position to the end of line (\n or \r\n or \n)
                          into the string s.
   
    ['|chsAddLine(s,f)|]  Append to the string s the content of the file
                          from the current position to the end of line (\n
                          or \r\n or \n).
                          
    ['|chsForLines(l,f)|] Reads the content of the file f one lines at the
                          time in the string l and executes the next
                          instruction until the end of file is reached.
                          For example to print lines in a file:
.v
           chs_t l;
           FILE *f;
             ...
           f = fopen("myfile.txt","r");  
           chsForLines(l,f) {
             printf("<%s>\n",l);
           }
           chsFree(l);
           if (f) fclose(f);
             ...
..  
   
   ..

*/

chs_t chs_read(chs_t dst, FILE *f, char how, char what);

#define chsRead(s,f,h)   (s = chs_read(s,f,h,'A'))
#define chsReadln(s,f,h) (s = chs_read(s,f,h,'L'))

#define chsCpyFile(s,f) (s = chs_read(s,f,'w','A'))
#define chsCpyLine(s,f) (s = chs_read(s,f,'w','L'))

#define chsAddFile(s,f) (s = chs_read(s,f,'a','A'))
#define chsAddLine(s,f) (s = chs_read(s,f,'a','L'))

#define chsForLines(l,f) \
     while (chsLen(chsCpyLine(l,f)) > 0)

/* .%% Matching strings
   ''''''''''''''''''''
*/

pmx_t chsMatch(chs_t s, long from, char *pat) ;

typedef char *(*chsSubF_t)(char *mtc, pmx_t cpt);

chs_t chs_SubStr(chs_t s, long pos, char *pat, char *rpl) ;
#define chsSubStr(s, n, p, r) (s = chs_SubStr(s,n,p,r))

chs_t chs_SubArr(chs_t s, long pos, char *pat, char **arr) ;
#define chsSubArr(s, n, p, r) (s = chs_SubArr(s,n,p,r))

chs_t chs_SubFun(chs_t s, long pos, char *pat, chsSubF_t f);
#define chsSubFun(s, n, p, f) (s = chs_SubFun(s,n,p,f))

/* .%% Other  */

#define chsUpper(s) \
 do { char *p=s; if (p) while (*p) {*p = toupper((int)*p); p++;}} while(utlZero)

#define chsUpperL(s) \
 do { char *p=s; int k=n; if (p) while (*p && k--) {*p = toupper((int)*p); p++;}} while(utlZero)
 
#define chsLower(s) \
 do { char *p=s; if (p) while (*p) {*p = tolower((int)*p); p++;}} while(utlZero)

#define chsLowerL(s,n) \
 do { char *p=s; int k=n; if (p) while (*p && k--) {*p = tolower((int)*p); p++;}} while(utlZero)

#define chsReverse(s) \
 do { char *p,*q,t; \
      if ((p=s)) \
        for (q = p+chsLen(p); q > p; p++) { \
          t = *--q ; *q = *p; *p = t; \
        } \
    } while(utlZero)

chs_t chs_Trim(chs_t st,char *left, char *right) ;
#define chsTrim(s,l,r) (s = chs_Trim(s,l,r))

/* .% Alternative syntax
*/

#define chsnew     chsNew            
#define chsfree    chsFree           
#define chslen     chsLen            
                                     
#define chscpy     chsCpy            
#define chsncpy    chsCpyL           
#define chscat     chsAddStr         
#define chsins     chsInsStr         
#define chsncat    chsAddStrL        
#define chsnins    chsInsStrL        
                                     
#define chssetchr  chsChrSet         
#define chscatchr  chsAddChar        
#define chsinschr  chsInsChar        

#define chsprintf chsPrintf
                                     
#define chsdel     chsDel            
#define chstrim    chsTrim           
                                     
#define chsupper   chsUpper          
#define chslower   chsLower          
#define chsnupper  chsUpper          
#define chsnlower  chsLower          
#define chsreverse chsReverse        
                                     
#define chsgetline chsCpyLine        
#define chsgetfile chsCpyFile        
#define chscatline chsAddLine        
#define chscatfile chsAddFile        
#define chslines   chsForLines       
                                     
#define chsmatch   chsMatch          
#define chssub     chsSubStr         
#define chssubarr  chsSubArr         
#define chssubfun  chsSubfun         
#define chssubfun_t chsSubF_t
                                     
#define chschrat   chsChrAt          


#endif  /* CHS_H */
#line 1 "tbl.h"
/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the BSD license:
**   http://creativecommons.org/licenses/BSD/
**   http://opensource.org/licenses/bsd-license.php 
*/

#ifndef TBL_H
#define TBL_H

typedef union {
  void           *p;
  char           *s;
  long            n;
  unsigned long   u;
  float           f;
} val_u;

#define valGetM(v)   ((v).p)
#define valGetT(v)   ((tbl_t)((v).p))
#define valGetV(v)   ((vec_t)((v).p))
#define valGetR(v)   (((v).p))
#define valGetP(v)   ((v).p)
#define valGetS(v)   ((v).s)
#define valGetN(v)   ((v).n)
#define valGetU(v)   ((v).u)
#define valGetF(v)   ((v).f)

#define valM(x) valP(x)
#define valT(x) valP(x)
#define valV(x) valP(x)
#define valR(x) valP(x)
val_u   valP(void *val);        
val_u   valS(char *val);         
val_u   valN(long val);
val_u   valU(unsigned long val);
val_u   valF(float val);


char *val_Sdup(char *s);
char *val_Sfree(char *s);

/********************************************/


#define sltSLOT \
  val_u  val;   \
  char   info[sizeof(val_u)]

typedef struct {
  sltSLOT;
} slt_slot_t;    

typedef slt_slot_t *slt_t;

/* A table slot.
*/ 

typedef struct {
  sltSLOT;
  val_u  key;
} tbl_slot_t;    


typedef struct {
  long count;
  long size;
  unsigned char max_dist;
  unsigned char flags;
  unsigned char pad1;
  unsigned char pad2;
  tbl_slot_t slot[1];  /* start of the slots array */
} tbl_table_t;

typedef tbl_table_t *tbl_t;
 
tbl_t tbl_new(long nslots);
#define tblNew(tb) (tb = tbl_new(2))

tbl_t tbl_free(tbl_t tb);
#define tblFree(tb) (tb = tbl_free(tb)) 
 
#define tblCount(tb) (tb?(tb)->count:0)

tbl_t tbl_set(tbl_t tb, char k_type, val_u key, char v_type, val_u val);

#define tblSetMM(tb,k,v) (tb = tbl_set(tb,'M',valM(k),'M',valM(v)))
#define tblSetMT(tb,k,v) (tb = tbl_set(tb,'M',valM(k),'T',valT(v)))
#define tblSetMV(tb,k,v) (tb = tbl_set(tb,'M',valM(k),'V',valV(v)))
#define tblSetMR(tb,k,v) (tb = tbl_set(tb,'M',valM(k),'R',valR(v)))
#define tblSetMP(tb,k,v) (tb = tbl_set(tb,'M',valM(k),'P',valP(v)))
#define tblSetMS(tb,k,v) (tb = tbl_set(tb,'M',valM(k),'S',valS(val_Sdup(v))))
#define tblSetMH(tb,k,v) (tb = tbl_set(tb,'M',valM(k),'S',valS(v)))
#define tblSetMN(tb,k,v) (tb = tbl_set(tb,'M',valM(k),'N',valN(v)))
#define tblSetMU(tb,k,v) (tb = tbl_set(tb,'M',valM(k),'U',valU(v)))
#define tblSetMF(tb,k,v) (tb = tbl_set(tb,'M',valM(k),'F',valF(v)))

#define tblSetTM(tb,k,v) (tb = tbl_set(tb,'T',valT(k),'M',valM(v)))
#define tblSetTT(tb,k,v) (tb = tbl_set(tb,'T',valT(k),'T',valT(v)))
#define tblSetTV(tb,k,v) (tb = tbl_set(tb,'T',valT(k),'V',valV(v)))
#define tblSetTR(tb,k,v) (tb = tbl_set(tb,'T',valT(k),'R',valR(v)))
#define tblSetTP(tb,k,v) (tb = tbl_set(tb,'T',valT(k),'P',valP(v)))
#define tblSetTS(tb,k,v) (tb = tbl_set(tb,'T',valT(k),'S',valS(val_Sdup(v))))
#define tblSetTH(tb,k,v) (tb = tbl_set(tb,'T',valT(k),'S',valS(v)))
#define tblSetTN(tb,k,v) (tb = tbl_set(tb,'T',valT(k),'N',valN(v)))
#define tblSetTU(tb,k,v) (tb = tbl_set(tb,'T',valT(k),'U',valU(v)))
#define tblSetTF(tb,k,v) (tb = tbl_set(tb,'T',valT(k),'F',valF(v)))

#define tblSetVM(tb,k,v) (tb = tbl_set(tb,'V',valV(k),'M',valM(v)))
#define tblSetVT(tb,k,v) (tb = tbl_set(tb,'V',valV(k),'T',valT(v)))
#define tblSetVV(tb,k,v) (tb = tbl_set(tb,'V',valV(k),'V',valV(v)))
#define tblSetVR(tb,k,v) (tb = tbl_set(tb,'V',valV(k),'R',valR(v)))
#define tblSetVP(tb,k,v) (tb = tbl_set(tb,'V',valV(k),'P',valP(v)))
#define tblSetVS(tb,k,v) (tb = tbl_set(tb,'V',valV(k),'S',valS(val_Sdup(v))))
#define tblSetVH(tb,k,v) (tb = tbl_set(tb,'V',valV(k),'S',valS(v)))
#define tblSetVN(tb,k,v) (tb = tbl_set(tb,'V',valV(k),'N',valN(v)))
#define tblSetVU(tb,k,v) (tb = tbl_set(tb,'V',valV(k),'U',valU(v)))
#define tblSetVF(tb,k,v) (tb = tbl_set(tb,'V',valV(k),'F',valF(v)))

#define tblSetRM(tb,k,v) (tb = tbl_set(tb,'R',valR(k),'M',valM(v)))
#define tblSetRT(tb,k,v) (tb = tbl_set(tb,'R',valR(k),'T',valT(v)))
#define tblSetRV(tb,k,v) (tb = tbl_set(tb,'R',valR(k),'V',valV(v)))
#define tblSetRR(tb,k,v) (tb = tbl_set(tb,'R',valR(k),'R',valR(v)))
#define tblSetRP(tb,k,v) (tb = tbl_set(tb,'R',valR(k),'P',valP(v)))
#define tblSetRS(tb,k,v) (tb = tbl_set(tb,'R',valR(k),'S',valS(val_Sdup(v))))
#define tblSetRH(tb,k,v) (tb = tbl_set(tb,'R',valR(k),'S',valS(v)))
#define tblSetRN(tb,k,v) (tb = tbl_set(tb,'R',valR(k),'N',valN(v)))
#define tblSetRU(tb,k,v) (tb = tbl_set(tb,'R',valR(k),'U',valU(v)))
#define tblSetRF(tb,k,v) (tb = tbl_set(tb,'R',valR(k),'F',valF(v)))

#define tblSetPM(tb,k,v) (tb = tbl_set(tb,'P',valP(k),'M',valM(v)))
#define tblSetPT(tb,k,v) (tb = tbl_set(tb,'P',valP(k),'T',valT(v)))
#define tblSetPV(tb,k,v) (tb = tbl_set(tb,'P',valP(k),'V',valV(v)))
#define tblSetPR(tb,k,v) (tb = tbl_set(tb,'P',valP(k),'R',valR(v)))
#define tblSetPP(tb,k,v) (tb = tbl_set(tb,'P',valP(k),'P',valP(v)))
#define tblSetPS(tb,k,v) (tb = tbl_set(tb,'P',valP(k),'S',valS(val_Sdup(v))))
#define tblSetPH(tb,k,v) (tb = tbl_set(tb,'P',valP(k),'S',valS(v)))
#define tblSetPN(tb,k,v) (tb = tbl_set(tb,'P',valP(k),'N',valN(v)))
#define tblSetPU(tb,k,v) (tb = tbl_set(tb,'P',valP(k),'U',valU(v)))
#define tblSetPF(tb,k,v) (tb = tbl_set(tb,'P',valP(k),'F',valF(v)))

#define tblSetSM(tb,k,v) (tb = tbl_set(tb,'S',valS(val_Sdup(k)),'M',valM(v)))
#define tblSetST(tb,k,v) (tb = tbl_set(tb,'S',valS(val_Sdup(k)),'T',valT(v)))
#define tblSetSV(tb,k,v) (tb = tbl_set(tb,'S',valS(val_Sdup(k)),'V',valV(v)))
#define tblSetSR(tb,k,v) (tb = tbl_set(tb,'S',valS(val_Sdup(k)),'R',valR(v)))
#define tblSetSP(tb,k,v) (tb = tbl_set(tb,'S',valS(val_Sdup(k)),'P',valP(v)))
#define tblSetSS(tb,k,v) (tb = tbl_set(tb,'S',valS(val_Sdup(k)),'S',valS(val_Sdup(v))))
#define tblSetSH(tb,k,v) (tb = tbl_set(tb,'S',valS(val_Sdup(k)),'S',valS(v)))
#define tblSetSN(tb,k,v) (tb = tbl_set(tb,'S',valS(val_Sdup(k)),'N',valN(v)))
#define tblSetSU(tb,k,v) (tb = tbl_set(tb,'S',valS(val_Sdup(k)),'U',valU(v)))
#define tblSetSF(tb,k,v) (tb = tbl_set(tb,'S',valS(val_Sdup(k)),'F',valF(v)))

#define tblSetHM(tb,k,v) (tb = tbl_set(tb,'S',valS(k),'M',valM(v)))
#define tblSetHT(tb,k,v) (tb = tbl_set(tb,'S',valS(k),'T',valT(v)))
#define tblSetHV(tb,k,v) (tb = tbl_set(tb,'S',valS(k),'V',valV(v)))
#define tblSetHR(tb,k,v) (tb = tbl_set(tb,'S',valS(k),'R',valR(v)))
#define tblSetHP(tb,k,v) (tb = tbl_set(tb,'S',valS(k),'P',valP(v)))
#define tblSetHS(tb,k,v) (tb = tbl_set(tb,'S',valS(k),'S',valS(v)))
#define tblSetHH(tb,k,v) (tb = tbl_set(tb,'S',valS(k),'S',valS(v)))
#define tblSetHN(tb,k,v) (tb = tbl_set(tb,'S',valS(k),'N',valN(v)))
#define tblSetHU(tb,k,v) (tb = tbl_set(tb,'S',valS(k),'U',valU(v)))
#define tblSetHF(tb,k,v) (tb = tbl_set(tb,'S',valS(k),'F',valF(v)))

#define tblSetNM(tb,k,v) (tb = tbl_set(tb,'N',valN(k),'M',valM(v)))
#define tblSetNT(tb,k,v) (tb = tbl_set(tb,'N',valN(k),'T',valT(v)))
#define tblSetNV(tb,k,v) (tb = tbl_set(tb,'N',valN(k),'V',valV(v)))
#define tblSetNR(tb,k,v) (tb = tbl_set(tb,'N',valN(k),'R',valR(v)))
#define tblSetNP(tb,k,v) (tb = tbl_set(tb,'N',valN(k),'P',valP(v)))
#define tblSetNS(tb,k,v) (tb = tbl_set(tb,'N',valN(k),'S',valS(val_Sdup(v))))
#define tblSetNH(tb,k,v) (tb = tbl_set(tb,'N',valN(k),'S',valS(v)))
#define tblSetNN(tb,k,v) (tb = tbl_set(tb,'N',valN(k),'N',valN(v)))
#define tblSetNU(tb,k,v) (tb = tbl_set(tb,'N',valN(k),'U',valU(v)))
#define tblSetNF(tb,k,v) (tb = tbl_set(tb,'N',valN(k),'F',valF(v)))

#define tblSetUM(tb,k,v) (tb = tbl_set(tb,'U',valU(k),'M',valM(v)))
#define tblSetUT(tb,k,v) (tb = tbl_set(tb,'U',valU(k),'T',valT(v)))
#define tblSetUV(tb,k,v) (tb = tbl_set(tb,'U',valU(k),'V',valV(v)))
#define tblSetUR(tb,k,v) (tb = tbl_set(tb,'U',valU(k),'R',valR(v)))
#define tblSetUP(tb,k,v) (tb = tbl_set(tb,'U',valU(k),'P',valP(v)))
#define tblSetUS(tb,k,v) (tb = tbl_set(tb,'U',valU(k),'S',valS(val_Sdup(v))))
#define tblSetUH(tb,k,v) (tb = tbl_set(tb,'U',valU(k),'S',valS(v)))
#define tblSetUN(tb,k,v) (tb = tbl_set(tb,'U',valU(k),'N',valN(v)))
#define tblSetUU(tb,k,v) (tb = tbl_set(tb,'U',valU(k),'U',valU(v)))
#define tblSetUF(tb,k,v) (tb = tbl_set(tb,'U',valU(k),'F',valF(v)))

#define tblSetFM(tb,k,v) (tb = tbl_set(tb,'F',valF(k),'M',valM(v)))
#define tblSetFT(tb,k,v) (tb = tbl_set(tb,'F',valF(k),'T',valT(v)))
#define tblSetFV(tb,k,v) (tb = tbl_set(tb,'F',valF(k),'V',valV(v)))
#define tblSetFR(tb,k,v) (tb = tbl_set(tb,'F',valF(k),'R',valR(v)))
#define tblSetFP(tb,k,v) (tb = tbl_set(tb,'F',valF(k),'P',valP(v)))
#define tblSetFS(tb,k,v) (tb = tbl_set(tb,'F',valF(k),'S',valS(val_Sdup(v))))
#define tblSetFH(tb,k,v) (tb = tbl_set(tb,'F',valF(k),'S',valS(v)))
#define tblSetFN(tb,k,v) (tb = tbl_set(tb,'F',valF(k),'N',valN(v)))
#define tblSetFU(tb,k,v) (tb = tbl_set(tb,'F',valF(k),'U',valU(v)))
#define tblSetFF(tb,k,v) (tb = tbl_set(tb,'F',valF(k),'F',valF(v)))

val_u tbl_get(tbl_t tb, char k_type, val_u key, char v_type, val_u def);

#define tblGetSM(tb,k,d) valGetM(tbl_get(tb,'S',valS(k),'M',valM(d)))
#define tblGetST(tb,k,d) valGetT(tbl_get(tb,'S',valS(k),'T',valT(d)))
#define tblGetSV(tb,k,d) valGetV(tbl_get(tb,'S',valS(k),'V',valV(d)))
#define tblGetSR(tb,k,d) valGetR(tbl_get(tb,'S',valS(k),'R',valR(d)))
#define tblGetSP(tb,k,d) valGetP(tbl_get(tb,'S',valS(k),'P',valP(d)))
#define tblGetSS(tb,k,d) valGetS(tbl_get(tb,'S',valS(k),'S',valS(d)))
#define tblGetSN(tb,k,d) valGetN(tbl_get(tb,'S',valS(k),'N',valN(d)))
#define tblGetSU(tb,k,d) valGetU(tbl_get(tb,'S',valS(k),'U',valU(d)))
#define tblGetSF(tb,k,d) valGetF(tbl_get(tb,'S',valS(k),'F',valF(d)))

#define tblGetNM(tb,k,d) valGetM(tbl_get(tb,'N',valN(k),'M',valM(d)))
#define tblGetNT(tb,k,d) valGetT(tbl_get(tb,'N',valN(k),'T',valT(d)))
#define tblGetNV(tb,k,d) valGetV(tbl_get(tb,'N',valN(k),'V',valV(d)))
#define tblGetNR(tb,k,d) valGetR(tbl_get(tb,'N',valN(k),'R',valR(d)))
#define tblGetNP(tb,k,d) valGetP(tbl_get(tb,'N',valN(k),'P',valP(d)))
#define tblGetNS(tb,k,d) valGetS(tbl_get(tb,'N',valN(k),'S',valS(d)))
#define tblGetNN(tb,k,d) valGetN(tbl_get(tb,'N',valN(k),'N',valN(d)))
#define tblGetNU(tb,k,d) valGetU(tbl_get(tb,'N',valN(k),'U',valU(d)))
#define tblGetNF(tb,k,d) valGetF(tbl_get(tb,'N',valN(k),'F',valF(d)))

#define tblGetUM(tb,k,d) valGetM(tbl_get(tb,'U',valU(k),'M',valM(d)))
#define tblGetUT(tb,k,d) valGetT(tbl_get(tb,'U',valU(k),'T',valT(d)))
#define tblGetUV(tb,k,d) valGetV(tbl_get(tb,'U',valU(k),'V',valV(d)))
#define tblGetUR(tb,k,d) valGetR(tbl_get(tb,'U',valU(k),'R',valR(d)))
#define tblGetUP(tb,k,d) valGetP(tbl_get(tb,'U',valU(k),'P',valP(d)))
#define tblGetUS(tb,k,d) valGetS(tbl_get(tb,'U',valU(k),'S',valS(d)))
#define tblGetUN(tb,k,d) valGetN(tbl_get(tb,'U',valU(k),'N',valN(d)))
#define tblGetUU(tb,k,d) valGetU(tbl_get(tb,'U',valU(k),'U',valU(d)))
#define tblGetUF(tb,k,d) valGetF(tbl_get(tb,'U',valU(k),'F',valF(d)))

#define tblGetFM(tb,k,d) valGetM(tbl_get(tb,'F',valF(k),'M',valM(d)))
#define tblGetFT(tb,k,d) valGetT(tbl_get(tb,'F',valF(k),'T',valT(d)))
#define tblGetFV(tb,k,d) valGetV(tbl_get(tb,'F',valF(k),'V',valV(d)))
#define tblGetFR(tb,k,d) valGetR(tbl_get(tb,'F',valF(k),'R',valR(d)))
#define tblGetFP(tb,k,d) valGetP(tbl_get(tb,'F',valF(k),'P',valP(d)))
#define tblGetFS(tb,k,d) valGetS(tbl_get(tb,'F',valF(k),'S',valS(d)))
#define tblGetFN(tb,k,d) valGetN(tbl_get(tb,'F',valF(k),'N',valN(d)))
#define tblGetFU(tb,k,d) valGetU(tbl_get(tb,'F',valF(k),'U',valU(d)))
#define tblGetFF(tb,k,d) valGetF(tbl_get(tb,'F',valF(k),'F',valF(d)))


tbl_t tbl_del(tbl_t tb, char k_type, val_u key);

#define tblDelM(tb,k) (tb = tbl_del(tb,'M',valM(k)))
#define tblDelT(tb,k) (tb = tbl_del(tb,'T',valT(k)))
#define tblDelV(tb,k) (tb = tbl_del(tb,'V',valV(k)))
#define tblDelR(tb,k) (tb = tbl_del(tb,'R',valR(k)))
#define tblDelP(tb,k) (tb = tbl_del(tb,'P',valP(k)))
#define tblDelS(tb,k) (tb = tbl_del(tb,'S',valS(k)))
#define tblDelN(tb,k) (tb = tbl_del(tb,'N',valN(k)))
#define tblDelU(tb,k) (tb = tbl_del(tb,'U',valU(k)))
#define tblDelF(tb,k) (tb = tbl_del(tb,'F',valF(k)))

typedef long tblptr_t;

tblptr_t tblNext(tbl_t tb, tblptr_t ndx);
#define tblFirst(tb) tblNext(tb,0)

#define tblForeach(tb,i) for(i=tblFirst(tb); i != 0; i = tblNext(tb,i))


tblptr_t tbl_find(tbl_t tb, char k_type, val_u key);

#define tblFindM(tb,k)  tbl_find(tb,'M',valM(k))
#define tblFindT(tb,k)  tbl_find(tb,'T',valT(k))
#define tblFindV(tb,k)  tbl_find(tb,'V',valT(k))
#define tblFindR(tb,k)  tbl_find(tb,'R',valT(k))
#define tblFindP(tb,k)  tbl_find(tb,'P',valP(k))
#define tblFindS(tb,k)  tbl_find(tb,'S',valS(k))
#define tblFindN(tb,k)  tbl_find(tb,'N',valN(k))
#define tblFindU(tb,k)  tbl_find(tb,'U',valU(k))
#define tblFindF(tb,k)  tbl_find(tb,'F',valF(k))

char tblKeyType(tbl_t tb, tblptr_t ndx);
char tblValType(tbl_t tb, tblptr_t ndx);

val_u tbl_key(tbl_t tb, tblptr_t ndx);

#if 0
val_u tbl_val(tbl_t tb, tblptr_t ndx);
#endif 

#define tbl_val(tb,ndx) (tb->slot[ndx-1].val)

#define tblKeyM(tb,n)  valGetM(tbl_key(tb,n))
#define tblKeyT(tb,n)  valGetT(tbl_key(tb,n))
#define tblKeyV(tb,n)  valGetV(tbl_key(tb,n))
#define tblKeyR(tb,n)  valGetR(tbl_key(tb,n))
#define tblKeyP(tb,n)  valGetP(tbl_key(tb,n))
#define tblKeyS(tb,n)  valGetS(tbl_key(tb,n))
#define tblKeyN(tb,n)  valGetN(tbl_key(tb,n))
#define tblKeyU(tb,n)  valGetU(tbl_key(tb,n))
#define tblKeyF(tb,n)  valGetF(tbl_key(tb,n))

#define tblValM(tb,n)  valGetM(tbl_val(tb,n))
#define tblValT(tb,n)  valGetT(tbl_val(tb,n))
#define tblValV(tb,n)  valGetV(tbl_val(tb,n))
#define tblValR(tb,n)  valGetR(tbl_val(tb,n))
#define tblValP(tb,n)  valGetP(tbl_val(tb,n))
#define tblValS(tb,n)  valGetS(tbl_val(tb,n))
#define tblValN(tb,n)  valGetN(tbl_val(tb,n))
#define tblValU(tb,n)  valGetU(tbl_val(tb,n))
#define tblValF(tb,n)  valGetF(tbl_val(tb,n))

void tbl_print(FILE *f, tbl_t tb);
#define tblPrint(f,t) tbl_print(f,t)

/******************/

typedef struct {
  sltSLOT;
} vec_slot_t;

typedef struct {
  long count;
  long size;
  long cur;
  long stride;
  vec_slot_t slot[1];
} vec_vector_t;

typedef vec_vector_t *vec_t;

vec_t vec_setsize(vec_t vt, long nslots);

#define vecNew(v)  (v = vec_setsize(NULL, 2))


vec_t vec_free(vec_t vt) ;
#define vecFree(v) (v = vec_free(v));

long vecSize(vec_t vt);
long vecCount(vec_t vt);


vec_t vec_set(vec_t vt, long ndx, char v_type, val_u val, int del);

#define vecSetM(vt,n,v)  (vt = vec_set(vt, n, 'M', valM(v),1))
#define vecSetT(vt,n,v)  (vt = vec_set(vt, n, 'T', valT(v),1))
#define vecSetV(vt,n,v)  (vt = vec_set(vt, n, 'V', valV(v),1))
#define vecSetR(vt,n,v)  (vt = vec_set(vt, n, 'R', valR(v),1))
#define vecSetP(vt,n,v)  (vt = vec_set(vt, n, 'P', valP(v),1))
#define vecSetS(vt,n,v)  (vt = vec_set(vt, n, 'S', valS(val_Sdup(v)),1))
#define vecSetH(vt,n,v)  (vt = vec_set(vt, n, 'S', valS(v),1))
#define vecSetN(vt,n,v)  (vt = vec_set(vt, n, 'N', valN(v),1))
#define vecSetU(vt,n,v)  (vt = vec_set(vt, n, 'U', valU(v),1))
#define vecSetF(vt,n,v)  (vt = vec_set(vt, n, 'F', valF(v),1))

#define vecSetZM(vt,n,v)  (vt = vec_set(vt, n, 'M', valM(v),0))
#define vecSetZT(vt,n,v)  (vt = vec_set(vt, n, 'T', valT(v),0))
#define vecSetZV(vt,n,v)  (vt = vec_set(vt, n, 'V', valV(v),0))
#define vecSetZR(vt,n,v)  (vt = vec_set(vt, n, 'R', valR(v),0))
#define vecSetZP(vt,n,v)  (vt = vec_set(vt, n, 'P', valP(v),0))
#define vecSetZS(vt,n,v)  (vt = vec_set(vt, n, 'S', valS(val_Sdup(v)),0))
#define vecSetZH(vt,n,v)  (vt = vec_set(vt, n, 'S', valS(v),0))
#define vecSetZN(vt,n,v)  (vt = vec_set(vt, n, 'N', valN(v),0))
#define vecSetZU(vt,n,v)  (vt = vec_set(vt, n, 'U', valU(v),0))
#define vecSetZF(vt,n,v)  (vt = vec_set(vt, n, 'F', valF(v),0))

#define vecAddM(vt,v)   vecSetM(vt,vecCount(vt),v)
#define vecAddT(vt,v)   vecSetT(vt,vecCount(vt),v)
#define vecAddV(vt,v)   vecSetV(vt,vecCount(vt),v)
#define vecAddR(vt,v)   vecSetR(vt,vecCount(vt),v)
#define vecAddP(vt,v)   vecSetP(vt,vecCount(vt),v)
#define vecAddS(vt,v)   vecSetS(vt,vecCount(vt),v)
#define vecAddH(vt,v)   vecSetH(vt,vecCount(vt),v)
#define vecAddN(vt,v)   vecSetN(vt,vecCount(vt),v)
#define vecAddU(vt,v)   vecSetU(vt,vecCount(vt),v)
#define vecAddF(vt,v)   vecSetF(vt,vecCount(vt),v)

vec_t vec_ins(vec_t vt, long ndx, char v_type, val_u val);

#define vecInsM(vt,n,v)  (vt = vec_ins(vt, n, 'M', valM(v)))
#define vecInsT(vt,n,v)  (vt = vec_ins(vt, n, 'T', valT(v)))
#define vecInsV(vt,n,v)  (vt = vec_ins(vt, n, 'V', valV(v)))
#define vecInsR(vt,n,v)  (vt = vec_ins(vt, n, 'R', valR(v)))
#define vecInsP(vt,n,v)  (vt = vec_ins(vt, n, 'P', valP(v)))
#define vecInsS(vt,n,v)  (vt = vec_ins(vt, n, 'S', valS(val_Sdup(v))))
#define vecInsH(vt,n,v)  (vt = vec_ins(vt, n, 'S', valS(v)))
#define vecInsN(vt,n,v)  (vt = vec_ins(vt, n, 'N', valN(v)))
#define vecInsU(vt,n,v)  (vt = vec_ins(vt, n, 'U', valU(v)))
#define vecInsF(vt,n,v)  (vt = vec_ins(vt, n, 'F', valF(v)))

val_u vec_get(vec_t vt, long ndx, char v_type, val_u def);

#define vecGetM(vt,n,d) valGetM(vec_get(vt,n,'M',valM(d)))
#define vecGetT(vt,n,d) valGetT(vec_get(vt,n,'T',valT(d)))
#define vecGetV(vt,n,d) valGetV(vec_get(vt,n,'V',valV(d)))
#define vecGetR(vt,n,d) valGetR(vec_get(vt,n,'R',valR(d)))
#define vecGetP(vt,n,d) valGetP(vec_get(vt,n,'P',valP(d)))
#define vecGetS(vt,n,d) valGetS(vec_get(vt,n,'S',valS(d)))
#define vecGetN(vt,n,d) valGetN(vec_get(vt,n,'N',valN(d)))
#define vecGetU(vt,n,d) valGetU(vec_get(vt,n,'U',valU(d)))
#define vecGetF(vt,n,d) valGetF(vec_get(vt,n,'F',valF(d)))

val_u vec_getz(vec_t vt, long ndx, char v_type, val_u def);

#define vecGetZM(vt,n,d) valGetM(vec_getz(vt,n,'M',valM(d)))
#define vecGetZT(vt,n,d) valGetT(vec_getz(vt,n,'T',valT(d)))
#define vecGetZV(vt,n,d) valGetV(vec_getz(vt,n,'V',valV(d)))
#define vecGetZR(vt,n,d) valGetR(vec_getz(vt,n,'R',valR(d)))
#define vecGetZP(vt,n,d) valGetP(vec_getz(vt,n,'P',valP(d)))
#define vecGetZS(vt,n,d) valGetS(vec_getz(vt,n,'S',valS(d)))
#define vecGetZN(vt,n,d) valGetN(vec_getz(vt,n,'N',valN(d)))
#define vecGetZU(vt,n,d) valGetU(vec_getz(vt,n,'U',valU(d)))
#define vecGetZF(vt,n,d) valGetF(vec_getz(vt,n,'F',valF(d)))

val_u vec_slot_get(vec_slot_t *slot, char v_type, val_u def);

#define vecSlotGetM(n,d) valGetM(vec_slot_get(n,'M',valM(d)))
#define vecSlotGetT(n,d) valGetT(vec_slot_get(n,'T',valT(d)))
#define vecSlotGetV(n,d) valGetV(vec_slot_get(n,'V',valV(d)))
#define vecSlotGetR(n,d) valGetR(vec_slot_get(n,'R',valR(d)))
#define vecSlotGetP(n,d) valGetP(vec_slot_get(n,'P',valP(d)))
#define vecSlotGetS(n,d) valGetS(vec_slot_get(n,'S',valS(d)))
#define vecSlotGetN(n,d) valGetN(vec_slot_get(n,'N',valN(d)))
#define vecSlotGetU(n,d) valGetU(vec_slot_get(n,'U',valU(d)))
#define vecSlotGetF(n,d) valGetF(vec_slot_get(n,'F',valF(d)))

#define vecSlots(v) ((v)->slot)

#define vecValM(vt,n)  valGetM(vecSlots(vt)[n])
#define vecValT(vt,n)  valGetT(vecSlots(vt)[n])
#define vecValV(vt,n)  valGetV(vecSlots(vt)[n])
#define vecValR(vt,n)  valGetR(vecSlots(vt)[n])
#define vecValP(vt,n)  valGetP(vecSlots(vt)[n])
#define vecValS(vt,n)  valGetS(vecSlots(vt)[n])
#define vecValN(vt,n)  valGetN(vecSlots(vt)[n])
#define vecValU(vt,n)  valGetU(vecSlots(vt)[n])
#define vecValF(vt,n)  valGetF(vecSlots(vt)[n])

char vecType(vec_t vt, long ndx);
char vecValType(vec_slot_t *slot);

vec_t vec_del(vec_t vt, long from, long to);
#define vecDel(vt,f,t)  (vt = vec_del(vt, f,t))

int vec_cmp (const void *a, const void *b);
#define vecSort(v)  qsort((v)->slot, vecCount(v) , sizeof(vec_slot_t), vec_cmp)

#define vecSortP(v,f)  qsort((v)->slot, vecCount(v) , sizeof(vec_slot_t), f)

#define vecStrideSet(v,n)  (v? (v)->stride = n: n)
#define vecStrideCount(v)  (vecCount(v)/((v)->stride))

#define vecStrideSortF(v,n,f) qsort(vecSlots(v), vecCount(v)/(n),\
                                        sizeof(vec_slot_t)*(n), f)


vec_t vec_split(char *s, char *sep,char *trim, int dup);

#define vecSplit(s, sep, trim) vec_split(s,sep,trim,1)
#define vecSplitP(s, sep, trim) vec_split(s,sep,trim,0)


/*****************/

/* .%% Stack discipline */

#define stk_t vec_t

#define stkNew(tb)       vecNew(tb)
#define stkFree(tb)      vecFree(tb)

#define stkPushM(tb,v)   vecAddM(tb,v)
#define stkPushT(tb,v)   vecAddT(tb,v)
#define stkPushV(tb,v)   vecAddV(tb,v)
#define stkPushR(tb,v)   vecAddR(tb,v)
#define stkPushP(tb,v)   vecAddP(tb,v)
#define stkPushS(tb,v)   vecAddS(tb,v)
#define stkPushH(tb,v)   vecAddH(tb,v)
#define stkPushN(tb,v)   vecAddN(tb,v)
#define stkPushU(tb,v)   vecAddU(tb,v)
#define stkPushF(tb,v)   vecAddF(tb,v)

#define stkIsEmpty(tb)   (vecCount(tb) == 0)
#define stkTopType(tb)   vecType(tb,-1)

#define stkTopM(tb)      vecGetM(tb,vecCount(tb),NULL)
#define stkTopT(tb)      vecGetT(tb,vecCount(tb),NULL)
#define stkTopV(tb)      vecGetV(tb,vecCount(tb),NULL)
#define stkTopR(tb)      vecGetR(tb,vecCount(tb),NULL)
#define stkTopP(tb)      vecGetP(tb,vecCount(tb),NULL)
#define stkTopS(tb)      vecGetS(tb,vecCount(tb),NULL)
#define stkTopN(tb)      vecGetN(tb,vecCount(tb),0)
#define stkTopU(tb)      vecGetN(tb,vecCount(tb),0)
#define stkTopF(tb)      vecGetF(tb,vecCount(tb),0.0)

#define stkPop(tb)       vecDel(tb,-1,-1)

/*****************/

#define que_t vec_t

#define queNew(qu)       vecNew(qu)
#define queFree(qu)      vecFree(qu)
#define queAddFirst(qu,v)
#define queAddLast(qu,v)
#define queDelFirst(qu)
#define queDelLast(qu)
#define queCount(qu)

#define queFirstType(qu)
#define queLastType(qu)

#define queFirst
#define queLast

/*****************/

#define que_t vec_t

#define prqNew(qu)       vecNew(qu)
#define prqFree(qu)      vecFree(qu)
#define prqAdd(qu,v,p)
#define prqDelFirst(qu)
#define prqDelLast(qu)
#define prqCount(qu)

#define prqFirstType(qu)
#define prqLastType(qu)

#define prqFirstPri(qu)
#define prqLastPri(qu)

#define prqFirst
#define prqLast

/*****************/
struct rec_f_t {
  int size;
  char  *name;
  int   (*cmp)  (void * , void *) ;
  void *(*cpy)  (void * , void *) ;
  char *(*uid)  (void *) ;
  void  (*del) (void *) ;
};

typedef struct { struct rec_f_t  *rec_f; } *rec_t;

#define rec(t,y) \
  struct rec_##t##_f;\
  typedef struct t##_s { struct rec_##t##_f *rec_f; y } t; \
  int    t##_cmp(t *a, t *b); \
  void   t##_cpy(t *a, t *b);\
  char  *t##_uid(t *a);\
  void   t##_del(t *a); \
  void   t##_init(t *); \
  char  *t##_name = #t; \
  struct rec_##t##_f {\
    int    size;\
    char  *name; \
    int   (*cmp)  (t * , t *) ;\
    void  (*cpy)  (t * , t *) ;\
    char *(*uid)  (t *) ;\
    void  (*del) (t *) ;\
  }

#define recFunCmp(t,a,b)   int   t##_cmp(t *a, t *b)
#define recFunCpy(t,a,b)   void  t##_cpy(t *a, t *b)
#define recFunUid(t,a)     char *t##_uid(t *a)
#define recFunFree(t,a)    void  t##_del(t *a)

#define recFunNew(t,a) \
    static struct rec_##t##_f rec_##t##_func; \
    t *t##_new() {\
      t *p = calloc(1,sizeof(t));      \
      rec_##t##_func.size = sizeof(t); \
      rec_##t##_func.name = t##_name;  \
      rec_##t##_func.cmp  = t##_cmp;   \
      rec_##t##_func.cpy  = t##_cpy;   \
      rec_##t##_func.uid  = t##_uid;   \
      rec_##t##_func.del  = t##_del;  \
      if (p) {\
        p->rec_f = &rec_##t##_func;\
        t##_init(p);\
      }\
      return p;\
    }\
    void t##_init(t *a)

#define recDef    rec
#define recDefCmp recFunCmp
#define recDefCpy recFunCpy
#define recDefUid recFunUid
#define recDefDel recFunFree
#define recDefFree recFunFree
#define recDefNew recFunNew

#define recCpy(a,b)  (a = rec_cpy((rec_t)a,(rec_t)b))

#define recNew(t,r)  (r = t##_new())

#define recFree(r)  \
  (r = (void *)(r? (( (rec_t)(r))->rec_f->del(r),free(r),NULL):NULL))

#define recSize(a)   ((a)? ((rec_t)(a))->rec_f->size : 0)

#define recUid(a)    ((a)? ((rec_t)(a))->rec_f->uid(a) : NULL)

#define recName(a)   ((a)? ((rec_t)(a))->rec_f->name : NULL)

#define recPtrCmp(a,b) ((char *)(a) - (char *)(b))

void *rec_cpy(rec_t a, rec_t b);
int   rec_cmp(rec_t a, rec_t b);

#define recCmp(a,b) rec_cmp((rec_t)a, (rec_t)b)

#define recIs(t,r) (r && (void *)(((rec_t)r)->rec_f) == (&rec_##t##_func))

/******************************/

#define lutItem(x,y) {x,y},

typedef struct { char *key;  int   val; } lutSN_slot_t;
typedef struct { int   key;  int   val; } lutNN_slot_t;
typedef struct { char *key;  char *val; } lutSS_slot_t;

typedef lutSN_slot_t lutSN_t[];
typedef lutNN_slot_t lutNN_t[];
typedef lutSS_slot_t lutSS_t[];

#define lutBegin(tt,lt) lut##tt##_t lt = {
#define lutEnd(lt)   } ; int lut_##lt##_size = sizeof(lt)/sizeof(lt[0]);

int lut_getSN(lutSN_t lt, int lt_size, char *key, int def);
#define lutGetSN(lt,k,d) lut_getSN(lt,lut_##lt##_size,k,d)

char *lut_getSS(lutSS_t lt, int lt_size, char *key, char *def);
#define lutGetSS(lt,k,d) lut_getSS(lt,lut_##lt##_size,k,d)


#endif
#endif 
