/* 
**  (C) by Remo Dentato (rdentato@gmail.com)
** 
** This software is distributed under the terms of the BSD license:
**   http://creativecommons.org/licenses/BSD/
**
** Based on "explicit.tcl" by Hudson Lacerda
**
*/

/* .% Overview
==============

  This is an example of use for ABCp. It adds explicit accidentals to
each note that doesn't have one. It will uses the K: field for the key and will
hold accidentals that happen in the same measure.

*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <ctype.h>
#include "abcp.h"

float keysig[7];
float barsig[7];

float lastbend;
int   lastnote;
float endingbend;
int   endingnote;
int tie; 

void printtok(abcScanner *scn)
{
  printf("%.*s",abcTokenLen(scn,0),abcTokenStart(scn,0));
}

void printnotetok(abcScanner *scn, float bend)
{
  int note;
  
  note = abcNote2Num(*abcTokenStart(scn,5));
  
  if (bend != 0.0) {
    barsig[note] = bend;
    printtok(scn);
    lastnote = note;
    lastbend = bend;
    return;
  }

  if (tie && lastnote == note && lastbend != 0.0) 
    bend = lastbend;
  else
    bend = barsig[note];
  
  lastnote = note;
  lastbend = bend;
  
  if (bend == abcNatural) {
    printf("=");
  }
  else {
    while (bend <= -100.0) {
      printf("_");
      bend += 100.0;
    }
    if (bend < 0.0) {
      printf("_%.2f",bend);
    } 
    while (bend >= 100.0) {
      printf("^");
      bend -= 100.0;
    }
    if (bend > 0.0) {
      printf("^%.2f",bend);
    }
  } 
  printf("%.*s",abcTokenLen(scn,5),abcTokenStart(scn,5));
  printf("%.*s",abcTokenLen(scn,6),abcTokenStart(scn,6));
  printf("%.*s",abcTokenLen(scn,7),abcTokenStart(scn,7));
  printf("%.*s",abcTokenLen(scn,8),abcTokenStart(scn,8));
  printf("%.*s",abcTokenLen(scn,9),abcTokenStart(scn,9));
}

void zerosig()
{
  int k;
  for (k=0; k<7; k++) {
    keysig[k] = 0.0;
    barsig[k] = 0.0;
  }  
}

void copysig(float *dest, float *ks, float *es)
{
  int k;
  for (k=0; k<7; k++) {
    if (es[k] != 0.0)  dest[k] = es[k];
    else dest[k] = ks[k];
  }
}

void resetbarsig()
{
  int k;
  for (k=0; k<7; k++) {
    barsig[k] = keysig[k];
  }  
}

int main(int argc, char *argv[])
{
  abcScanner *scn;
  abcToken tok;
  
  if (argc < 2) {
    fprintf(stderr,"Usage: explicit filename\n");
    exit (1);
  }

  scn = abcScannerNew(file,argv[1]);
  if (scn == NULL) {
    fprintf(stderr,"Unable to create an ABC scanner for file: '%s'\n",argv[1]);
    exit (1);    
  }
  
  zerosig();
  resetbarsig();
  lastnote = 0;
  lastbend = 0.0;
  tie = 0;
  
  while ((tok = abcNextToken(scn)) != T_EOF) {
    switch(tok) {
      case T_TIE:
        tie = 1;
        printtok(scn);
        break;
               
      case T_ENDING:
        if (abcBarEnding(scn) == 1) {
          if (tie == 0)
            endingnote = -1;
          else {
            endingnote = lastnote;
            endingbend = lastbend;
          }
        }
        else {
          if (endingnote != -1) {
            lastnote = endingnote;
            lastbend = endingbend;
            tie = 1;
          }
        }
        printtok(scn);
        break;
        
      case T_BAR:
        resetbarsig();
        printtok(scn);
        break;
        
      case T_FIELD : 
        switch (abcField(scn)) {
          case 'K' :
            copysig(keysig, abcKeySignature(scn), abcKeyExpSignature(scn));
            resetbarsig();
            printtok(scn);
            break;
            
          default: printtok(scn);
        }
        break;
        
      case T_NOTE :
        printnotetok(scn,abcNoteBending(scn));
        tie = 0;
        break;
      
      default:
        printtok(scn);
    }
  }

  abcScannerFree(scn);
  return (0);
}
