/*****************************************************************************/
/* latticeg.c - Lattice tester for orthomodular lattice algebras             */
/*                                                                           */
/*       Copyright (C) 2000  NORMAN D. MEGILL  <nm@alum.mit.edu>             */
/*             License terms:  GNU General Public License                    */
/*****************************************************************************/
/*34567890123456 (79-character line to adjust text window width) 678901234567*/

#define VERSION "1.3 22-Oct-00"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>

/* Largest lattice size plus 1 */
#define MAX_NODES 101
/* Binary infix operator list - i is only for Polish conversion */
/* Note: i is now obsolete - it used to be "universal implication" -
   but it is still implemented in beran.c and beranf.c */
/* 2/20/00 - Added ~ (NOT) & (AND) V (OR) } (implies) : (bicond)
   classical metalogical connectives */
/* 2/24/00 - Added @ (for all) ] (exists) */
#define ALL_BIN_CONNECTIVES "^v#OI2345=<>[&V}:"
/* MAX_BIN_CONNECTIVES is the length of ALL_BIN_CONNECTIVES */
#define MAX_BIN_CONNECTIVES 17
#define LOGIC_BIN_CONNECTIVES "&V}:"
#define LOGIC_AND_RELATION_CONNECTIVES "=<>[&V}:~"
#define OPER_AND_RELATION_BIN_CONNECTIVES "^v#OI2345=<>["
#define QUANTIFIER_CONNECTIVES "@]"
/* Lattice binary operations */
/* (In the future direct references to operation symbols will be
   changed to use these constants) */
#define INF_OPER '^'
#define SUP_OPER 'v'
#define ID_OPER '#'
#define IMP0_OPER 'O'
#define IMP1_OPER 'I'
#define IMP2_OPER '2'
#define IMP3_OPER '3'
#define IMP4_OPER '4'
#define IMP5_OPER '5'
/* Lattice relations */
#define EQ_OPER '='
#define LE_OPER '<'
#define GE_OPER '>'
#define COM_OPER '['
/* Negation prefix operator */
#define NEG_OPER '-'
/* 2/20/00 - Added classical metalogical NOT */
#define NOT_OPER '~'
/* 2/20/00 - Added classical binary metalogical connectives */
#define AND_OPER '&'
#define OR_OPER 'V'
#define IMPL_OPER '}'
#define BI_OPER ':'
/* 2/20/00 - Added classical truth values - these should be different
   from ASCII letters to avoid ambiguity, but less than MAX_NODES to
   avoid operation table overflow, and nonzero to avoid end-of-string
   problems */
#define FALSE_CONST 1
#define TRUE_CONST 2
/* 2/24/00 - Added quantifiers */
#define FORALL_OPER '@'
#define EXISTS_OPER ']'
/* Old "universal implication" - now obsolete */
#define UNIV_IMPL 'i'
/* List of legal variable names - note v,i,o skipped because of operators */
#define VAR_LIST "abcdefghjklmnpqrstuwxyz"
/* MAX_VARS is the length of VAR_LIST */
#define MAX_VARS 23
/* MAX_STACK is length of longest formula expressed in Polish notation */
#define MAX_STACK 10000
/* MAX_STACK2 is the maximum nesting level of a formula */
#define MAX_STACK2 1000
/* Largest number of user hypotheses */
#define MAX_HYPS 50
/* Largest number of lattices */
#define MAX_LATTICES 1000

/* Mapping for Greechie diagram atoms */
#define ATOM_MAP "123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrs" \
    "tuvwxyz!\"#$%&'()*-/:;<=>?@[\\]^_`{|}~"
/* Maximum number of blocks */
#define MAX_BLOCKS 64
/* Minimum block size - only 3 and 4 are implemented */
#define MIN_BLOCK_SIZE 3
/* Maximum block size - only 3 and 4 are implemented */
#define MAX_BLOCK_SIZE 4

/******************** Start of string handling prototypes ********************/
typedef char* vstring;

/* String assignment - MUST be used to assign vstrings */
void let(vstring *target,vstring source);
/* String concatenation - last argument MUST be NULL */
vstring cat(vstring string1,...);

/* Emulate BASIC linput statement; returns NULL if EOF */
/* Note that linput assigns target string with let(&target,...) */
  /*
    BASIC:  linput "what";a$
    c:      linput(NULL,"what?",&a);

    BASIC:  linput #1,a$                        (error trap on EOF)
    c:      if (!linput(file1,NULL,&a)) break;  (break on EOF)

  */
vstring linput(FILE *stream,vstring ask,vstring *target);

/* Emulation of BASIC string functions */
vstring seg(vstring sin, long p1, long p2);
vstring mid(vstring sin, long p, long l);
vstring left(vstring sin, long n);
vstring right(vstring sin, long n);
vstring edit(vstring sin, long control);
vstring space(long n);
vstring string(long n, char c);
vstring chr(long n);
vstring xlate(vstring sin, vstring control);
vstring date(void);
vstring time_(void);
vstring num(double x);
vstring num1(double x);
vstring str(double x);
long len(vstring s);
long instr(long start, vstring sin, vstring s);
long ascii_(vstring c);
double val(vstring s);
/* Emulation of PROGRESS string functions added 11/25/98 */
vstring entry(long element, vstring list);
long lookup(vstring expression, vstring list);
long numEntries(vstring list);
long entryPosition(long element, vstring list);
/* Print to log file as well as terminal if fplog opened */
void print2(char* fmt,...);
FILE *fplog = NULL;
/* Opens files with error message; opens output files with
   backup of previous version.   Mode must be "r" or "w". */
FILE *fSafeOpen(vstring fileName, vstring mode);


/******* Special pupose routines for better
      memory allocation (use with caution) *******/
/* Make string have temporary allocation to be released by next let() */
/* Warning:  after makeTempAlloc() is called, the vstring may NOT be
   assigned again with let() */
void makeTempAlloc(vstring s);   /* Make string have temporary allocation to be
                                    released by next let() */
/* Remaining prototypes (outside of mmvstr.h) */
char *tempAlloc(long size);     /* String memory allocation/deallocation */

#define MAX_ALLOC_STACK 100
int tempAllocStackTop=0;        /* Top of stack for tempAlloc functon */
int startTempAllocStack=0;      /* Where to start freeing temporary allocation
                                    when let() is called (normally 0, except in
                                    special nested vstring functions) */
char *tempAllocStack[MAX_ALLOC_STACK];

/* Bug check error */
void bug(int bugNum);

/******************** End of string handling prototypes **********************/

/******************** Prototypes *********************************************/
void usersGreechieDiagrams(void);
void printhelp(void);
void testAllLattices(void);
void findCommutingPairs(void);
void init(void);
void initLattice(long latticeNum);

void greechie3(vstring name, vstring glattice);
unsigned char test(void);
vstring unabbreviate(vstring eqn);
vstring subformulaList(vstring eqn);
vstring toPolish(vstring equation);
long subEqnLen(vstring subEqn);
vstring fromPolish(vstring polishEqn);
unsigned char eval(vstring trialEqn, long eqnLen);
unsigned char fastEval(vstring trialEqn, long startChar);
unsigned char ultraFastEval(vstring trialEqn, long eqnLen);
vstring subformula(vstring eqn);
long subformulaLen(vstring eqn, long startChar);
unsigned char lookupCompl(unsigned char arg);
unsigned char lookupNot(unsigned char arg);
unsigned char lookupBinOp(unsigned char operation, unsigned char arg1,
    unsigned char arg2);
vstring unPrintableString(vstring sin);
vstring printableString(vstring sin);

/******************** Global variables ***************************************/
vstring nodeList[MAX_NODES];
vstring sup[MAX_NODES][MAX_NODES]; /* Supremum (disjunction) table */
vstring nodeNames = ""; /* List of node names in nodeList order */
long nodes;
vstring latticeName = ""; /* Name of lattice being worked with */
vstring equation = ""; /* Expression being tested */
vstring hypList[MAX_HYPS]; /* List of user's hypotheses (0 or more) */
vstring polHypList[MAX_HYPS]; /* List of user's hypotheses in Polish */
long hypotheses;
vstring conclusion = ""; /* User's conclusion to test */
vstring polConclusion = ""; /* Concl in Polish, with quantifiers stripped */
vstring quantifiers = ""; /* Quantifiers in prefix of prenex normal form */
vstring quantifierTypes = ""; /* @ or ] in order of occurrence */
vstring quantifierVars = ""; /* Quantified vars in order of occurrence */
vstring failingAssignment = ""; /* Assignment that violated lattice */
long userArg; /* -1 = test all; n = test only nth lattice */
long startArg = 0; /* Lattice # to start from */
long endArg = 0; /* Lattice # to end at */
/* Special for Greechie diagram program latticeg.c */
vstring lletterlist = ""; /* Node names for atoms */
vstring uletterlist = ""; /* Node names for complemented atoms */
long glatticeNum;
long glatticeCount;
vstring greechieStmt = ""; /* Greechie lattice C statement */
char showGreechie = 0; /* Set to 1 by -g option */
char removeLegs = 0; /* Set to 1 by -l option */
FILE *fp = NULL;  /* Input file of greechie diagrams */
long fileLineNum = 0; /* Most recent line number (= lattice number) read */
long fileLattices = 0; /* Number of input file diagrams */


unsigned char negMap[256]; /* Map of negatives of ASCII node names
    for speedup */
unsigned char opMap[256]; /* Map of from ASCII operator to position in
    ALL_BIN_CONNECTIVES for speedup */
unsigned char nodeNameMap[256]; /* Map from ASCII node name to lattice node
    number for speedup */

/* Binary operation table for fast lookup */
unsigned char binOpTable[MAX_BIN_CONNECTIVES][MAX_NODES][MAX_NODES];
unsigned char ultraFastBinOpTable[MAX_BIN_CONNECTIVES][256][256];

/* Special flags for commuting pair option */
char commuteOption; /* Flag that user specified -c program option */
char OADoneFlag; /* Flag for last lattice passing the OA law */
char OMDoneFlag; /* Flag for last lattice passing the OM law */

/* For -v (show lattice points visited) option */
char showVisits = 0; /* Set to 1 to show all
                        lattice points visited by failure.  Useful for
                        identifying unnecessary (redundant) lattice points. */
char visitFlag = 0; /* Used internally when showVisits is 1 */
vstring visitList = ""; /* List of nodes visited */

/* For -f (show all failures) option */
char showAllFailures = 0;  /* Set to 1 to show
                        all possible lattice failures.  Useful for comparing
                        lattice behavior of two different equations. */
void usersGreechieDiagrams(void)
{
  /*** Start of user's Greechie diagrams ***/
  /* The first argument of greechie3, if non-blank, is the lattice
     name - otherwise a default name is generated */
  /* The second argument uses the matrix notation in
     Kalmbach, "Orthomodular Lattices", p. 319, with the matrix
     represented as a comma-separated linear string. */

  greechie3("L42 (OM, OA)",
  "1 2 3  3 4 5  5 6 7  7 8 9  9 10 11  11 12 13  13 14 1"
  "  3 15 10  6 16 13  8 17 18  13 19 18  3 20 17");
  /* Alternate numbering for L42 (in oapass.oml): */
  /*
  "1 2 3  1 4 5  1 6 7  1 8 9  2 10 11  4 12 13  6 14 15"
    "  8 16 17  10 12 14  11 16 18  13 16 19  15 16 20");
  */

  greechie3("L40 (OM, OA)",
  "1 2 11  1 3 12  1 4 13  5 6 7  8 9 10  2 5 14"
  "  2 8 15  3 6 16  3 9 17 4 7 18  4 10 19");

  greechie3("Mayet Fig. 5 (OM, OA)",
   "1 2 3  1 4 7  2 5 8  3 6 9  7 12 14  8 10 12  8 11 13  9 13 16  14 15 16");

  greechie3("L38 (OM, non-OA)",
      "1,2,3,"
      "3,4,5,"
      "5,6,7,"
      "7,8,9,"
      "9,10,11,"
      "11,12,13,"
      "13,14,1,"
      "12,15,4,"
      "15,16,17,"
      "17,18,6");

  greechie3("L36 (OM, non-OA)",
  " 1 2 11  1 3 12  4 5 6  4 7 8  5 9 10  1 6 13"
  "  2 7 14  2 9 15  3 8 16  3 10 17");

  greechie3("L36b (OM, non-OA)",
  "1 2 3  1 4 5  2 6 7  3 8 9  4 10 11  5 12 13"
  "  6 10 14  7 12 15 8 11 16  9 13 14  15 16 17");

  greechie3("L$ (OM, non-OA)",
  "1 2 3  3 4 5  5 6 7  7 8 9  9 10 11  11 12 1  10 13 4");

  /*** End of user's Greechie diagrams ***/
} /* usersGreechieDiagrams() */



/******************** Main program *******************************************/

int main(int argc, char *argv[])
{

  vstring str1 = "";
  vstring str2 = "";
  long i, j, argOffset;
  char argOffsetChanged;
  vstring contLine = ""; /* Continuation line */
  char printLatticeFlag = 0;

  /* argc is the number of arguments; argv points to array containing them */

  /* Line continuation for DOS: "$x" means get argument x from prompt */
  for (i = 1; i < argc; i++) {
    let(&str1, argv[i]);
    if (str1[0] == '$') {
      if (str1[0] == '\'' || str1[0] == '\"') {
        /* Strip quotes */
        let(&str1, seg(str1, 2, strlen(str1) - 1));
      }
      /* Strip off successive "$x"s and replace with continuation lines */
      while (str1[0] == '$') {
        let(&str2, cat(left(str1, 2), "> ", NULL));
        linput(NULL, str2, &contLine);
        let(&str1, cat(right(str1, 3), contLine, NULL));
      }
      /* Note that we'll just give up and not free the memory argv[i] points
         to; it seems the safest thing to do since it may not be a real malloc
         object depending on compiler */
      /* To be extra safe we might want to clone the argv pointer array first
         before reassigning it; maybe in a future version */
      argv[i] = str1;
      str1 = ""; /* Relinquish string ownership to argv[i] (instead of let()) */
    }
  }
  let(&str1, ""); /* Deallocate string */
  let(&str2, ""); /* Deallocate string */
  let(&contLine, ""); /* Deallocate string */

  if (argc <= 1) {
    /* No arguments means print help */
    printhelp();
    return 0;
  }

  /* Get command line options */
  /* Important: none of the options have the syntax of a legal wff.   This
     should be the case for any future options as well. */
  userArg = 0;
  commuteOption = 0;
  argOffset = 0;
  argOffsetChanged = 1;
  while (argOffsetChanged) {
    argOffsetChanged = 0;
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "--help")) {
      printhelp();
      return 0;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-a")) {
      argOffset++;
      argOffsetChanged = 1;
      userArg = -1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-c")) {
      argOffset++;
      argOffsetChanged = 1;
      commuteOption = 1;
      /* This is not implement for latticeg.c, only lattice.c */
      print2("-c is implemented in lattice.c only\n");
      return 0;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-v")) {
      argOffset++;
      argOffsetChanged = 1;
      showVisits = 1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-f")) {
      argOffset++;
      argOffsetChanged = 1;
      showAllFailures = 1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-g")) {
      argOffset++;
      argOffsetChanged = 1;
      showGreechie = 1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-l")) {
      argOffset++;
      argOffsetChanged = 1;
      removeLegs = 1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-i")) {
      if (argc - argOffset > 2) {
        /* Open the output file */
        fp = fopen(argv[2 + argOffset], "r");
        if (fp == NULL) {
          print2("?Error: Couldn't open the file \"%s\".\n",
              argv[2 + argOffset]);
          return 0;
        }
      } else {
        print2("?Error: No input file specified.\n");
        return 0;
      }
      argOffset += 2;
      argOffsetChanged = 1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-o")) {
      if (argc - argOffset > 2) {
        if (fplog != NULL) {
          print2("?Error: Cannot specify more than one output file.\n");
          return 0;
        }
        /* Open the output file */
        fplog = fSafeOpen(argv[2 + argOffset], "w");
        if (fplog == NULL) {
          return 0;
        }
        /* Disable buffering of the output file so that all partial results
           will be there in case a run is aborted before completion */
        setbuf(fplog, NULL);
      } else {
        print2("?Error: No output log file specified.\n");
        return 0;
      }
      argOffset += 2;
      argOffsetChanged = 1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "--o")) {
      if (argc - argOffset > 2) {
        if (fplog != NULL) {
          print2("?Error: Cannot specify more than one output file.\n");
          return 0;
        }
        /* Open the output file - append mode */
        fplog = fopen(argv[2 + argOffset], "a");
        if (fplog == NULL) {
          print2("?Error: Couldn't open the file \"%s\".\n",
              argv[2 + argOffset]);
          return 0;
        }
        /* Disable buffering of the output file so that all partial results
           will be there in case a run is aborted before completion */
        setbuf(fplog, NULL);
      } else {
        print2("?Error: No output log file specified.\n");
        return 0;
      }
      argOffset += 2;
      argOffsetChanged = 1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-n")) {
      if (argc - argOffset > 2) userArg = val(argv[2 + argOffset]);
      if (userArg <= 0  || strcmp(argv[2 + argOffset], str(userArg))) {
        print2("?Error: Expected positive integer after -n\n");
        return 0;
      }
      argOffset += 2;
      argOffsetChanged = 1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-s")) {
      if (argc - argOffset > 2) startArg = val(argv[2 + argOffset]);
      if (startArg <= 0  || strcmp(argv[2 + argOffset], str(startArg))) {
        print2("?Error: Expected positive integer after -s\n");
        return 0;
      }
      argOffset += 2;
      argOffsetChanged = 1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-e")) {
      if (argc - argOffset > 2) endArg = val(argv[2 + argOffset]);
      if (endArg <= 0  || strcmp(argv[2 + argOffset], str(endArg))) {
        print2("?Error: Expected positive integer after -e\n");
        return 0;
      }
      argOffset += 2;
      argOffsetChanged = 1;
    }
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-p")) {
      if (argc - argOffset > 2) userArg = val(argv[2 + argOffset]);
      if (userArg <= 0  || strcmp(argv[2 + argOffset], str(userArg))) {
        print2("?Error: Expected positive integer after -p\n");
        return 0;
      }
      printLatticeFlag = 1;
      argOffset += 2;
      argOffsetChanged = 1;
    }
  }

  init(); /* One-time initialization */

  if (printLatticeFlag) {
    /* Print lattice for user */
    i = 0;
    if (fp != NULL) i = i + 2;
    if (fplog != NULL) i = i + 2;
    if (argc != 3 + i) {
      print2(
          "?Error: Only the -i and -o (or --o) options may be used with -p\n");
      return 0;
    }
    initLattice(userArg);
    if (!nodes && userArg > 0) {
      print2("?Error: Lattice %ld does not exist\n", userArg);
      return 0;
    }
    print2("Name of lattice %ld: %s\n", userArg, latticeName);
    print2("Number of nodes: %ld\n", nodes);
    print2("Greechie diagram:\n");
    print2(" %s\n", greechieStmt);
    print2("The lattice nodes map to Greechie atoms: a=1, b=2, etc.\n");
    print2("The lattice node names are capitalized to indicate complement.\n");
    print2("\n");
    print2("  Node  Nodes directly below it\n");
    print2("  ----  -----------------------\n");
    for (i = 1; i <= nodes; i++) {
      let(&str1, right(nodeList[i], 2));
      for (j = strlen(str1); j > 1; j--) {
        let(&str1, cat(left(str1, j - 1), ", ", right(str1, j), NULL));
      }
      let(&str1, cat("  ", left(nodeList[i], 1), "     ",
          str1, NULL));
      print2("%s\n", printableString(str1));
    }
    return 0;
  } /* if printLatticeFlag */

  /* Assign user's hypotheses and conclusion */
  hypotheses = argc - 2 - argOffset;
  for (i = 1; i < argc - argOffset; i++) {
    let(&str2, argv[i + argOffset]);
    /* Strip quotes */
    for (j = 0; j < strlen(str2); j++) {

      if (str2[j] == '\'' || str2[j] == '\"') {
        let(&str2, cat(left(str2, j), right(str2, j + 2), NULL));
      }
    }
    let(&str2, edit(str2, 1 + 2 + 4)); /* Strip spaces & garbage chars */
    if (i != argc - 1 - argOffset) {
      /* Assign hypothesis */
      let(&(hypList[i - 1]), str2);
    } else {
      /* Assign conclusion */
      let(&conclusion, str2);
    }
  }
  let(&str2, ""); /* Deallocate */

  /* Repeat the hypotheses and conclusion for user verification */
  for (i = 0; i < hypotheses; i++) {
    if (i > 0) {
      print2("& ");
    }
    print2("%s ", hypList[i]);
  }
  if (hypotheses > 0) {
    print2("=> ");
  }
  print2("%s\n", conclusion);

  /* Convert to Polish */
  for (i = 0; i < hypotheses; i++) {
    let(&(polHypList[i]), "");
    polHypList[i] = toPolish(hypList[i]);
  }
  let(&polConclusion, "");
  polConclusion = toPolish(conclusion);

  /* Strip off any quantifiers from equation (assumed to be in prenex
     normal form) */
  j = strlen(polConclusion);
  for (i = j - 1; i >= -1; i--) {
    if (i == -1) break;
    if (strchr(QUANTIFIER_CONNECTIVES, polConclusion[i]) != NULL)
      break;
  }
  let(&quantifiers, "");
  if (i >= 0) {
    /* There are quantifiers; remove and save them */
    let(&quantifiers, left(polConclusion, i + 2));
    let(&polConclusion, right(polConclusion, i + 3));
    if (hypotheses > 0) {
      print2("?Error:  Hypotheses are not permitted when using quantifiers\n");
      exit(0);
    }
    /* For each variable in the conclusion, make sure there is a quantifier;
       otherwise add "for all" at the beginning */
    /* Use separate str1 to accumulate so they'll be added in order of
       first occurrence to make test() evaluate them in this order, so
       user will have a known evaluation order */
    let(&str1, "");
    j = strlen(polConclusion);
    for (i = 0; i < j; i++) {
      if (strchr(VAR_LIST, polConclusion[i]) != NULL) {
        if (strchr(quantifiers, polConclusion[i]) == NULL &&
            strchr(str1, polConclusion[i]) == NULL) {
          let(&str1, cat(str1, chr(FORALL_OPER), chr(polConclusion[i]), NULL));
        }
      }
    }
    let(&quantifiers, cat(str1, quantifiers, NULL));
    /* Separate into quantifier types and variables for later efficiency */
    j = strlen(quantifiers);
    let(&quantifierTypes, space(j / 2));
    let(&quantifierVars, space(j / 2));
    for (i = 0; i < j; i = i + 2) {
      quantifierTypes[i / 2] = quantifiers[i];
      quantifierVars[i / 2] = quantifiers[i + 1];
    }
  } /* If there are quantifiers */
  /* If there are no quanifiers, we'll use the fact that quantifierVars is
     empty as an indicator for it */


  if (!commuteOption) {
    testAllLattices(); /* Run the program */
  } else {
    /* Run -c special option to find potentially commuting pairs  */
    findCommutingPairs();
  }

  return 0;
} /* main */

void printhelp(void)
{
vstring a = "";
print2("\n");
print2("latticeg.c - Orthomodular Lattice Evaluator for Greechie Diagrams\n");
print2("Usage: latticeg [options] <hyp> <hyp> ... <conclusion>\n");
print2("         options:\n");
print2("           -a - test all lattices (don't stop after first failure)\n");
print2(
"           -n <integer> - test only the program's <integer>th lattice\n");
print2(
"           -s <integer> - start at the program's <integer>th lattice\n");
print2(
"           -e <integer> - end at the program's <integer>th lattice\n");
print2("           -v - show all visits to lattice points in a failure\n");
print2("           -f - show all failures in failing lattice\n");
print2(
"           -g - categorize Greechie diagrams (legs, pass/fail, etc.)\n");
print2("           -l - skip (ignore) Greechie diagrams with legs\n");
print2(
"           -i <file> - use Greechie lattices from <file> instead of the\n");
print2(
"                       built-in ones\n");
print2(
"           -o <file> - write output to <file> as well as terminal\n");
print2(
"           --o <file> - append output to <file> as well as terminal\n");
print2(
"       latticeg -p <integer> - print the program's <integer>th lattice\n");
print2(
"           (you may use the -i and -o [or --o] options with -p)\n");
print2("       latticeg --help (or no argument) - print this message\n");
print2("Notes: Only one of -a and -n should be used.  -s and -e are applied\n");
print2("before any others.\n");
print2("\n");
print2(
   "Copyright (C) 2000 Norman D. Megill <nm@alum.mit.edu> Version %s\n",
   VERSION);
print2("License terms:  GNU General Public License\n");
linput(NULL,"Press Enter to continue, q to quit...",&a);
if (toupper(a[0]) == 'Q') goto returnPoint;
print2("\n");
print2("This program checks built-in lattices (hard-coded into the\n");
print2("function 'initLattice') for violation of the user's input\n");
print2("conjecture.  Input formulas may have up to 23 variables\n");
print2("a,b,c,d,e,f,g,h,j,k,l,m,n,p,q,r,s,t,u,w,x,y,z (i,o,v omitted)\n");
print2("and constants 0,1.\n");
print2("\n");
print2("The output is of the form \"<result> <name>\" where <name> is\n");
print2("the lattice name.  An assignment that violates the lattice is\n");
print2("given when the <result> is FAILED.\n");
print2("\n");
print2("To compile for Unix:  Use 'gcc latticeg.c -o latticeg'.  The\n");
print2("entire program is contained in the single file latticeg.c .\n");
print2("\n");
print2("To run:  Type, at the Unix or DOS prompt, \'latticeg <hyp> \n");
print2("<hyp> ... <conclusion>\' where there are zero or more hypotheses\n");
print2("followed by one conclusion.  Each argument may be enclosed in\n");
print2("double or (Unix) single quotes if there is ambiguity.  Example:\n");
print2("\'latticeg \"1<(x#y)\" \"x=y\"\' tests the orthomodular law.\n");
print2("\n");
print2("\n");
print2("\n");
print2("Each <hyp> and the <conclusion> must be a <wff> defined as follows:\n");
print2("\n");
linput(NULL,"Press Enter to continue, q to quit...",&a);
if (toupper(a[0]) == 'Q') goto returnPoint;
print2("\n");
print2("    <var> := a | b | c | d | e | f | g | h | j | k | l | m | n |\n");
print2("                 p | q | r | s | t | u | w | x | y | z\n");
print2("    <opr> := ^ | v | # | O | I | 2 | 3 | 4 | 5 \n");
print2(
"    <const> := 0 | 1           <uopr> := -\n");
print2(
"    <term> := <var> | <const> | <uopr> <term> | ( <term> <opr> <term> )\n");
print2(
"    <brel> := = | < | > | [    <ucon> := ~       <bcon> := & | V | } | :\n");
print2(
"    <wff> := ( <term> <brel> <term> ) | <ucon> <wff> | ( <wff> <bcon> <wff> )\n");
print2("\n");
print2("where a,b,c,... are variables (no i,o,v); 0,1 are constants; and\n");
print2("    - = negation (orthocomplement)\n");
print2("    ^ = conjunction (cap, meet, infimum)\n");
print2("    v = disjunction (cup, join, supremum)\n");
print2("    # = biimplication: ((x^y)v(-x^-y))\n");
print2("    O = ->0 = classical arrow: (-xvy)\n");
print2("    I = ->1 = Sasaki arrow: (-xv(x^y))\n");
print2("    2 = ->2 = Dishkant arrow: (-yI-x)\n");
print2("    3 = ->3 = Kalmbach arrow: (((-x^y)v(-x^-y))v(x^(-xvy)))\n");
print2("    4 = ->4 = non-tollens arrow: (-y3-x)\n");
print2("    5 = ->5 = relevance arrow: (((x^y)v(-x^y))v(-x^-y))\n");
print2(
"and = is equality, < is less-than-or-equal, > is g.e., [ is commutes:\n");
print2("    x<y is (xvy)=y; x>y is y<x; x[y is x=((x^y)v(x^-y)).\n");
print2(
"Metalogical connectives:  ~,&,V,},: are NOT,AND,OR,IMPLIES,EQUIVALENT.\n");
print2("The outermost parentheses of a <wff> are optional.\n");
linput(NULL,"Press Enter to continue, q to quit...",&a);
if (toupper(a[0]) == 'Q') goto returnPoint;
print2("\n");
print2("Predicate logic:\n");
print2("\n");
print2("The present implementation has the following limitations:\n");
print2("1. No hypotheses may be present if quantifiers are used.\n");
print2("   Use & (AND) and } (IMPLIES) in the conclusion instead.\n");
print2("2. The conclusion must be a <qwff> as defined below.\n");
print2("3. No two quantifiers may be followed by the same variable.\n");
print2("\n");
print2("We extend the wff syntax as follows:\n");
print2("    <qwff> := <wff> | @ <var> <qwff> | ] <var> <qwff>\n");
print2("where quantifier @ means \"for all\" and ] means \"exists\".\n");
print2("\n");
print2("Thus the conclusion must be in prenex normal form, i.e. with all\n");
print2("quantifiers at the beginning of the expression.\n");
print2("\n");
print2(
"Example:  'latticeg \"]x@y(z<(xvy))\"' means \"for all z (implicitly),\n");
print2("there is an x s.t. for all y, z is l.e. xvy.\"\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
linput(NULL,"Press Enter to continue, q to quit...",&a);
if (toupper(a[0]) == 'Q') goto returnPoint;
print2("\n");
print2("A FAILED result shows the failing assignments with a,b,c... as\n");
print2("lattice points and A,B,C,... as complemented lattice points.\n");
print2("Use the 'latticeg -p' program option to see lattice contents.\n");
print2("\n");
print2("For lattices with more than 48 lattice points, the failing\n");
print2("assignments beyond ...x,y,z are shown as &a,&b,&c,... with\n");
print2("complements &A,&B,&C,...\n");
print2("\n");
print2("The Greechie lattice atom numbers 1,2,3,...,23 correspond to nodes\n");
print2("a,b,c,... from the list:\n");
print2("\n");
print2("     a,b,c,d,e,f,g,h,j,k,l,m,n,p,q,r,s,t,u,w,x,y,z\n");
print2("\n");
print2("(with i,o,v omitted) and 24,25,26,... by:\n");
print2("\n");
print2("     &a,&b,&c,...\n");
print2("\n");
print2("with no omissions in the letter sequence.\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
linput(NULL,"Press Enter to continue, q to quit...",&a);
if (toupper(a[0]) == 'Q') goto returnPoint;
print2("\n");
print2("How to handle long equations:\n");
print2("You may specify strings to be replaced by continuation lines using\n");
print2("arguments of the form '$x' where x is a single character.  The\n");
print2("program will prompt with '$x> '; in response you should enter the\n");
print2("corresponding argument (quotes are optional).  Hint:  In Unix\n");
print2("enclose '$x' in single quotes to suppress shell interpretation.\n");
print2("\n");
print2("  latticeg $1 $2 \"x=z\"\n");
print2("  $1> x=y\n");
print2("  $2> y=z\n");
print2("\n");
print2("is the same as\n");
print2("\n");
print2("  latticeg \"x=y\" \"y=z\" \"x=z\"\n");
print2("\n");
print2("To break up very long formulas, use '$x$y$z...'; the above is also\n");
print2("the same as:\n");
print2("\n");
print2("  latticeg $1$2 $3 \"x=z\"\n");
print2("  $1> x=\n");
print2("  $2> y\n");
print2("  $3> y=z\n");
print2("\n");
linput(NULL,"Press Enter to continue, q to quit...",&a);
if (toupper(a[0]) == 'Q') goto returnPoint;
print2("\n");
print2("Using an input file (-i option):\n");
print2("\n");
print2("The input file should have one Greechie diagram per line, with each\n");
print2("atom name a character from the list:\n");
print2("  %s\n", left(ATOM_MAP, 54));
print2("  %s\n", right(ATOM_MAP, 55));
print2("where atom 1 = 1, atom 9 = 9, atom 10 = A, atom 35 = Z,\n");
print2("atom 36 = a, etc. with no gaps in numbering.  Blocks are separated\n");
print2("with a comma, and the last block ends with a period.  Each block\n");
print2("must have 3 or 4 atoms.  For example the diagram on p. 319 of\n");
print2("Kalmbach's _Orthomodular Lattices_ would be\n");
print2("  124,235,167,389.\n");
print2("Use the -p option to see the resulting lattice.\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
returnPoint:
let(&a, ""); /* Deallocate string */
return;
} /* printhelp() */


void testAllLattices(void)
{
  long latticeCase = 0;
  if (userArg > 0) latticeCase = userArg - 1;

  while (1) {
    latticeCase++;
    if (latticeCase > MAX_LATTICES    /* Built-in lattice limit */
        && latticeCase > fileLattices /* External file lattices */
        && userArg <= 0) break;                   /* Exhausted lattice cases */
    initLattice(latticeCase);
    if (!nodes && userArg > 0) {
      print2("?Error: program does not have lattice %ld\n", userArg);
      return;
    }
    if (!nodes) continue; /* Ignore gap in lattice numbering */
    if (test() == TRUE_CONST) {
      print2("Passed %s\n", latticeName);
      if (showGreechie) print2("%s passed: %s\n", latticeName, greechieStmt);
    } else {
      if (!showAllFailures) {
        /* (If showAllFaiure, failure was printed already by test()) */
        print2("FAILED %s at %s\n", latticeName, failingAssignment);
      }
      if (showGreechie) print2("%s failed: %s\n", latticeName, greechieStmt);
      if (userArg == 0) break; /* Default: stop on first failure */
    }
    if (userArg > 0) break; /* User wants to try only one lattice */
  }

  return;
} /* testAllLattices() */

/* This function is called when the user specifies the -c option.
   The hypotheses and conclusion are broken down into subformulas,
   then all subformula pairs are tested to see if they potentially
   commute (i.e. if assuming they commute does not fail the OM
   lattices).  The pairs that potentially commute are printed out. */
void findCommutingPairs(void)
{
  long latticeCase;
  long i, j, subformulas;
  unsigned char testResult;
  vstring str1 = "";
  vstring exprList = "";
  vstring subformList = "";
  vstring fromPol1 = "";
  vstring fromPol2 = "";
  char OAOnlyFlag;

  /* Make sure no metalogic is present */
  let(&str1, cat(LOGIC_BIN_CONNECTIVES, chr(NOT_OPER),
      QUANTIFIER_CONNECTIVES, NULL));
  for (j = 0; j < strlen(polConclusion); j++) {
    if (strchr(str1, polConclusion[j]) != NULL) {
      print2("?Error: Metalogic is not implemented for the -c option.\n");
      exit(0);
    }
  }
  for (i = 0; i < hypotheses; i++) {
    for (j = 0; j < strlen(polConclusion); j++) {
      if (strchr(str1, polHypList[i][j]) != NULL) {
        print2("?Error: Metalogic is not implemented for the -c option.\n");
        exit(0);
      }
    }
  }
  let(&str1, "");

  print2(
"This is a list of potentially commuting pairs i.e. that pass all OM\n");
  print2(
"lattices with the hypotheses assumed.  Additional pairs that potentially\n");
  print2(
"commute assuming OA are prefixed with '(OA)'.\n");

  /* Build a list with all expressions - abbreviated */
  for (i = 0; i < hypotheses; i++) {
    /* Strip leading '=' before adding to subformula list */
    let(&exprList, cat(exprList, right(polHypList[i], 2), NULL));
  }
  let(&exprList, cat(exprList, right(polConclusion, 2), NULL));

  /* Unabbreviate the hypotheses and conclusion */
  let(&str1, "");
  for (i = 0; i < hypotheses; i++) {
    str1 = unabbreviate(polHypList[i]);
    let(&polHypList[i], str1);
    let(&str1, ""); /* Deallocate from unabbreviate fn call */
  }
  str1 = unabbreviate(polConclusion);
  let(&polConclusion, str1);
  let(&str1, ""); /* Deallocate from unabbreviate fn call */

  /* Build a list with all expressions - unabbreviated */
  /* The unabbreviated expressions obtain additional commute pairs */
  for (i = 0; i < hypotheses; i++) {
    /* Strip leading '=' before adding to subformula list */
    let(&exprList, cat(exprList, right(polHypList[i], 2), NULL));
  }
  let(&exprList, cat(exprList, right(polConclusion, 2), NULL));

  /* Build a list with all subformulas */
  let(&subformList, "");
  subformList = subformulaList(exprList);

  subformulas = numEntries(subformList);
  for (i = 1; i <= subformulas - 1; i++) {
    for (j = i + 1; j <= subformulas; j++) {
      /* Set conclusion to "subformula pair commutes" */
      let(&polConclusion, cat("[", entry(i, subformList),
          entry(j, subformList), NULL));
      latticeCase = 0;
      OAOnlyFlag = 0;
      while (1) {
        latticeCase++;
        initLattice(latticeCase);
        if (latticeCase > MAX_LATTICES)
          bug(10);/* Exhausted lattice cases - means OMDoneFlag
                     was never set */
        if (!nodes) continue; /* Ignore gap in lattice numbering */
        testResult = test();
        if (testResult != TRUE_CONST) break;
           /* Failed - definitely doesn't commute */
        if (OADoneFlag) OAOnlyFlag = 1;
        if (OMDoneFlag) break; /* Did all OM cases */
      }
      if (testResult == TRUE_CONST ||
          OAOnlyFlag) {
        /* The pair potentially commutes - print it out */
        let(&fromPol1, "");
        fromPol1 = fromPolish(entry(i, subformList));
        /* Strip leading and trailing parenths */
        /* if (fromPol1[0] == '(')
          let(&fromPol1, seg(fromPol1, 2, strlen(fromPol1) - 1));*/
        let(&fromPol2, "");
        fromPol2 = fromPolish(entry(j, subformList));
        /* Strip leading and trailing parenths */
        /*if (fromPol2[0] == '(')
          let(&fromPol2, seg(fromPol2, 2, strlen(fromPol2) - 1));*/
        if (testResult != TRUE_CONST)
          let(&fromPol1, cat("(OA) ", fromPol1, NULL));
        if (strlen(fromPol1) + strlen(fromPol2) <= 74) {
          /* Print on one line */
          print2("%s and %s\n", fromPol1, fromPol2);
        } else {
          /* Print on two lines */
          print2("%s and\n  %s\n", fromPol1, fromPol2);
        }
      }
    } /* next j */
  } /* next i */

  let(&str1, "");
  let(&exprList, "");
  let(&subformList, "");
  let(&fromPol1, "");
  let(&fromPol2, "");
  return;
} /* findCommutingPairs() */



void init(void) /* Should be called only once!! */
{
  long i,j;
  vstring tmpStr = "";
  if (fp != NULL) {
    /* The user specified an input file */
    /* Count the lines - each line is a lattice */

    fileLattices = 0;
    while (linput(fp, NULL, &tmpStr)) {
      fileLattices++;
    }
    print2("The input file has %ld lattice(s).\n", fileLattices);
    /* Put the lines into an array of strings */
    rewind(fp);
    fileLineNum = 0;
  }

  /* Check that constants are consistent */
  if (MAX_BIN_CONNECTIVES != strlen(ALL_BIN_CONNECTIVES)) bug(1);
  if (MAX_VARS != strlen(VAR_LIST)) bug(2);
  if (instr(1, ALL_BIN_CONNECTIVES, chr(NEG_OPER))) bug(203);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(INF_OPER))) bug(204);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(SUP_OPER))) bug(205);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(ID_OPER))) bug(206);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(IMP0_OPER))) bug(207);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(IMP1_OPER))) bug(208);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(IMP2_OPER))) bug(209);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(IMP3_OPER))) bug(210);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(IMP4_OPER))) bug(211);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(IMP5_OPER))) bug(212);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(EQ_OPER))) bug(213);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(LE_OPER))) bug(214);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(GE_OPER))) bug(215);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(COM_OPER))) bug(216);
  if (instr(1, ALL_BIN_CONNECTIVES, chr(NOT_OPER))) bug(217);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(AND_OPER))) bug(218);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(OR_OPER))) bug(219);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(IMPL_OPER))) bug(220);
  if (!instr(1, ALL_BIN_CONNECTIVES, chr(BI_OPER))) bug(221);
  if (MAX_NODES <= FALSE_CONST || MAX_NODES <= TRUE_CONST
      || FALSE_CONST == 0 || TRUE_CONST == 0)
    /* Bad FALSE_CONST or TRUE_CONST will overflow binOpTable or
       cause end-of-string problems */
    bug(226);

  /* vstring initialization */
  for (i = 0; i < MAX_NODES; i++) {
    nodeList[i] = "";
    for (j = 0; j < MAX_NODES; j++) {
      sup[i][j] = "";
    }
  }
  for (i = 0; i < MAX_HYPS; i++) {
    hypList[i] = "";
    polHypList[i] = "";
  }

  /* Initialize speedup table for negative */
  for (i = 'A'; i <= 'z'; i++) {
    if (isupper(i)) {
      negMap[i] = tolower(i);
      /* Extended characters for >23 atoms */
      negMap[i + 128] = 128 + tolower(i);
    }
    if (islower(i)) {
      negMap[i] = toupper(i);
      /* Extended characters for >23 atoms */
      negMap[i + 128] = 128 + toupper(i);
    }
  }
  negMap['0'] = '1';
  negMap['1'] = '0';

  /* Initialize speedup table for operator */
  for (i = 1; i < 256; i++) {
    j = instr(1, ALL_BIN_CONNECTIVES, chr(i));
    if (j != 0) {
      opMap[i] = j - 1; /* Points to location of char in ALL_BIN_CONNECTIVES string */
    } else {
      opMap[i] = 127; /* Not a binary operation */
    }
    let(&tmpStr, ""); /* Deallocate temporary stack to prevent overflow */
  }
  let(&lletterlist, cat("abcdefghjklmnpqrstuwxyz", space(26), NULL));
  let(&uletterlist, cat("ABCDEFGHJKLMNPQRSTUWXYZ", space(26), NULL));
  for (i = 0; i < 26; i++) {
    lletterlist[i + 23] = (i + 'a' + 128) & 0xFF;
    uletterlist[i + 23] = (i + 'A' + 128) & 0xFF;
  }

  /* To debug extended characters:  This code makes the extended characters
     the first ones that are used, so the "&" notation can be debugged with
     smaller Greechie diagrams. */
  /*
  let(&lletterlist, cat(space(26), "abcdefghjklmnpqrstuwxyz", NULL));
  let(&uletterlist, cat(space(26), "ABCDEFGHJKLMNPQRSTUWXYZ", NULL));
  for (i = 0; i < 26; i++) {
    lletterlist[i ] = (i + 'a' + 128) & 0xFF;
    uletterlist[i ] = (i + 'A' + 128) & 0xFF;
  }
  */

} /* init */

void initLattice(long latticeNum)
{
  /* latticeNum:  1 = Boolean lattice, 2 = MO2 lattice, ... */
  long i ,j ,k, p, q, changed;
  vstring nname = "";
  vstring nbranch = "";
  vstring big = "";
  vstring small1 = "";
  vstring small2 = "";
  vstring oldsup = "";
  vstring tmpNodeList[MAX_NODES];
  vstring fileLine = "";

  nodes = 0; /* If 0 is returned, lattice is undefined */

  if (endArg && latticeNum > endArg) return;

  /* Initialize flags for special lattices */
  OADoneFlag = 0;
  OMDoneFlag = 0;

  /* Global variables used by greechie3() */
  glatticeNum = latticeNum;
  glatticeCount = 0;

  /* Get the Greechie diagram corresponding to glatticeNum */
  if (fp == NULL) {
    /* Use the ones hard-coded into this program */
    usersGreechieDiagrams();
  } else {
    /* We read from an input file */
    /* Trick greechie3() into thinking we've already done latticeNum cases */
    glatticeCount = latticeNum - 1;
    /* Call greechie3() as if this were a hard-coded user lattice */
    if (latticeNum <= fileLattices) {
      /* The program should never try to read "backwards" */
      if (fileLineNum >= latticeNum) bug(13);
      /* Skip any input lines before latticeNum in case of -n,-p options */
      while (fileLineNum < latticeNum - 1) {
        fileLineNum++;
        if (!linput(fp, NULL, &fileLine)) bug(14); /* Beyond EOF */
      }
      /* Read the line we're interested in */
      fileLineNum++;
      if (!linput(fp, NULL, &fileLine)) bug(15); /* Beyond EOF */
      greechie3("", fileLine);
    }
  }

  if (!nodes) return;
  if (nodes >= MAX_NODES) bug(8);

  /* Node name list */
  let(&nodeNames, "");
  for (i = 1; i <= nodes; i++) {
    let(&nodeNames, cat(nodeNames, left(nodeList[i], 1), NULL));
    /* Initialize speed-up lookup table */
    nodeNameMap[ascii_(left(nodeList[i], 1))] = i;
  }

  /* Fill out ordering table */
  for (i = 1; i <= nodes; i++) {
    tmpNodeList[i] = "";
    let(&(tmpNodeList[i]), nodeList[i]);
  }
  changed = 1;
  while (changed) {
    changed = 0;
    for (i = 1; i <= nodes; i++) {
      for (j = 1; j <= nodes; j++) {
        if (i != j) {
          let(&nname, left(tmpNodeList[j], 1));
          p = instr(2, tmpNodeList[i], nname);
          if (p != 0) {
            for (k = 2; k <= len(tmpNodeList[j]); k++) {
              let(&nbranch, mid(tmpNodeList[j], k, 1));
              q = instr(2, tmpNodeList[i], nbranch);
              if (q == 0) {
                changed = 1;
                let(&(tmpNodeList[i]), cat(tmpNodeList[i], nbranch, NULL));
              } /* end if */
            } /* next k */
          } /* end if */
        } /* end if */
      } /* next j */
    } /* next i */
  } /* next while */

  /* Build supremum (disjunction) table */
  for (i = 1; i <= nodes; i++) {
    for (j = 1; j <= nodes; j++) {
      let(&(sup[i][j]), "1");
    } /* next j */
  } /* next i */
  for (i = 1; i <= nodes; i++) {
    let(&big, left(tmpNodeList[i], 1));
    for (j = 1; j <= len(tmpNodeList[i]); j++) {
      for (k = 1; k <= len(tmpNodeList[i]); k++) {
        let(&small1, mid(tmpNodeList[i], j, 1));
        let(&small2, mid(tmpNodeList[i], k, 1));
        let(&oldsup, sup[instr(1, nodeNames, small1)]
            [instr(1, nodeNames, small2)]);
        /* oldsup > big */
        if (instr(2, tmpNodeList[instr(1, nodeNames, oldsup)], big)) {
          let(&(sup[instr(1, nodeNames, small1)]
              [instr(1, nodeNames, small2)]), big);
        } /* end if */
      } /* next k */
    } /* next j */
  } /* next i */

  /* Debug */
  /*
  for (i = 1; i <= nodes; i++) {
    for (j= 1; j <= nodes; j++) {
      print2("%s",sup[i][j]);
    }
    print2("\n");
  }
  */

  /* Build table with all binary operations for fast lookup */
  for (i = 0; i < strlen(ALL_BIN_CONNECTIVES); i++) {
    if (strchr(LOGIC_BIN_CONNECTIVES, ALL_BIN_CONNECTIVES[i]) == NULL) {
      for (j = 1; j <= nodes; j++) {
        for (k = 1; k <= nodes; k++) {
          binOpTable[i][j][k] = lookupBinOp(ALL_BIN_CONNECTIVES[i],
              (unsigned char)(nodeNames[j - 1]),
              (unsigned char)(nodeNames[k - 1]));
          /* Build even faster table with all binary operations for fast
             lookup */
          /* Eliminates subscript indirections */
          ultraFastBinOpTable[i][(unsigned char)(nodeNames[j - 1])]
              [(unsigned char)(nodeNames[k - 1])] =
              binOpTable[i][j][k];
        } /* next k */
      } /* next j */
    } else {
      /* Build tables for metalogical connectives */
      for (j = FALSE_CONST; j <= TRUE_CONST;
          j = j + (TRUE_CONST - FALSE_CONST)) {
        for (k = FALSE_CONST; k <= TRUE_CONST;
            k = k + (TRUE_CONST - FALSE_CONST)) {
          binOpTable[i][j][k] = lookupBinOp(ALL_BIN_CONNECTIVES[i], j, k);
          /* Build even faster table with all binary operations for fast
             lookup */
          /* Eliminates subscript indirections */
          /* For metalogical constants, the table is the same as
             binOpTable */
          ultraFastBinOpTable[i][j][k] =
              binOpTable[i][j][k];
        } /* next k */
      } /* next k */
    } /* next j */
  } /* next i */

  /* Deallocate vstrings */
  for (i = 1; i <= nodes; i++) {
    let(&(tmpNodeList[i]), "");
  }
  let(&nname, "");
  let(&nbranch, "");
  let(&big, "");
  let(&small1, "");
  let(&small2, "");
  let(&oldsup, "");
  let(&fileLine, "");

} /* initLattice() */


/* Converts a Greechie height 3 lattice, using the matrix notation in
   Kalmbach, "Orthomodular Lattices", p. 319, and populates the
   nodes variable and the nodeList array.  glattice is a comma-separated
   list of all nodes, with rows (assumed to be width 3) placed after
   each other. */
void greechie3(vstring name, vstring glattice) {
  long atoms, atom, atom2, msize, legs, i, j, k, m, n;
  vstring glattice1 = "";
  vstring str1 = "";
  vstring str2 = "";
  long block[MAX_BLOCKS + 1][MAX_BLOCK_SIZE + 1];
  long blockSize[MAX_BLOCKS + 1];
  long block4Offset[MAX_BLOCKS + 1];
  long blocks;

  glatticeCount++;  /* Global variable */
  if (glatticeCount != glatticeNum) return; /* Early exit if not the one */
  if (startArg && glatticeNum < startArg) return;

  /* In case of temp alloc of glattice; also trim leading, trailing spaces */
  /* Also trim CR in case of DOS file read in Unix */
  let(&glattice1, edit(glattice, 8 + 128 + 4));

  /* Assign the lattice name */
  if (name[0] == 0) {
    /* Blank argument - assign default name */
    let(&latticeName, cat("#", str(glatticeNum), NULL));
  } else {
    let(&latticeName, name); /* User's name */
  }

  if (showGreechie || removeLegs)
    print2("%s original: %s\n", latticeName, glattice1);

  /* Parse the input string describing the lattice */

  if (instr(1, glattice1, ".") == 0) {
    /* No period - assume old standard:  3-atom blocks */

    /* The following code allows free-formatted lists with spaces and/or
       commas as delimiters */
    i = 0;
    while (glattice1[i]) { /* Convert commas to spaces */
      if (glattice1[i] == ',') glattice1[i] = ' ';
      i++;
    }
    let(&glattice1, edit(glattice1, 8 + 16 + 128)); /* Reduce & trim spaces */
    i = 0;
    while (glattice1[i]) { /* Convert spaces to commas */
      if (glattice1[i] == ' ') glattice1[i] = ',';
      i++;
    }

    msize = numEntries(glattice1); /* Total matrix entries (nx3 matrix) */
    if ((msize / 3) * 3 != msize) {
      print2("%s: %s\n", latticeName, glattice1);
      print2(
"?Error: Number of Greechie matrix entries not factor of 3 (or missing '.')\n");
      exit(0);
    }
    blocks = msize / 3;
    atoms = 0;
    for (i = 1; i <= msize; i++) {
      j = val(entry(i, glattice1));
      let(&str1, ""); /* Purge temp alloc in entry() */
      if (j <= 0) {
        print2("%s: %s\n", latticeName, glattice1);
        print2(
"?Error: Greechie matrix nodes must be numbers > 1 (or missing '.')\n");
        exit(0);
      }
      if (j > atoms) atoms = j;
    }

    for (i = 1; i < msize; i = i + 3) {
      blockSize[(i + 2) / 3] = 3; /* Fixed block size of 3 */
      for (j = 0; j < 3; j++) {
        block[(i + 2) / 3][j + 1] = val(entry(i + j, glattice1));
      }
    }
  } else {
    /* glattice1 has period - assume new (Brendan) compact standard */
    if (glattice1[0] == '+') {
      print2("%s: %s\n", latticeName, glattice1);
      print2("?Error: '+' notation for large diagrams is not implemented\n");
      exit(0);
    }
    let(&glattice1, edit(glattice1, 2)); /* Remove spaces */
    n = strlen(glattice1);
    if (glattice1[n - 1] != '.') {
      print2("%s: %s\n", latticeName, glattice1);
      print2("?Error: Last character should be a period\n");
      exit(0);
    }
    if (instr(1, left(glattice1, n - 1), ".") != 0) {
      print2("%s: %s\n", latticeName, glattice1);
      print2("?Error: Period can only be last character\n");
      exit(0);
    }
    if (n == 1) {
      print2("%s: %s\n", latticeName, glattice1);
      print2("?Error: Diagram must have at least one block\n");
      exit(0);
    }
    atoms = 0;
    blocks = 1;
    blockSize[blocks] = 0;
    for (i = 0; i < n; i++) {
      if (glattice1[i] == ',' || glattice1[i] == '.') {
        /* End of block */
        if (blockSize[blocks] < MIN_BLOCK_SIZE) {
          print2("%s: %s\n", latticeName, glattice1);
          print2("?Error: Minimum block size is %ld\n", (long)MIN_BLOCK_SIZE);
          exit(0);
        }
        if (glattice1[i] == ',') {
          /* Start of new block */
          blocks++;
          if (blocks > MAX_BLOCKS) {
            print2("%s: %s\n", latticeName, glattice1);
            print2("?Error: Maximum blocks allowed is %ld\n",
                (long)MAX_BLOCKS);
            exit(0);
          }
          blockSize[blocks] = 0;
        }
        continue;
      }
      /* Get the atom number */
      j = instr(1, ATOM_MAP, chr(glattice1[i]));
      let(&str1, ""); /* Deallocate chr call */
      if (j == 0) {
        print2("%s: %s\n", latticeName, glattice1);
        print2("?Error: Illegal character '%c' in diagram\n", glattice1[i]);
        exit(0);
      }
      blockSize[blocks]++;
      if (blockSize[blocks] > MAX_BLOCK_SIZE) {
        print2("%s: %s\n", latticeName, glattice1);
        print2("?Error: Maximum block size is %ld\n", (long)MAX_BLOCK_SIZE);
        exit(0);
      }
      /* Assign the atom */
      block[blocks][blockSize[blocks]] = j;
      if (j > atoms) atoms = j; /* Maximum atom number */
    } /* next i */
  } /* glattice1 has a period (new compact standard) */

  for (i = 1; i <= blocks; i++) {
    for (j = 1; j <= blockSize[i] - 1; j++) {
      for (k = j + 1; k <= blockSize[i]; k++) {
        if (block[i][j] == block[i][k]) {
          print2("%s: %s\n", latticeName, glattice1);
          print2("?Error: Duplicate Greechie matrix node numbers in a block\n");
          exit(0);
        }
      }
    }
  }

  /* Special function:  see if diagram has 1 or more "legs" i.e. a block
     of rank 1. */
  /* This function also checks to see that 2-atom blocks are disconnected. */
  legs = 0;
  if (showGreechie || removeLegs) {
    for (i = 1; i <= blocks; i++) {
      n = 0; /* rank of block */
      for (j = 1; j <= blockSize[i]; j++) {
        /* Scan all other blocks for atom block[i][j] */
        for (k = 1; k <= blocks; k++) {
          if (k == i) continue;
          for (m = 1; m <= blockSize[k]; m++) {
            if (block[i][j] == block[k][m]) {
              if (blockSize[i] == 2 || blockSize[k] == 2) {
                print2("%s: %s\n", latticeName, glattice1);
                print2("?Error: Connected 2-atom blocks are not allowed\n");
                exit(0);
              }
              n++; /* Increase rank */
              goto exit_scan;
            }
          }
        }
       exit_scan:
        continue;
      }
      if (n == 1) {
        /* This is a foot */
        legs = 1; /* Means at least one foot */
        break;
      }
    } /* next i */
  }

  /* Count gaps in atom numbering */
  let(&str1, string(atoms, '0'));
  for (i = 1; i <= blocks; i++) {
    for (j = 1; j <= blockSize[i]; j++) {
      str1[block[i][j] - 1] = '1';
    }
  }
  n = 0; /* Number of gaps in atom numbering */
  for (i = 0; i < atoms; i++) {
    if (str1[i] == '0') n++;
  }
  /* Error if gaps in numbering */
  if (n > 0) {
    print2("%s: %s\n", latticeName, glattice1);
    print2("?Error: There are gaps in atom numbering\n");
    exit(0);
  }

  /* This is the global greechie3 diagram for use wherever
     we want to print it out.  It is in the new compact form; must
     be enhanced later for huge diagrams ("+" notation) */
  let(&greechieStmt, "");
  for (i = 1; i <= blocks; i++) {
    for (j = 1; j <= blockSize[i]; j++) {
      let(&greechieStmt, cat(greechieStmt, mid(ATOM_MAP, block[i][j], 1),
          NULL));
    }
    if (i < blocks) {
      let(&greechieStmt, cat(greechieStmt, ",", NULL));
    } else {
      let(&greechieStmt, cat(greechieStmt, ".", NULL));
    }
  }

  nodes = (atoms * 2) + 2;
  n = atoms;
  for (i = 1; i <= blocks; i++) {
    /* Add 6 nodes for each 4-atom block */
    block4Offset[i] = 0;
    if (blockSize[i] == 4) {
      block4Offset[i] = n;
      nodes = nodes + 6;
      n = n + 3;
    }
  }
  /* Enhance lattice name with atom/block/node count */
  let(&latticeName, cat(latticeName, " (", str(atoms), "/", str(blocks),
      "/", str(nodes), ")", NULL));

  if (removeLegs && legs > 0) {
    print2("%s >0 legs, skipped: %s\n", latticeName,
        greechieStmt);
    nodes = 0; /* Force lattice to be skipped */
    goto returnPoint;
  }

  if (nodes > 100) {
    print2("%s: %s\n", latticeName, greechieStmt);
    print2("?Error: Greechie diagram %s has more than 100 nodes\n",
        latticeName);
    nodes = 0;  /* Force lattice to be skipped */
    goto returnPoint;
  }

  /* Assign the unit node - all complemented atoms go below it */
  let(&(nodeList[1]), cat("1", left(uletterlist, atoms), NULL));

  /* Assign the zero node below all uncomplemented atoms */
  for (i = 1; i <= atoms; i++) {
    let(&(nodeList[i + 1]), cat(mid(lletterlist, i, 1), "0", NULL));
  }

  /* Assign the zero node */
  let(&(nodeList[atoms + 2]), "0");

  /* Create complemented atom list */
  for (i = 1; i <= atoms; i++) {
    let(&(nodeList[atoms + 2 + i]), mid(uletterlist, i, 1));
  }

  /* Create extra nodes for 4-atom blocks */
  for (i = 1; i <= blocks; i++) {
    if (blockSize[i] == 4) {
      for (j = 0; j < 3; j++) {
        let(&(nodeList[2 * block4Offset[i] + 2 + 2*j + 1]),
            mid(uletterlist, block4Offset[i] + j + 1, 1));
        let(&(nodeList[2 * block4Offset[i] + 2 + 2*j + 2]),
            mid(lletterlist, block4Offset[i] + j + 1, 1));
      }
      /* t */
      let(&(nodeList[2 * block4Offset[i] + 2 + 1]), cat(
          mid(lletterlist, block4Offset[i] + 1, 1),
          mid(lletterlist, block[i][1], 1),
          mid(lletterlist, block[i][2], 1), NULL));
      /* u */
      let(&(nodeList[2 * block4Offset[i] + 2 + 2]), cat(
          mid(lletterlist, block4Offset[i] + 2, 1),
          mid(lletterlist, block[i][1], 1),
          mid(lletterlist, block[i][3], 1), NULL));
      /* v */
      let(&(nodeList[2 * block4Offset[i] + 2 + 3]), cat(
          mid(lletterlist, block4Offset[i] + 3, 1),
          mid(lletterlist, block[i][1], 1),
          mid(lletterlist, block[i][4], 1), NULL));
      /* t' */
      let(&(nodeList[2 * block4Offset[i] + 2 + 4]), cat(
          mid(uletterlist, block4Offset[i] + 1, 1),
          mid(lletterlist, block[i][3], 1),
          mid(lletterlist, block[i][4], 1), NULL));
      /* u' */
      let(&(nodeList[2 * block4Offset[i] + 2 + 5]), cat(
          mid(uletterlist, block4Offset[i] + 2, 1),
          mid(lletterlist, block[i][2], 1),
          mid(lletterlist, block[i][4], 1), NULL));
      /* v' */
      let(&(nodeList[2 * block4Offset[i] + 2 + 6]), cat(
          mid(uletterlist, block4Offset[i] + 3, 1),
          mid(lletterlist, block[i][2], 1),
          mid(lletterlist, block[i][3], 1), NULL));
    }
  }

  /* Process Boolean blocks */
  for (i = 1; i <= blocks; i++) {
    if (blockSize[i] == 3) {
      for (j = 1; j <= 3; j++) {
        atom = block[i][j];
        for (k = 1; k <= 3; k++) {
          if (k == j) continue;
          atom2 = block[i][k];
          let(&(nodeList[atoms + atom + 2]), cat(nodeList[atoms + atom + 2],
              mid(lletterlist, atom2, 1), NULL));
        }
      }
      continue;
    }
    if (blockSize[i] == 4) {
      let(&(nodeList[atoms + block[i][1] + 2]),
          cat(nodeList[atoms + block[i][1] + 2],
          mid(uletterlist, block4Offset[i] + 1, 1),
          mid(uletterlist, block4Offset[i] + 2, 1),
          mid(uletterlist, block4Offset[i] + 3, 1), NULL));
      let(&(nodeList[atoms + block[i][2] + 2]),
          cat(nodeList[atoms + block[i][2] + 2],
          mid(uletterlist, block4Offset[i] + 1, 1),
          mid(lletterlist, block4Offset[i] + 2, 1),
          mid(lletterlist, block4Offset[i] + 3, 1), NULL));
      let(&(nodeList[atoms + block[i][3] + 2]),
          cat(nodeList[atoms + block[i][3] + 2],
          mid(lletterlist, block4Offset[i] + 1, 1),
          mid(uletterlist, block4Offset[i] + 2, 1),
          mid(lletterlist, block4Offset[i] + 3, 1), NULL));
      let(&(nodeList[atoms + block[i][4] + 2]),
          cat(nodeList[atoms + block[i][4] + 2],
          mid(lletterlist, block4Offset[i] + 1, 1),
          mid(lletterlist, block4Offset[i] + 2, 1),
          mid(uletterlist, block4Offset[i] + 3, 1), NULL));
      continue;
    }
    bug(9);
  }

 returnPoint:
  let(&str1, ""); /* Deallocate */
  let(&str2, ""); /* Deallocate */
  let(&glattice1, ""); /* Deallocate */

} /* greechie3 */



/* Returns TRUE_CONST if matrix test passed, FALSE_CONST if failed */
/* When FALSE_CONST is returned, the string failingAssignment has the
   assignment that violates the lattice */
unsigned char test(void)
{
  long i, j, k, conclusionLen, p, e, maxDepth, loopDepth;
  vstring trialConclusion = "";
  vstring trialHypList[MAX_HYPS];
  long loopVar[MAX_VARS];
  vstring fromPol = "";
  long hypLen[MAX_HYPS];
  long maxHypVar[MAX_HYPS];
  long maxConclusionVar;
  vstring tmpStr = "";
  vstring tmpStr2 = "";
  vstring varOrder = "";
  char atLeastOneFailure = 0; /* Used in conjunction with showAllFailures */
  long n;
  vstring tmpStr3 = "";
  vstring tmpStr4 = "";
  vstring tmpStr5 = "";

  /* Local string array initialization */
  for (i = 0; i < hypotheses; i++) {
    trialHypList[i] = "";
  }

  /* Initialization */
  let(&trialConclusion, polConclusion);
  conclusionLen = strlen(polConclusion);
  for (i = 0; i < hypotheses; i++) {
    let(&(trialHypList[i]), polHypList[i]);
    hypLen[i] = strlen(polHypList[i]);
  }

  /* Build a list of all variables found to determine best scanning order */
  /* This way, failures in early hypotheses with few variables can be used
     to skip evaluations of later hypotheses and conclusion for speed-up */
  /* (We assume the user places hypotheses in order of fewest variables
     or most likelihood of failure first) */
  let(&varOrder, "");
  for (i = 0; i < hypotheses; i++) {
    maxHypVar[i] = -1;
    for (p = 1; p <= hypLen[i]; p++) {
      let(&tmpStr, mid(polHypList[i], p, 1));
      j = instr(1, VAR_LIST, tmpStr);
      if (j != 0) { /* It's a variable */
        if (instr(1, varOrder, tmpStr) == 0) { /* but not in list */
          let(&varOrder, cat(varOrder, tmpStr, NULL)); /* then add to list */
        }
        /* Find out where it is in the list */
        j = instr(1, varOrder, tmpStr) - 1;
        if (j < 0) bug(3);
        /* Save the highest variable in the hypothesis for later speedup */
        if (j > maxHypVar[i]) maxHypVar[i] = j;
      }
    }
  }
  maxConclusionVar = -1;
  /* Since hypotheses aren't allowed with quantified formulas, we don't
     have to do this above.  Without hypotheses, the varOrder will
     be exactly in quantifierVars order - this is important for
     algorithm to work. */
  if (quantifierVars[0] != 0) {
    /* Predicate logic */
    k = strlen(quantifierVars);
  } else {
    /* Propositional logic only */
    k = conclusionLen;
  }
  for (p = 1; p <= k; p++) {
    if (quantifierVars[0] != 0) {
      let(&tmpStr, mid(quantifierVars, p, 1));
    } else {
      let(&tmpStr, mid(polConclusion, p, 1));
    }
    j = instr(1, VAR_LIST, tmpStr);
    if (j != 0) { /* It's a variable */
      if (instr(1, varOrder, tmpStr) == 0) { /* but not in list */
        let(&varOrder, cat(varOrder, tmpStr, NULL)); /* then add to list */
      }
      /* Find out where it is in the list */
      j = instr(1, varOrder, tmpStr) - 1;
      if (j < 0) bug(4);
      /* Save the highest variable in the conclusion for later speedup */
      if (j > maxConclusionVar) maxConclusionVar = j;
    }
  }
  maxDepth = strlen(varOrder) - 1;
  if (quantifierVars[0] != 0) {
    /* If quantified, there are no hypotheses so the below should never
       be the case. */
    if (maxConclusionVar != maxDepth) bug(270);
  }


  /* Perform all possible evaluations */
  loopDepth = 0;
  loopVar[loopDepth] = 0;
  while (1) {
    if (loopVar[loopDepth] < nodes) { /* Loop not exhausted */
      /* Evaluate any hypotheses with less than current # variables */
      e = TRUE_CONST;
      for (i = 0; i < hypotheses; i++) {
        if (maxHypVar[i] >= loopDepth) {
          /* Substitute lattice points for current variable in the
             hypothesis */
          for (p = 0; p < hypLen[i]; p++) {
            /* (If there are no variables at all, varOrder[0] will be 0
               i.e. end of string so we're still OK) */
            if ((polHypList[i])[p] == varOrder[loopDepth]) {
              (trialHypList[i])[p] = nodeNames[loopVar[loopDepth]];
            }
          }
        }
        if (maxHypVar[i] == loopDepth
            /* Special case: make sure we don't skip hypotheses
               with constants only, using the following OR condition: */
            || (maxHypVar[i] < 0 && loopDepth == 0) /* For hyp w/ no vars */
            ) {
          e = eval(trialHypList[i], hypLen[i]);
          if (e != TRUE_CONST) break; /* Out of i loop */
        }
      } /* Next i */
      /* Skip deeper iterations if hypothesis is false */
      if (e != TRUE_CONST) goto nextIter; /* A hypothesis failed;
                                             next iteration */
      if (maxConclusionVar >= loopDepth) {
        /* Substitute lattice points for current variable in the conclusion */
        for (p = 0; p < conclusionLen; p++) {
          if (polConclusion[p] == varOrder[loopDepth]) {
            trialConclusion[p] = nodeNames[loopVar[loopDepth]];
          }
        }
      }
      /* Make iteration deeper unless deepest level */
      if (loopDepth < maxDepth) {
        loopDepth++;
        loopVar[loopDepth] = 0; /* Init var for next deeper level */
        continue;
      } else { /* loopDepth == maxDepth (or special case: loopDepth = 0
          and maxDepth = -1 if both hyp's & conclusion are all constants) */
        /* We're at the maximum variable depth; evaluate conclusion */
        e = eval(trialConclusion, conclusionLen);
        if (e != TRUE_CONST) {
          if (e != FALSE_CONST) {print2("Bug - should be T or F\n"); exit(0);}

          if (quantifierVars[0] != 0) {
            /* This is a quantified formula */
            /* See if this is a real failure, or whether we have more trials
               left for any existentially quantified variable */
            /* Find the last 'exists', if any, that still has not exhausted
               its variable assignment.  If there is one, it's not a real
               failure yet, but we will increment its variable assignment
               and try again. */
            for (i = maxDepth; i >= 0; i--) {
              if (quantifierTypes[i] == EXISTS_OPER && loopVar[i] < nodes - 1) {
                /* This is not a real failure yet as we still have more
                   trials left for this existential variable */
                /* Increment the variable and point the loop depth to
                   it to try again */
                loopDepth = i;
                loopVar[loopDepth]++;
                goto continue_;
              }
            }
            /* If it gets here, we've exhausted any 'exists' so it is
               a real failure; let normal (propositional) code proceed */
          }


          /* Conclusion is false */
          atLeastOneFailure = 1;

          if (showVisits) {
            /* Show details of lattice failure evaluation (debugging mode) */
            printf("Details of lattice evaluation at failure:\n");
            visitFlag = 1;
            let(&visitList, ""); /* Initialize (redundant but just in case) */
            for (i = 0; i < hypotheses; i++) {
              print2("Hypothesis %ld\n", i + 1);

              /*e = eval(trialHypList[i]);*/ /* bad - doesn't show all nodes */
              /* Use unabbreviated version to see detailed visits */
              let(&tmpStr, "");
              tmpStr = unabbreviate(trialHypList[i]);
              e = eval(tmpStr, strlen(tmpStr));
              let(&tmpStr, "");

            } /* Next i */
            print2("Conclusion\n");

            /*e = eval(trialConclusion);*/ /* bad - doesn't show all nodes */
            /* Use unabbreviated version to see detailed visits */
            let(&tmpStr, "");
            tmpStr = unabbreviate(trialConclusion);
            e = eval(tmpStr, strlen(tmpStr));
            let(&tmpStr, "");

            /* Show nodes not visited (useful for identifying redundant
               lattice points) */
            let(&tmpStr2, "");
            for (i = 0; i < strlen(nodeNames); i++) {
              let(&tmpStr, chr(nodeNames[i]));
              if (!instr(1, visitList, tmpStr)) {
                let(&tmpStr2, cat(tmpStr2, " ", tmpStr, NULL));
              }
            }
            print2("Nodes not visited: %s\n", printableString(tmpStr2));
            /* For Greechie diagrams: show atoms not visited (useful for
               indentifying redundant atoms) */
            let(&tmpStr2, "");
/*??????**THIS DOES NOT WORK FOR 4-ATOM BLOCKS YET!! *****/
            /* Form comma-separated greechie matrix - tmpStr3 */
            let(&tmpStr3, edit(greechieStmt, 8 + 16 + 128));
            i = 0;
            while (tmpStr3[i]) { /* Convert spaces to commas */
              if (tmpStr3[i] == ' ') tmpStr3[i] = ',';
              i++;
            }
            /* Form binary greechie matrix - tmpStr4 */
            j = numEntries(tmpStr3);
            let(&tmpStr4, space(j));
            for (i = 0; i < j; i++) {
              tmpStr4[i] = val(entry(i + 1, tmpStr3));
            }
            /* Form atom visit flag list - tmpStr5 */
            let(&tmpStr5, string(j, 1));
            /* Scan node visits to get Greechie diagram info */
            for (i = 0; i < strlen(lletterlist); i++) {
              let(&tmpStr, chr(lletterlist[i]));
              if (!instr(1, nodeNames, tmpStr)) break;
              if (!instr(1, visitList, tmpStr) &&
                  !instr(1, visitList, chr(uletterlist[i]))) {
                let(&tmpStr2, cat(tmpStr2, " ", str(i + 1), NULL));
                /* Flag unvisited atoms in Greechie matrix */
                for (k = 0; k < j; k++) {
                  if (tmpStr4[k] == i + 1) {
                    tmpStr5[k] = 2;
                  }
                }
              }
            }
            print2("Greechie lattice atoms not visited: %s\n", tmpStr2);
            let(&tmpStr2, "");
            for (i = 0; i < j; i = i + 3) {
              n = 0;
              for (k = i; k < i + 3; k++) {
                if (tmpStr5[k] == 2) n++;
              }
              if (n > 1) {
                if (tmpStr2[0] != 0) {
                  let(&tmpStr2, cat(tmpStr2, " ", NULL));
                }
                for (k = i; k < i + 3; k++) {
                  let(&tmpStr2, cat(tmpStr2, " ", entry(k + 1, tmpStr3),
                      NULL));
                }
              }
            }
/*????*****THIS DOES NOT WORK WITH NEW BRENDAN NOTATION!!! ********/
            print2("Greechie blocks with 0 or 1 visits:%s\n", tmpStr2);

            visitFlag = 0;
            let(&visitList, ""); /* Deallocate */
          } /* if (showVisits) */

          /* Construct failing assignment info string */
          let(&failingAssignment, ""); /* Clear any earlier failure */
          for (i = 0; i < hypotheses; i++) {
            if (i > 0)
              let(&failingAssignment, cat(failingAssignment, " & ", NULL));
            let(&fromPol, "");
            fromPol = fromPolish(trialHypList[i]);
            /* Strip leading and trailing parenths */
            if (fromPol[0] == '(')
              let(&fromPol, seg(fromPol, 2, strlen(fromPol) - 1));
            let(&failingAssignment, cat(failingAssignment, fromPol,
                NULL));
          }
          if (hypotheses > 0)
            let(&failingAssignment, cat(failingAssignment, " => ", NULL));
          let(&fromPol, "");
          fromPol = fromPolish(trialConclusion);
          /* Strip leading and trailing parenths */
          if (fromPol[0] == '(')
            let(&fromPol, seg(fromPol, 2, strlen(fromPol) - 1));
          let(&failingAssignment, cat(failingAssignment, fromPol,
              NULL));
          /* Convert unprintable characters */
          let(&failingAssignment, printableString(failingAssignment));

          /* Normal (non-debugging) operation:  exit on first failure */
          if (!showAllFailures) goto done; /* Failed */

          /* Debugging mode: print the failure and try for another one */
          print2("FAILED %s at %s\n", latticeName, failingAssignment);

        } else { /* e == TRUE_CONST) */

          if (quantifierVars[0] != 0) {
            /* This is a quantified formula */
            /* See if this is a real pass, or whether we have more trials
               left for any universally quantified variable */
            /* Find the last 'forall', if any, that still has not exhausted
               its variable assignment.  If there is one, it's not a real
               pass yet, but we will increment its variable assignment
               and try again. */
            for (i = maxDepth; i >= 0; i--) {
              if (quantifierTypes[i] == FORALL_OPER && loopVar[i] < nodes - 1) {
                /* This is not a real pass yet as we still have more
                   trials left for this universal variable */
                /* Increment the variable and point the loop depth to
                   it to try again */
                loopDepth = i;
                loopVar[loopDepth]++;
                goto continue_;
              }
            }
            /* If it gets here, we've exhausted any 'for all' so it is
               a real pass */
            goto done;
          }

        }
      } /* end of loopDepth >= maxDepth case */
    } else { /* loopVar[loopDepth] >= nodes) - loop exhausted */
      /* End of inner loop; pop up one level */
      loopDepth--;
      if (loopDepth < 0) break; /* Completely done - out of while loop */
    } /* if (loopVar[loopDepth] < nodes) */
    /* Increment the loop variable */
   nextIter:
    loopVar[loopDepth]++;
   continue_:
    continue;
  } /* while (1) */
 done:
  if (atLeastOneFailure) {
    e = FALSE_CONST;
  } else {
    e = TRUE_CONST;
  }
  let(&trialConclusion, ""); /* Deallocate string */
  let(&fromPol, ""); /* Deallocate string */
  let(&tmpStr, ""); /* Deallocate string */
  let(&tmpStr2, ""); /* Deallocate string */
  let(&varOrder, ""); /* Deallocate string */
  for (i = 0; i < hypotheses; i++) {
    let(&(trialHypList[i]), "");  /* Deallocate string */
  }
  let(&tmpStr3, ""); /* Deallocate string */
  let(&tmpStr4, ""); /* Deallocate string */
  let(&tmpStr5, ""); /* Deallocate string */
  return e;
} /* test() */


/* Converts all operators to v,^,- */
/* The caller must deallocate the result */
vstring unabbreviate(vstring eqn)
{
  long i;
  long sblen1, sblen2;
  vstring subEqn1 = "";
  vstring subEqn2 = "";
  vstring newSubEqn = "";
  vstring subform = "";
  vstring eqn1 = "";
  let(&eqn1, eqn); /* In case temp allocation passed in */
  /* Note: don't precompute strlen(eqn1), since eqn1 grows */
  for (i = 1; i <= strlen(eqn1); i++) {
    if (opMap[(unsigned char)(eqn1[i - 1])] < 127) {
      /* It's a binary operation - unabbreviate it */
      subEqn1 = subformula(right(eqn1, i + 1));
      sblen1 = strlen(subEqn1);
      subEqn2 = subformula(right(eqn1, i + 1 + sblen1));
      sblen2 = strlen(subEqn2);
      switch ((unsigned char)(eqn1[i - 1])) {
        /* I'm not sure whether it's helpful to do < & [ */
        case '>': /* greater than or equal:  x>y is y<x */
        case '<': /* less than or equal:  x<y is (xvy)=y */
          /*
          let(&newSubEqn, cat("=v", subEqn1, subEqn2, subEqn2, NULL));
          break;
          */
        case '[': /* commutes:  x[y is x=((x^y)v(x^-y)) */
          /*
          let(&newSubEqn, cat("=", subEqn1, "v^", subEqn1, subEqn2, "^",
              subEqn1, "-", subEqn2, NULL));
          break;
          */
        /* Primitive and metalogical connectives are not expanded */
        case 'v':
        case '^':
        /*case '-':*/ /* Unary - shouldn't happen; let bugtrap catch it */
        case '=':
        /*case '~':*/ /* Unary - shouldn't happen; let bugtrap catch it */
        case '&': /* Metalogical AND */
        case 'V': /* Metalogical OR */
        case '}': /* Metalogical implies */
        case ':': /* Metalogical biconditional */
          /* This assignment does not expand the binary operation */
          let(&newSubEqn, cat(mid(eqn1, i, 1), subEqn1, subEqn2, NULL));
          break;
        case '#': /* # = biimplication: ((x^y)v(-x^-y))*/
          let(&newSubEqn, cat("v^", subEqn1, subEqn2, "^-",
              subEqn1, "-", subEqn2, NULL));
          break;
        case 'O': /* O = ->0 = classical arrow: (-xvy) */
          let(&newSubEqn, cat("v-", subEqn1, subEqn2, NULL));
          break;
        case 'I': /* I = ->1 = Sasaki arrow: (-xv(x^y)) */
          let(&newSubEqn, cat("v-", subEqn1, "^", subEqn1, subEqn2, NULL));
          break;
        case '2': /* 2 = ->2 = Dishkant arrow: (yv(-x^-y)) */
          let(&newSubEqn, cat("v", subEqn2, "^-", subEqn1, "-", subEqn2, NULL));
          break;
        case '3': /* 3 = ->3 = Kalmbach arrow: (((-x^y)v(-x^-y))v(x^(-xvy))) */
          let(&newSubEqn, cat("vv^-", subEqn1, subEqn2, "^-", subEqn1, "-",
              subEqn2, "^", subEqn1, "v-", subEqn1, subEqn2, NULL));
          break;
        case '4': /* 4 = ->4 = non-tollens arrow: (((x^y)v(-x^y))v((-xvy)^-y))*/
          let(&newSubEqn, cat("vv^", subEqn1, subEqn2, "^-", subEqn1, subEqn2,
              "^v-", subEqn1, subEqn2, "-", subEqn2, NULL));
          break;
        case '5': /* 5 = ->5 = relevance arrow: (((x^y)v(-x^y))v(-x^-y)) */
          let(&newSubEqn, cat("vv^", subEqn1, subEqn2, "^-", subEqn1, subEqn2,
              "^-", subEqn1, "-", subEqn2, NULL));
          break;
        default:
          bug(5);
      } /* end switch */
      let(&eqn1, cat(left(eqn1, i - 1), newSubEqn, right(eqn1, i + sblen1
          + sblen2 + 1), NULL));
      let(&subEqn1, ""); /* Deallocate from subformula() call */
      let(&subEqn2, ""); /* Deallocate from subformula() call */
    } /* if (opMap[eqn1[i]] < 127) */
  } /* next i */
  let(&newSubEqn, ""); /* Deallocate */
  let(&subform, ""); /* Deallocate */
  return eqn1;
} /* unabbreviate */

/* Returns a comma-separated list of all subformulas in an
   equation */
/* The caller must deallocate the result */
vstring subformulaList(vstring eqn)
{
  long i, j, k, p;
  long length;
  vstring subform = "";
  vstring list = "";
  vstring eqn1 = "";
  vstring e = "";
  vstring unabbrList = "";
  let(&eqn1, eqn); /* In case temp allocation passed in */
  length = strlen(eqn1);
  for (i = 1; i <= length; i++) {
    let(&subform, ""); subform = subformula(right(eqn1, i));
    /* Ignore negated subformulas to make list cleaner */
    while (subform[0] == NEG_OPER || subform[0] == NOT_OPER)
      let(&subform, right(subform, 2));
    /* If the subformula isn't in the list, add it */
    if (lookup(subform, list) == 0
        /* Also look at unabbreviated list, so we won't have abbreviated
           and unabbreviated duplicates (this assume abbreviated form of
           expression appears first in eqn1 - see findCommutingPairs()
           function) */
        && lookup(subform, unabbrList) == 0
        ) {
      if (list[0] == 0) {
        let(&list, subform);
      } else {
        /*let(&list, cat(list, ",", subform, NULL));*/
        /* Figure out where it should go - shortest first */
        j = numEntries(list);
        for (k = 1; k <= j; k++) {
          let(&e, entry(k, list));
          if (strlen(e) > strlen(subform) ||
              (strlen(e) == strlen(subform) && strcmp(e, subform) > 0)) {
            /* A longer one found - put it before */
            p = entryPosition(k, list);
            let(&list, cat(left(list, p - 1), subform, ",", right(list, p),
                NULL));
            break;
          }
        } /* next k */
        if (k > j) {
          /* It's the biggest - put at end of list */
          let(&list, cat(list, ",", subform, NULL));
        }
      }

      /* Add to unabbreviated list */
      let(&e, "");
      e = unabbreviate(subform);
      if (unabbrList[0] == 0) {
        let(&unabbrList, e);
      } else {
        let(&unabbrList, cat(unabbrList, ",", e, NULL));
      }

    }
  }
  let(&subform, "");
  let(&eqn1, "");
  let(&e, "");
  return list;
} /* subformulaList() */



/* Returns the char value of the nodeName character that the
   trial equation evaluates to */
unsigned char eval(vstring trialEqn, long eqnLen)
{
  unsigned char e;
  vstring subEqn1; /* Don't initialize here (for speedup) */
  vstring subEqn2; /* Don't initialize here (for speedup) */

  /* Speedup - use the fast version */
  if (1) {
    /* Put "visitFlag || 1" for verifying fastEval vs. ultraFastEval
       for debugging purposes */
    if (visitFlag) {
      /* This evaluation prints out the visits */
      e = fastEval(trialEqn, 0);
      /* This bugcheck makes sure that the two methods give the same
         result */
      if (e != ultraFastEval(trialEqn, eqnLen)) bug(111);
      return e;
    } else {
      return ultraFastEval(trialEqn, eqnLen);
    }
  }

  /* The code below is for the slower original algorithm and never
     gets run unless the speedup block above is commented out -
     but it should give the same results for debugging, except
     the "visitFlag" is not implemented */
  /* For speedup - initialize vstrings only if old algorithm is being run */
  subEqn1 = "";
  subEqn2 = "";
  if ((unsigned char)(trialEqn[0]) == NEG_OPER) {
    let(&subEqn1, right(trialEqn, 2));
    /*
    e = lookupCompl(eval(subEqn1));
    */
    /* Speedup */
    e = negMap[eval(subEqn1, strlen(subEqn1))];
  } else {
    if ((unsigned char)(trialEqn[0]) == NOT_OPER) {
      /* Classical metalogical NOT */
      let(&subEqn1, right(trialEqn, 2));
      if (eval(subEqn1, strlen(subEqn1)) == TRUE_CONST) {
        e = FALSE_CONST;
      } else {
        if (eval(subEqn1, strlen(subEqn1)) == FALSE_CONST) {
          e = TRUE_CONST;
        } else {
          bug(200);
        }
      }
    } else {
      /*
      if (instr(1, ALL_BIN_CONNECTIVES, chr(trialEqn[0])) != 0) {
      */
      /* Speedup */
      if (opMap[(unsigned char)(trialEqn[0])] < 127) {
        subEqn1 = subformula(right(trialEqn, 2));
        subEqn2 = subformula(right(trialEqn, strlen(subEqn1) + 2));
        /*
        e = lookupBinOp(trialEqn[0], eval(subEqn1), eval(subEqn2));
        */
        /* Speedup */
        if (strchr(LOGIC_BIN_CONNECTIVES, trialEqn[0]) == NULL) {
          e = binOpTable[opMap[(unsigned char)(trialEqn[0])]]
             [nodeNameMap[eval(subEqn1, strlen(subEqn1))]]
             [nodeNameMap[eval(subEqn2, strlen(subEqn2))]];
        } else {
          e = binOpTable[opMap[(unsigned char)(trialEqn[0])]]
             [eval(subEqn1, strlen(subEqn1))]
             [eval(subEqn2, strlen(subEqn2))];
        }
      } else {
        /* Must be a node */
        e = (unsigned char)(trialEqn[0]);
      }
    }
  }
  let(&subEqn1, "");  /* Deallocate vstring */
  let(&subEqn2, "");  /* Deallocate vstring */
  return e;

} /* eval */


/* Returns the char value of the nodeName character that the
   trial equation evaluates to */
/* startChar = 0 is 1st char in string, 1 is 2nd, etc. */
unsigned char fastEval(vstring trialEqn, long startChar)
{
  unsigned char e;
  long i;
  vstring s1; /* Don't initialize for speedup */
  vstring s2; /* Don't initialize for speedup */

  if ((unsigned char)(trialEqn[startChar]) == NEG_OPER) {
    e = negMap[fastEval(trialEqn, startChar + 1)];
  } else {
    if ((unsigned char)(trialEqn[startChar]) == NOT_OPER) {
      e = fastEval(trialEqn, startChar + 1);
      if (e == TRUE_CONST) {
        e = FALSE_CONST;
      } else {
        if (e == FALSE_CONST) {
          e = TRUE_CONST;
        } else {
          bug(252);
        }
      }
    } else {
      if (opMap[(unsigned char)(trialEqn[startChar])] < 127) {
        i = subformulaLen(trialEqn, startChar + 1);
        if (strchr(LOGIC_BIN_CONNECTIVES, trialEqn[startChar]) == NULL) {
          e = binOpTable[opMap[(unsigned char)(trialEqn[startChar])]]
             [nodeNameMap[fastEval(trialEqn, startChar + 1)]]
             [nodeNameMap[fastEval(trialEqn, startChar + i + 1)]];
        } else {
          e = binOpTable[opMap[(unsigned char)(trialEqn[startChar])]]
             [fastEval(trialEqn, startChar + 1)]
             [fastEval(trialEqn, startChar + i + 1)];
        }
      } else {
        /* Must be a node */
        e = (unsigned char)(trialEqn[startChar]);
      }
    }
    if (visitFlag) {
      /* Show user details of failing lattice visit */
      s1 = subformula(right(trialEqn, startChar + 1));
      s2 = fromPolish(s1);
      if (strchr(LOGIC_AND_RELATION_CONNECTIVES, trialEqn[startChar]) != NULL) {
        /* Ignore final result (will be FALSE_CONST, which means "false"
           or TRUE_CONST for "true" in the case of hypothesis) */
        if (e != FALSE_CONST && e != TRUE_CONST) printf("%ld\n",(long)e);
        if (e != FALSE_CONST && e != TRUE_CONST) bug(201);
        if (e == FALSE_CONST) print2("(false) = %s\n", printableString(s2));
        if (e == TRUE_CONST) print2("(true) = %s\n", printableString(s2));
      } else {
        print2("%s = %s\n", printableString(chr(e)), printableString(s2));
        let(&visitList, cat(visitList, chr(e), NULL));
      }
      let(&s1, ""); /* Deallocate */
      let(&s2, ""); /* Deallocate */
    }
  }
  return e;
}

/* Returns the char value of the nodeName character that the
   trial equation evaluates to */
/* This is faster than "fastEval" because it eliminates a
   recursive call */
unsigned char ultraFastEval(vstring trialEqn, long eqnLen)
{
  unsigned char stack[MAX_STACK2];
  unsigned char e, e1;
  long i, stackPtr;

  /* For debugging - print the equation in Polish notation */
  /* print2("%s\n", trialEqn); */

  stackPtr = -1;

  for (i = eqnLen - 1; i >= 0; i--) {
    e = (unsigned char)(trialEqn[i]);

    if (e == NEG_OPER) {
      /* Comment out bugcheck for speedup */
      /* Should never happen because of input expression syntax check */
      /* if (stackPtr < 0) bug(107); */
      stack[stackPtr] = negMap[stack[stackPtr]];
    } else {
      if (e == NOT_OPER) {
        if (stack[stackPtr] != TRUE_CONST
            && stack[stackPtr] != FALSE_CONST)
          bug(251);
        if (stack[stackPtr] == TRUE_CONST) {
          stack[stackPtr] = FALSE_CONST;
        } else {
          stack[stackPtr] = TRUE_CONST;
        }
      } else {
        e1 = opMap[e];
        if (e1 < 127) {
          stackPtr--;
          /* Comment out bugcheck for speedup */
          /* Should never happen because of input expression syntax check */
          /* if (stackPtr < 0) bug(108); */
          /*
          stack[stackPtr] = binOpTable[opMap[e]]
             [nodeNameMap[stack[stackPtr + 1]]]
             [nodeNameMap[stack[stackPtr]]];
          */
          /* Speedup:  eliminate indirect subscript */
          stack[stackPtr] = ultraFastBinOpTable[e1]
             [stack[stackPtr + 1]]
             [stack[stackPtr]];
        } else {
          /* Must be a node */
          stackPtr++;
          /* Could comment out bugcheck for ~5% speedup - but would be dangerous
             if input expression too long - future: we could pre-check this */
          if (stackPtr >= MAX_STACK2) bug(109);
          stack[stackPtr] = e;
        }
      }
    }
  }
  /* Comment out bugcheck for speedup */
  /* Should never happen because of input expression syntax check */
  /* if (stackPtr) bug(110); */
  return (unsigned char)(stack[0]);
} /* ultraFastEval */


/* Returns the shortest subformula from beginning of equation */
/* The caller must deallocate the result */
vstring subformula(vstring eqn)
{
  vstring result = "";
  long i, p;
  let(&result, eqn); /* In case temp allocation passed in */
  i = 0;
  p = 1;
  while (p > 0) {
    /*
    if (instr(1, ALL_BIN_CONNECTIVES, chr(result[i])) != 0) {
    */
    /* Speedup */
    if (opMap[(unsigned char)(result[i])] < 127) {
      p++;
    } else {
      if ((unsigned char)(result[i]) != NEG_OPER
          && (unsigned char)(result[i]) != NOT_OPER)
        p--;  /* It is a node name or constant */
    }
    i++;
  }
  let(&result, left(result, i));
  return result;
}


/* Returns the length of the shortest subformula from startChar */
long subformulaLen(vstring eqn, long startChar)
{
  long i, p;
  i = 0;
  p = 1;
  while (p > 0) {
    if (opMap[(unsigned char)(eqn[startChar + i])] < 127) {
      p++;
    } else {
      if ((unsigned char)(eqn[startChar + i]) != NEG_OPER
          && (unsigned char)(eqn[startChar + i]) != NOT_OPER)
        p--; /* It is a node name or constant */
    }
    i++;
  }
  return i;
}


unsigned char lookupCompl(unsigned char arg) {
  /* abc... = node names; ABC.. = complemented node names */
  /*
  if (arg == '0') return '1';
  if (arg == '1') return '0';
  if (isupper(arg)) return tolower(arg);
  if (islower(arg)) return toupper(arg);
  bug(6);
  return 0;
  */
  /* Speedup */
  return negMap[arg];
}

unsigned char lookupNot(unsigned char arg) {
  /* Complement TRUE_CONST or FALSE_CONST */
  if (arg == TRUE_CONST) return FALSE_CONST;
  if (arg == FALSE_CONST) return TRUE_CONST;
  bug(225);
  return 0;
}

/* The input/output of lookupBinOp are actual nodes names. */
/* Except, for the case of metalogical true and false, the inputs must
   be the (meaningless) node names TRUE_CONST
   and FALSE_CONST.  The outputs are also these
   meaningless node names.  The nodeNames and nodeNameMap should not
   be used for these. */
unsigned char lookupBinOp(unsigned char operation, unsigned char arg1,
    unsigned char arg2) {
    /*
    ^ = conjunction
    v = disjunction
    # = biimplication: ((x^y)v(-x^-y))
    O = ->0 = classical arrow: (-xvy)
    I = ->1 = Sasaki arrow: (-xv(x^y))
    2 = ->2 = Dishkant arrow: (-yI-x)
    3 = ->3 = Kalmbach arrow: (((-x^y)v(-x^-y))v(x^(-xvy)))
    4 = ->4 = non-tollens arrow: (-y3-x)
    5 = ->5 = relevance arrow: (((x^y)v(-x^y))v(-x^-y))

    = = equals predicate - returns T or F
    < = less-than-or-equal predicate:  ((xvy)=y) - returns T or F
    [ = commutes predicate:  (x=((x^y)v(x^-y))) - returns T or F
    */
  unsigned char result;
  switch (operation) {
    case 'v':
      /* Speedup:  eliminate instr() w/ char map */
      /* the sup[][] table indexes are 0,1,... from node name
         via nodeNameMap.   The value of sup[][] is an actual
         node name. */
      if (1) return (sup[nodeNameMap[arg1]][nodeNameMap[arg2]])[0];
      break;
    case '^':
      result = lookupCompl(lookupBinOp('v', lookupCompl(arg1),
          lookupCompl(arg2)));
      break;
    case '#':
      result = lookupBinOp('v',
                lookupBinOp('^', arg1, arg2),
                lookupCompl(lookupBinOp('v', arg1, arg2)));
      break;
    case 'O':
      result = lookupBinOp('v', lookupCompl(arg1), arg2);
      break;
    case 'I':
      result = lookupBinOp('v', lookupCompl(arg1),
                lookupBinOp('^', arg1, arg2));
      break;
    case '2':
      result = lookupBinOp('I', lookupCompl(arg2), lookupCompl(arg1));
      break;
    case '3':
      result = lookupBinOp('v',
               lookupBinOp('v',
                 lookupBinOp('^', lookupCompl(arg1), arg2),
                 lookupBinOp('^', lookupCompl(arg1), lookupCompl(arg2))),
               lookupBinOp('^', arg1,
                 lookupBinOp('v', lookupCompl(arg1), arg2)));
      break;
    case '4':
      result = lookupBinOp('3', lookupCompl(arg2), lookupCompl(arg1));
      break;
    case '5':
      result = lookupBinOp('v',
               lookupBinOp('v',
                 lookupBinOp('^', arg1, arg2),
                 lookupBinOp('^', lookupCompl(arg1), arg2)),
               lookupBinOp('^', lookupCompl(arg1), lookupCompl(arg2)));
      break;

    case '=':
      if (arg1 == arg2) {
        result = TRUE_CONST;
      } else {
        result = FALSE_CONST;
      }
      break;
    case '<': /* Less than or equal to */
      result = lookupBinOp('=', lookupBinOp('v', arg1, arg2), arg2);
      break;
    case '>': /* Greater than or equal to */
      result = lookupBinOp('<', arg2, arg1);
      break;
    case '[': /* Commutes */
      result = lookupBinOp('=', arg1, lookupBinOp('v',
                 lookupBinOp('^', arg1, arg2),
                 lookupBinOp('^', arg1, lookupCompl(arg2))));
      break;
    case OR_OPER:
      /* Classical metalogic */
      if ((arg1 != TRUE_CONST
            && arg1 != FALSE_CONST)
          || (arg2 != TRUE_CONST
            && arg2 != FALSE_CONST))
        bug(231);
      if (arg1 == TRUE_CONST
          || arg2 == TRUE_CONST) {
        result = TRUE_CONST;
      } else {
        result = FALSE_CONST;
      }
      break;
    case AND_OPER:
      /* Classical metalogic */
      if ((arg1 != TRUE_CONST
            && arg1 != FALSE_CONST)
          || (arg2 != TRUE_CONST
            && arg2 != FALSE_CONST))
        bug(230);
      result = lookupNot(lookupBinOp(OR_OPER, lookupNot(arg1),
          lookupNot(arg2)));
      break;
    case IMPL_OPER:
      /* Classical metalogic */
      if ((arg1 != TRUE_CONST
            && arg1 != FALSE_CONST)
          || (arg2 != TRUE_CONST
            && arg2 != FALSE_CONST))
        bug(232);
      result = lookupBinOp(OR_OPER, lookupNot(arg1), arg2);
      break;
    case BI_OPER:
      /* Classical metalogic */
      if ((arg1 != TRUE_CONST
            && arg1 != FALSE_CONST)
          || (arg2 != TRUE_CONST
            && arg2 != FALSE_CONST))
        bug(233);
      result = lookupBinOp(OR_OPER,
                lookupBinOp(AND_OPER, arg1, arg2),
                lookupNot(lookupBinOp(OR_OPER, arg1, arg2)));
      break;
    default:
      result = 0;
      bug(7);
  } /* switch (operation) */

  /* Uncomment for debugging */
  /*
  print2("%s %s %s = %s\n", chr(arg1), chr(operation), chr(arg2),
      chr(result));
  */
  return result;
}


/* ***Note:  The caller must deallocate returned string! */
/* Convert normal to Polish notation */
/* recursiveCall should be 0 when calling from outside, and nonzero when
   toPolish calls itself recursively */
/* Since this error checks user's input arguments, error exits program with
   message rather than return an error value */
vstring toPolish(vstring equation)
{
 /* This function converts a theorem in parentheses notation to
    one in Polish notation */

  long i, j, k, eqnLen, subEqn1Len, subEqn2Len;
  char operation;
  vstring polEqn = "";
  vstring equation1 = "";
  vstring subEqn1 = "";
  vstring subEqn2 = "";
  vstring polSubEqn1 = "";
  vstring polSubEqn2 = "";
  vstring quantifiers = "";
  static long recursiveCall = 0;

  /* Make any tempAlloc of equation permanent */
  let(&equation1, equation);

  eqnLen = strlen(equation1);

  if (!recursiveCall) {

    /* Upon first entry, count parentheses for better user information */
    j = 0; k = 0;
    for (i = 0; i < eqnLen; i++) {
      if (equation1[i] == '(') j++;
      if (equation1[i] == ')') k++;
    }
    if (j != k) {
      print2("?Error: There are %ld left but %ld right parentheses in %s\n",
          j, k, equation1);
      exit(0);
    }

    /* Upon first entry, strip off quantifiers to leave only the
       wff part of a qwff in prenex normal form */
    for (i = eqnLen - 1; i >= -1; i--) {
      if (i == -1) break;
      if (strchr(QUANTIFIER_CONNECTIVES, equation1[i]) != NULL)
        break;
    }
    if (i >= 0) {
      /* There are quantifiers; remove and save them */
      let(&quantifiers, left(equation1, i + 2));
      let(&equation1, right(equation1, i + 3));
      eqnLen = strlen(equation1); /* Correct eqnLen */
      /* Check quantifier syntax */
      j = strlen(quantifiers);
      for (i = 0; i < j; i = i + 2) {
        if (strchr(QUANTIFIER_CONNECTIVES, quantifiers[i]) == NULL) {
          print2("?Error: Character position %ld should be @ or ] in %s%s\n",
              (long)(i + 1), quantifiers, equation1);
          print2("?Make sure input expression is in prenex normal form.\n");
          exit(0);
        }
        if (strchr(VAR_LIST, quantifiers[i + 1]) == NULL
            || quantifiers[i + 1] == 0) {
          print2(
              "?Error: Character position %ld should be a variable in %s%s\n",
              (long)(i + 2), quantifiers, equation1);
          print2("?Make sure input expression is in prenex normal form.\n");
          exit(0);
        }
        for (k = 0; k < j; k = k + 2) {
          if (k == i) continue;
          if (quantifiers[i + 1] == quantifiers[k + 1]) {
            print2("?Error: Variable %c is quantified twice in %s%s\n",
                quantifiers[i + 1], quantifiers, equation1);
            exit(0);
          }
        }
      }
    }

  } /* if (!recursiveCall) */

  subEqn1Len = subEqnLen(equation1);
  let(&subEqn1, left(equation1, subEqn1Len));

  if (subEqn1Len == eqnLen) {
    /* Handle unary prefix operators */
    if (equation1[0] == NEG_OPER || equation1[0] == NOT_OPER) {
      if (equation1[0] == NEG_OPER) {
        /* Check to make sure it does not scope metalogic or relations */
        j = strlen(equation1);
        for (i = 1; i < j; i++) {
          if (strchr(LOGIC_AND_RELATION_CONNECTIVES, equation1[i]) != NULL) {
            print2("?Error: %c should not be in scope of %c in %s\n",
                equation1[i], equation1[0], equation1);
            exit(0);
          }
        }
      }
      recursiveCall++;
      polSubEqn1 = toPolish(right(equation1, 2));
      recursiveCall--;

      /* Make sure a logical connective does scope a wff */
      if (equation1[0] == NOT_OPER) {
        if (strchr(LOGIC_AND_RELATION_CONNECTIVES, polSubEqn1[0]) == NULL) {
          print2("?Error:  %c should scope wffs in %s but %s is not a wff\n",
              NOT_OPER, equation1, right(subEqn1, 2));
          exit(0);
        }
      }

      let(&polEqn, cat(left(equation1, 1), polSubEqn1, NULL));
      goto exitPoint;
    }

    /* Handle expression enclosed in parentheses */
    if (equation1[0] == '(') {
      /* Strip parentheses */
      recursiveCall++;
      polEqn = toPolish(seg(equation1, 2, eqnLen - 1));
      recursiveCall--;
    } else {
      if (subEqn1Len != 1) {
        print2("?Error:  Expected expression %s to be length 1\n",
            equation1);
        exit(0);
      }
      if (strchr(cat(VAR_LIST, "01", NULL), equation1[0]) == NULL) {
        print2("?Error:  Expected %s to be a variable or 0 or 1\n",
            equation1);
        exit(0);
      }
      /* It's a variable or constant */
      let(&polEqn, subEqn1);
    }
    goto exitPoint;
  } /* if (subEqn1Len == eqnLen) */

  operation = equation1[subEqn1Len];
  if (strchr(ALL_BIN_CONNECTIVES, operation) == NULL) {
    print2("?Error:  Expected %c to be a binary connective in %s\n",
        operation, equation1);
    exit(0);
  }
  subEqn2Len = subEqnLen(right(equation1, subEqn1Len + 2));
  let(&subEqn2, right(equation1, subEqn1Len + 2));
  if (subEqn1Len + 1 + subEqn2Len != eqnLen) {
    print2("?Error:  Expected %s not %s after %c in %s\n",
        mid(equation1, subEqn1Len + 2, subEqn2Len), subEqn2,
        operation, equation1);
    exit(0);
  }
  /* Should be caught by above error check */
  if (subEqn2Len != strlen(subEqn2)) bug(250);

  /* Make sure a lattice operator doesn't scope a wff */
  if (strchr(OPER_AND_RELATION_BIN_CONNECTIVES, operation) != NULL) {
    /* Check to make sure it does not scope metalogic or relations */
    for (i = 0; i < subEqn1Len; i++) {
      if (strchr(LOGIC_AND_RELATION_CONNECTIVES, subEqn1[i]) != NULL) {
        print2("?Error: %c should not be in scope of %c in %s\n",
            subEqn1[i], operation, equation1);
        exit(0);
      }
    }
    for (i = 0; i < subEqn2Len; i++) {
      if (strchr(LOGIC_AND_RELATION_CONNECTIVES, subEqn2[i]) != NULL) {
        print2("?Error: %c should not be in scope of %c in %s\n",
            subEqn2[i], operation, equation1);
        exit(0);
      }
    }
  }

  /* Build the polish version */
  recursiveCall++;
  polSubEqn1 = toPolish(subEqn1);
  polSubEqn2 = toPolish(subEqn2);
  recursiveCall--;

  /* Make sure a logical connective does scope a wff */
  if (strchr(LOGIC_BIN_CONNECTIVES, operation) != NULL) {
    if (strchr(LOGIC_AND_RELATION_CONNECTIVES, polSubEqn1[0]) == NULL) {
      print2("?Error:  %c should scope wffs in %s but %s is not a wff\n",
          operation, equation1, subEqn1);
      exit(0);
    }
    if (strchr(LOGIC_AND_RELATION_CONNECTIVES, polSubEqn2[0]) == NULL) {
      print2("?Error:  %c should scope wffs in %s but %s is not a wff\n",
          operation, equation1, subEqn2);
      exit(0);
    }
  }

  let(&polEqn, cat(chr(operation), polSubEqn1, polSubEqn2, NULL));

 exitPoint:
  if (!recursiveCall
      && strchr(LOGIC_AND_RELATION_CONNECTIVES, polEqn[0]) == NULL) {
    print2("?Error:  %s is a term, not a wff\n", equation1);
    exit(0);
  }

  if (!recursiveCall) {
    /* Place any quantifiers back onto result */
    let(&polEqn, cat(quantifiers, polEqn, NULL));
  }

  /* Print the Polish subequation for debugging */
  /*print2("%ld %s\n",recursiveCall, polEqn);*/

  /* Deallocate strings */
  let(&equation1, "");
  let(&subEqn1, "");
  let(&subEqn2, "");
  let(&polSubEqn1, "");
  let(&polSubEqn2, "");

  return polEqn;

} /* toPolish */


/* Called by toPolish */
/* Returns length of first subequation */
long subEqnLen(vstring subEqn)
{
  long i, j, k;
  i = 0;
  j = 0;
  k = strlen(subEqn);
  while (1) {
    if (i >= k) break;
    if (subEqn[i] == NEG_OPER || subEqn[i] == NOT_OPER) {
      i++;
      continue;
    }
    if (subEqn[i] == '(') {
      j++;
      i++;
      continue;
    }
    if (subEqn[i] == ')') {
      j--;
    }
    i++;
    if (j == 0) break;
  }
  return i;
} /* subEqnLen */



/* ***Note:  The caller must deallocate returned string */
/* Convert Polish to normal notation */
vstring fromPolish(vstring polishEqn)
{
  /* This function converts a theorem in Polish notation to one
     in parentheses notation */
  vstring stack[MAX_STACK2];
  long i, stackPtr;
  long maxStack = 0;
  vstring stackEntry = "";
  vstring ch = "";
  vstring polishEqn1 = "";
  let(&polishEqn1, polishEqn); /* In case temp alloc string is passed in */

  stackPtr = 0;
  for (i = len(polishEqn1); i >= 1; i--) {
    let(&ch, mid(polishEqn1, i, 1));
    if (ch[0] == NEG_OPER || ch[0] == NOT_OPER) {
      if (stackPtr > 0) {
        let(&(stack[stackPtr]), cat(ch, stack[stackPtr], NULL));
      } else {
        print2("?Error 8: Stack underflow at %ld\n", i);
        exit(0);
      }
    } else {
      if (instr(1, ALL_BIN_CONNECTIVES, ch)) {
        if (stackPtr > 1) {
          stackPtr--;
          let(&(stack[stackPtr]), cat("(", stack[stackPtr + 1], ch,
              stack[stackPtr], ")", NULL));
        } else {
          print2("?Error 9:  Stack underflow at %ld\n", i);
          exit(0);
        }
      } else { /* Variable assumed */
        stackPtr++;
        if (stackPtr >= MAX_STACK2) {
          print2(
           "?Error 12:  Stack overflow - increase MAX_STACK2 and recompile\n");
          exit(0);
        }
        if (stackPtr > maxStack) {
          maxStack = stackPtr;
          stack[stackPtr] = "";
        }
        let(&(stack[stackPtr]), ch);
      }
    }
  } /* next i */
  if (stackPtr != 1) {
    print2("?Error 10: Stack not emptied\n");
    exit(0);
  }
  /* Deallocate vstring array */
  for (i = 2; i <= maxStack; i++) {
    let(&(stack[i]), "");
  }
  let(&ch, "");
  let(&stackEntry, "");
  let(&polishEqn1, "");
  return (stack[stackPtr]);
} /* fromPolish */

/* Convert a printable string to internal upper ASCII */
/* This function converts ascii "&a", "&b", ... to 'a'+128, 'b'+128,... */
/* Use like any vstring function (left, right,...) */
/* This function (used to parse lattice.c Hasse diagrams only)
   does not (and needs not) deal with the metalogical
   TRUE_CONST and FALSE_CONST */
vstring unPrintableString(vstring sin)
{
  vstring sout;
  long i, j, n;
  unsigned char c;
  j = strlen(sin);
  n = 0;
  for (i = 0; i < j; i++) if (sin[i] == '&') n++;
  sout = tempAlloc(j - n + 1);
  sout[j - n] = 0;
  n = 0;
  for (i = 0; i < j; i++) {
    if (sin[i] == '&') {
      c = sin[i + 1];
      c += 128;
      sout[i - n] = c;
      n++;
      i++;
    } else {
      sout[i - n] = sin[i];
    }
  }
  return sout;
}

/* Return a printable string */
/* This function converts ascii 'a'+128, 'b'+128,... to "&a", "&b", ... */
/* It also converts FALSE_CONST and TRUE_CONST to "F" and "T" - which
   could be ambigous with node names but usually shouldn't cause user
   confusion */
/* Use like any vstring function (left, right,...) */
vstring printableString(vstring sin)
{
  vstring sout;
  long i, j, k, n;
  j = strlen(sin);
  n = 0;
  for (i = 0; i < j; i++) {
    /* If k is eliminated below, it introduces a bug in the lcc compiler
       with the optimization option */
    k = (unsigned char)(sin[i]);
    /* FALSE_CONST and TRUE_CONST will normally be less than 127
       (e.g. 1 and 2) but since it is not strictly required we
       AND them in for extra safety */
    if (k > 127 && k != FALSE_CONST && k != TRUE_CONST) n++;
  }
  sout = tempAlloc(j + n + 1);
  sout[j + n] = 0;
  n = 0;
  for (i = 0; i < j; i++) {
    k = (unsigned char)(sin[i]);
    if (k == FALSE_CONST) {
      sout[i + n] = 'F';
    } else {
      if (k == TRUE_CONST) {
        sout[i + n] = 'T';
      } else {
        if (k > 127) {
          sout[i + n] = '&';
          n++;
          sout[i + n] = k - 128;
        } else {
          sout[i + n] = sin[i];
        }
      }
    }
  }
  return (sout);
}



/*****************************************************************************/
/*       Copyright (C) 2000  NORMAN D. MEGILL  <nm@alum.mit.edu>             */
/*             License terms:  GNU General Public License                    */
/*****************************************************************************/

/*34567890123456 (79-character line to adjust text window width) 678901234567*/
/*
mmvstr.h - VMS-BASIC variable length string library routines header
This is a collection of useful built-in string functions available in VMS BASIC.
*/

/**************************************************************************

Variable-length string handler
------------------------------

     This collection of string-handling functions emulate most of the
string functions of VMS BASIC.  The objects manipulated by these functions
are strings of a special type called 'vstring' which
have no pre-defined upper length limit but are dynamically allocated
and deallocated as needed.  To use the vstring functions within a program,
all vstrings must be initially set to the null string when declared or
before first used, for example:

        vstring string1 = "";
        vstring stringArray[] = {"","",""};

        vstring bigArray[100][10]; /- Must be initialized before using -/
        int i,j;
        for (i=0; i<100; i++)
          for (j=0; j<10; j++)
            bigArray[i][j] = ""; /- Initialize -/


     After initialization, vstrings should be assigned with the 'let(&'
function only; for example the statements

        let(&string1,"abc");
        let(&string1,string2);
        let(&string1,left(string2,3));

all assign the second argument to 'string1'.  The 'let(&' function must
not be used to initialize a vstring for the first time.

     The 'cat' function emulates the '+' concatenation operator in BASIC.
It has a variable number of arguments, and the last argument should always
be NULL.  For example,

        let(&string1,cat("abc","def",NULL));

assigns "abcdef" to 'string1'.  Warning: 0 will work instead of NULL on the
VAX but not on the Macintosh, so always use NULL.

     All other functions are generally used exactly like their BASIC
equivalents.  For example, the BASIC statement

        let string1$=left$("def",len(right$("xxx",2)))+"ghi"+string2$

is emulated in c as

        let(&string1,cat(left("def",len(right("xxx",2))),"ghi",string2,NULL));

Note that ANSII c does not allow "$" as part of an identifier
name, so the names in c have had the "$" suffix removed.

     The string arguments of the vstring functions may be either standard c
strings or vstrings (except that the first argument of the 'let(&' function
must be a vstring).  The standard c string functions may use vstrings or
vstring functions as their string arguments, as long as the vstring variable
itself (which is a char * pointer) is not modified and no attempt is made to
increase the length of a vstring.  Caution must be excercised when
assigning standard c string pointers to vstrings or the results of
vstring functions, as the memory space may be deallocated when the
'let(&' function is next executed.  For example,

        char *stdstr; /- A standard c string pointer -/
         ...
        stdstr=left("abc",2);

will assign "ab" to 'stdstr', but this assignment will be lost when the
next 'let(&' function is executed.  To be safe, use 'strcpy':

        char stdstr1[80]; /- A fixed length standard c string -/
         ...
        strcpy(stdstr1,left("abc",2));

Here, of course, the user must ensure that the string copied to 'stdstr1'
does not exceed 79 characters in length.

     The vstring functions allocate temporary memory whenever they are called.
This temporary memory is deallocated whenever a 'let(&' assignment is
made.  The user should be aware of this when using vstring functions
outside of 'let(&' assignments; for example

        for (i=0; i<10000; i++)
          print2("%s\n",left(string1,70));

will allocate another 70 bytes or so of memory each pass through the loop.
If necessary, dummy 'let(&' assignments can be made periodically to clear
this temporary memory:

        for (i=0; i<10000; i++)
          {
          print2("%s\n",left(string1,70));
          let(&dummy,"");
          }

It should be noted that the 'linput' function assigns its target string
with 'let(&' and thus has the same effect as 'let(&'.

************************************************************************/


vstring tempAlloc(long size)    /* String memory allocation/deallocation */
{
  /* When "size" is >0, "size" bytes are allocated. */
  /* When "size" is 0, all memory previously allocated with this */
  /* function is deallocated. */
  /* EXCEPT:  When startTempAllocStack != 0, the freeing will start at
     startTempAllocStack. */
  int i;
  if (size) {
    if (tempAllocStackTop>=(MAX_ALLOC_STACK-1)) {
      print2("?Error: Temporary string stack overflow\n");
      bug(101);
    }
    if (!(tempAllocStack[tempAllocStackTop++]=malloc(size))) {
      print2("?Error: Temporary string allocation failed\n");
      bug(102);
    }
    return (tempAllocStack[tempAllocStackTop-1]);
  } else {
    for (i=startTempAllocStack; i<tempAllocStackTop; i++) {
      free(tempAllocStack[i]);
    }
    tempAllocStackTop=startTempAllocStack;
    return (NULL);
  }
}


/* Make string have temporary allocation to be released by next let() */
/* Warning:  after makeTempAlloc() is called, the genString may NOT be
   assigned again with let() */
void makeTempAlloc(vstring s)
{
    if (tempAllocStackTop>=(MAX_ALLOC_STACK-1)) {
      print2("?Error: Temporary string stack overflow\n");
      bug(103);
    }
    tempAllocStack[tempAllocStackTop++]=s;
}


void let(vstring *target,vstring source)        /* String assignment */
/* This function must ALWAYS be called to make assignment to */
/* a vstring in order for the memory cleanup routines, etc. */
/* to work properly.  If a vstring has never been assigned before, */
/* it is the user's responsibility to initialize it to "" (the */
/* null string). */
{
  long targetLength,sourceLength;

  sourceLength=strlen(source);  /* Save its length */
  targetLength=strlen(*target); /* Save its length */
  if (targetLength) {
    if (sourceLength) { /* source and target are both nonzero length */

      if (targetLength>=sourceLength) { /* Old string has room for new one */
        strcpy(*target,source); /* Re-use the old space to save CPU time */
      } else {
        /* Free old string space and allocate new space */
        free(*target);  /* Free old space */
        *target=malloc(sourceLength+1); /* Allocate new space */
        if (!*target) {
          print2("?Error: String memory couldn't be allocated\n");
          bug(104);
        }
        strcpy(*target,source);
      }

    } else {    /* source is 0 length, target is not */
      free(*target);
      *target= "";
    }
  } else {
    if (sourceLength) { /* target is 0 length, source is not */
      *target=malloc(sourceLength+1);   /* Allocate new space */
      if (!*target) {
        print2("?Error: Could not allocate string memory\n");
        bug(105);
      }
      strcpy(*target,source);
    } else {    /* source and target are both 0 length */
      *target= "";
    }
  }

  tempAlloc(0); /* Free up temporary strings used in expression computation */

}




vstring cat(vstring string1,...)        /* String concatenation */
#define MAX_CAT_ARGS 30
{
  va_list ap;   /* Declare list incrementer */
  vstring arg[MAX_CAT_ARGS];    /* Array to store arguments */
  long argLength[MAX_CAT_ARGS]; /* Array to store argument lengths */
  int numArgs=1;        /* Define "last argument" */
  int i;
  long j;
  vstring ptr;

  arg[0]=string1;       /* First argument */

  va_start(ap,string1); /* Begin the session */
  while ((arg[numArgs++]=va_arg(ap,char *)))
        /* User-provided argument list must terminate with 0 */
    if (numArgs>=MAX_CAT_ARGS-1) {
      print2("?Error: Too many cat() arguments\n");
      bug(106);
    }
  va_end(ap);           /* End var args session */

  numArgs--;    /* The last argument (0) is not a string */

  /* Find out the total string length needed */
  j=0;
  for (i=0; i<numArgs; i++) {
    argLength[i]=strlen(arg[i]);
    j=j+argLength[i];
  }
  /* Allocate the memory for it */
  ptr=tempAlloc(j+1);
  /* Move the strings into the newly allocated area */
  j=0;
  for (i=0; i<numArgs; i++) {
    strcpy(ptr+j,arg[i]);
    j=j+argLength[i];
  }
  return (ptr);

}


/* input a line from the user or from a file */
vstring linput(FILE *stream,vstring ask,vstring *target)
{
  /*
    BASIC:  linput "what";a$
    c:      linput(NULL,"what?",&a);

    BASIC:  linput #1,a$                        (error trap on EOF)
    c:      if (!linput(file1,NULL,&a)) break;  (break on EOF)

  */
  /* This function prints a prompt (if 'ask' is not NULL), gets a line from
    the stream, and assigns it to target using the let(&...) function.
    NULL is returned when end-of-file is encountered.  The vstring
    *target MUST be initialized to "" or previously assigned by let(&...)
    before using it in linput. */
  char f[10001]; /* Allow up to 10000 characters */
  if (ask) print2("%s",ask);
  if (stream == NULL) stream = stdin;
  if (!fgets(f,10000,stream)) {
    /* End of file */
    return NULL;
  }
  f[10000]=0;     /* Just in case */
  f[strlen(f)-1]=0;     /* Eliminate new-line character */
  /* Assign the user's input line */
  let(target,f);
  return *target;
}


/* Find out the length of a string */
long len(vstring s)
{
  return (strlen(s));
}


/* Extract sin from character position start to stop into sout */
vstring seg(vstring sin,long start,long stop)
{
  vstring sout;
  long len;
  if (start<1) start=1;
  if (stop<1) stop=0;
  len=stop-start+1;
  if (len<0) len=0;
  sout=tempAlloc(len+1);
  strncpy(sout,sin+start-1,len);
  sout[len]=0;
  return (sout);
}

/* Extract sin from character position start for length len */
vstring mid(vstring sin,long start,long len)
{
  vstring sout;
  if (start<1) start=1;
  if (len<0) len=0;
  sout=tempAlloc(len+1);
  strncpy(sout,sin+start-1,len);
/*??? Should db be substracted from if len > end of string? */
  sout[len]=0;
  return (sout);
}

/* Extract leftmost n characters */
vstring left(vstring sin,long n)
{
  vstring sout;
  if (n < 0) n = 0;
  sout=tempAlloc(n+1);
  strncpy(sout,sin,n);
  sout[n]=0;
  return (sout);
}

/* Extract after character n */
vstring right(vstring sin,long n)
{
  /*??? We could just return &sin[n-1], but this is safer for debugging. */
  vstring sout;
  long i;
  if (n<1) n=1;
  i = strlen(sin);
  if (n>i) return ("");
  sout = tempAlloc(i - n + 2);
  strcpy(sout,&sin[n-1]);
  return (sout);
}

/* Emulate VMS BASIC edit$ command */
vstring edit(vstring sin,long control)
#define isblank(c) ((c==' ') || (c=='\t'))
{
/*
EDIT$
  Syntax
         str-vbl = EDIT$(str-exp, int-exp)
     Values   Effect
     1        Trim parity bits
     2        Discard all spaces and tabs
     4        Discard characters: CR, LF, FF, ESC, RUBOUT, and NULL
     8        Discard leading spaces and tabs
     16       Reduce spaces and tabs to one space
     32       Convert lowercase to uppercase
     64       Convert [ to ( and ] to )
     128      Discard trailing spaces and tabs
     256      Do not alter characters inside quotes

     (non-BASIC extensions)
     512      Convert uppercase to lowercase
     1024     Tab the line (convert spaces to equivalent tabs)
     2048     Untab the line (convert tabs to equivalent spaces)
     4096     Convert VT220 screen print frame graphics to -,|,+ characters
*/
  vstring sout;
  long i,j,k;
  int last_char_is_blank;
  int trim_flag,discardcr_flag,bracket_flag,quote_flag,case_flag;
  int alldiscard_flag,leaddiscard_flag,traildiscard_flag,reduce_flag;
  int processing_inside_quote=0;
  int lowercase_flag, tab_flag, untab_flag, screen_flag;
  unsigned char graphicsChar;

  /* Set up the flags */
  trim_flag=control & 1;
  alldiscard_flag=control & 2;
  discardcr_flag=control & 4;
  leaddiscard_flag=control & 8;
  reduce_flag=control & 16;
  case_flag=control & 32;
  bracket_flag=control & 64;
  traildiscard_flag=control & 128;
  quote_flag=control & 256;

  /* Non-BASIC extensions */
  lowercase_flag = control & 512;
  tab_flag = control & 1024;
  untab_flag = control & 2048;
  screen_flag = control & 4096; /* Convert VT220 screen prints to |,-,+
                                   format */

  /* Copy string */
  i = strlen(sin) + 1;
  if (untab_flag) i = i * 7;
  sout=tempAlloc(i);
  strcpy(sout,sin);

  /* Discard leading space/tab */
  i=0;
  if (leaddiscard_flag)
    while ((sout[i]!=0) && isblank(sout[i]))
      sout[i++]=0;

  /* Main processing loop */
  while (sout[i]!=0) {

    /* Alter characters inside quotes ? */
    if (quote_flag && ((sout[i]=='"') || (sout[i]=='\'')))
       processing_inside_quote=~processing_inside_quote;
    if (processing_inside_quote) {
       /* Skip the rest of the code and continue to process next character */
       i++; continue;
    }

    /* Discard all space/tab */
    if ((alldiscard_flag) && isblank(sout[i]))
        sout[i]=0;

    /* Trim parity (eighth?) bit */
    if (trim_flag)
       sout[i]=sout[i] & 0x7F;

    /* Discard CR,LF,FF,ESC,BS */
    if ((discardcr_flag) && (
         (sout[i]=='\015') || /* CR  */
         (sout[i]=='\012') || /* LF  */
         (sout[i]=='\014') || /* FF  */
         (sout[i]=='\033') || /* ESC */
         /*(sout[i]=='\032') ||*/ /* ^Z */ /* DIFFERENCE won't work w/ this */
         (sout[i]=='\010')))  /* BS  */
      sout[i]=0;

    /* Convert lowercase to uppercase */
    if ((case_flag) && (islower(sout[i])))
       sout[i]=toupper(sout[i]);

    /* Convert [] to () */
    if ((bracket_flag) && (sout[i]=='['))
       sout[i]='(';
    if ((bracket_flag) && (sout[i]==']'))
       sout[i]=')';

    /* Convert uppercase to lowercase */
    if ((lowercase_flag) && (islower(sout[i])))
       sout[i]=tolower(sout[i]);

    /* Convert VT220 screen print frame graphics to +,|,- */
    if (screen_flag) {
      graphicsChar = sout[i]; /* Need unsigned char for >127 */
      /* vt220 */
      if (graphicsChar >= 234 && graphicsChar <= 237) sout[i] = '+';
      if (graphicsChar == 241) sout[i] = '-';
      if (graphicsChar == 248) sout[i] = '|';
      /* vt100 */
      if (graphicsChar == 218 /*up left*/ || graphicsChar == 217 /*lo r*/
          || graphicsChar == 191 /*up r*/ || graphicsChar == 192 /*lo l*/)
        sout[i] = '+';
      if (graphicsChar == 196) sout[i] = '-';
      if (graphicsChar == 179) sout[i] = '|';
    }

    /* Process next character */
    i++;
  }
  /* sout[i]=0 is the last character at this point */

  /* Clean up the deleted characters */
  for (j=0,k=0; j<=i; j++)
    if (sout[j]!=0) sout[k++]=sout[j];
  sout[k]=0;
  /* sout[k]=0 is the last character at this point */

  /* Discard trailing space/tab */
  if (traildiscard_flag) {
    --k;
    while ((k>=0) && isblank(sout[k])) --k;
    sout[++k]=0;
  }

  /* Reduce multiple space/tab to a single space */
  if (reduce_flag) {
    i=j=last_char_is_blank=0;
    while (i<=k-1) {
      if (!isblank(sout[i])) {
        sout[j++]=sout[i++];
        last_char_is_blank=0;
      } else {
        if (!last_char_is_blank)
          sout[j++]=' '; /* Insert a space at the first occurrence of a blank */
        last_char_is_blank=1; /* Register that a blank is found */
        i++; /* Process next character */
      }
    }
    sout[j]=0;
  }

  /* Untab the line */
  if (untab_flag || tab_flag) {

    /*
    DEF FNUNTAB$(L$)      ! UNTAB LINE L$
    I9%=1%
    I9%=INSTR(I9%,L$,CHR$(9%))
    WHILE I9%
      L$=LEFT(L$,I9%-1%)+SPACE$(8%-((I9%-1%) AND 7%))+RIGHT(L$,I9%+1%)
      I9%=INSTR(I9%,L$,CHR$(9%))
    NEXT
    FNUNTAB$=L$
    FNEND
    */

    k = strlen(sout);
    for (i = 1; i <= k; i++) {
      if (sout[i - 1] != '\t') continue;
      for (j = k; j >= i; j--) {
        sout[j + 8 - ((i - 1) & 7) - 1] = sout[j];
      }
      for (j = i; j < i + 8 - ((i - 1) & 7); j++) {
        sout[j - 1] = ' ';
      }
      k = k + 8 - ((i - 1) & 7);
    }
  }

  /* Tab the line */
  if (tab_flag) {

    /*
    DEF FNTAB$(L$)        ! TAB LINE L$
    I9%=0%
    FOR I9%=8% STEP 8% WHILE I9%<LEN(L$)
      J9%=I9%
      J9%=J9%-1% UNTIL ASCII(MID(L$,J9%,1%))<>32% OR J9%=I9%-8%
      IF J9%<=I9%-2% THEN
        L$=LEFT(L$,J9%)+CHR$(9%)+RIGHT(L$,I9%+1%)
        I9%=J9%+1%
      END IF
    NEXT I9%
    FNTAB$=L$
    FNEND
    */

    i = 0;
    k = strlen(sout);
    for (i = 8; i < k; i = i + 8) {
      j = i;
      while (sout[j - 1] == ' ' && j > i - 8) j--;
      if (j <= i - 2) {
        sout[j] = '\t';
        j = i;
        while (sout[j - 1] == ' ' && j > i - 8 + 1) {
          sout[j - 1] = 0;
          j--;
        }
      }
    }
    i = k;
    /* sout[i]=0 is the last character at this point */
    /* Clean up the deleted characters */
    for (j = 0, k = 0; j <= i; j++)
      if (sout[j] != 0) sout[k++] = sout[j];
    sout[k] = 0;
    /* sout[k]=0 is the last character at this point */
  }

  return (sout);
}


/* Return a string of the same character */
vstring string(long n, char c)
{
  vstring sout;
  long j=0;
  if (n<0) n=0;
  sout=tempAlloc(n+1);
  while (j<n) sout[j++]=c;
  sout[j]=0;
  return (sout);
}


/* Return a string of spaces */
vstring space(long n)
{
  return (string(n,' '));
}


/* Return a character given its ASCII value */
vstring chr(long n)
{
  vstring sout;
  sout=tempAlloc(2);
  sout[0]= n & 0xFF;
  sout[1]=0;
  return(sout);
}


/* Search for string2 in string 1 starting at start_position */
long instr(long start_position,vstring string1,vstring string2)
{
   char *sp1,*sp2;
   long ls1,ls2;
   long found=0;
   if (start_position<1) start_position=1;
   ls1=strlen(string1);
   ls2=strlen(string2);
   if (start_position>ls1) start_position=ls1+1;
   sp1=string1+start_position-1;
   while ((sp2=strchr(sp1,string2[0]))!=0) {
     if (strncmp(sp2,string2,ls2)==0) {
        found=sp2-string1+1;
        break;
     } else
        sp1=sp2+1;
   }
   return (found);
}


/* Translate string in sin to sout based on table.
   Table must be 256 characters long!! <- not true anymore? */
vstring xlate(vstring sin,vstring table)
{
  vstring sout;
  long len_table,len_sin;
  long i,j;
  long table_entry;
  char m;
  len_sin=strlen(sin);
  len_table=strlen(table);
  sout=tempAlloc(len_sin+1);
  for (i=j=0; i<len_sin; i++)
  {
    table_entry= 0x000000FF & (long)sin[i];
    if (table_entry<len_table)
      if ((m=table[table_entry])!='\0')
        sout[j++]=m;
  }
  sout[j]='\0';
  return (sout);
}


/* Returns the ascii value of a character */
long ascii_(vstring c)
{
  return (long)((unsigned char)(c[0]));
}

/* Returns the floating-point value of a numeric string */
double val(vstring s)
{
  /*
  return (atof(s));
  */
  /* 12/10/98 - NDM - atof may corrupt memory when processing
     random character strings.
     The implementation below makes best guess of value of any
     random string, ignoring commas, etc. and tolerating numbers
     suffixed with sign.  "E" notation is not handled. */
  double v = 0;
  char signFound = 0;
  double power = 1.0;
  long i;
  /* Scan from lsd backwards to minimize rounding errors */
  for (i = strlen(s) - 1; i >= 0; i--) {
    switch (s[i]) {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        v = v + ((double)(s[i] - '0')) * power;
        power = 10.0 * power;
        break;
      case '.':
        v = v / power;
        power = 1.0;
        break;
      case '-':
        signFound = 1;
        break;
    }
  }
  if (signFound) v = - v;
  return v;
}


/* Returns current date as an ASCII string */
vstring date()
{
        vstring sout;
        struct tm *time_structure;
        time_t time_val;
        char *month[12];

        /* (Aggregrate initialization is not portable) */
        /* (It must be done explicitly for portability) */
        month[0]="Jan";
        month[1]="Feb";
        month[2]="Mar";
        month[3]="Apr";
        month[4]="May";
        month[5]="Jun";
        month[6]="Jul";
        month[7]="Aug";
        month[8]="Sep";
        month[9]="Oct";
        month[10]="Nov";
        month[11]="Dec";

        time(&time_val);                        /* Retrieve time */
        time_structure=localtime(&time_val); /* Translate to time structure */
        sout=tempAlloc(12);
        /* "%02d" means leading zeros with min. field width of 2 */
        sprintf(sout,"%d-%s-%02d",
                time_structure->tm_mday,
                month[time_structure->tm_mon],
                (int)((time_structure->tm_year) % 100)); /* Y2K */
        return(sout);
}

/* Return current time as an ASCII string */
vstring time_()
{
        vstring sout;
        struct tm *time_structure;
        time_t time_val;
        int i;
        char *format;
        char *format1="%d:%d %s";
        char *format2="%d:0%d %s";
        char *am_pm[2];
        /* (Aggregrate initialization is not portable) */
        /* (It must be done explicitly for portability) */
        am_pm[0]="AM";
        am_pm[1]="PM";

        time(&time_val);                        /* Retrieve time */
        time_structure=localtime(&time_val); /* Translate to time structure */
        if (time_structure->tm_hour>=12) i=1;
        else                             i=0;
        if (time_structure->tm_hour>12) time_structure->tm_hour-=12;
        if (time_structure->tm_hour==0) time_structure->tm_hour=12;
        sout=tempAlloc(12);
        if (time_structure->tm_min>=10)
          format=format1;
        else
          format=format2;
        sprintf(sout,format,
                time_structure->tm_hour,
                time_structure->tm_min,
                am_pm[i]);
        return(sout);

}


/* Return a number as an ASCII string */
vstring str(double f)
{
  /* This function converts a floating point number to a string in the */
  /* same way that %f in printf does, except that trailing zeroes after */
  /* the one after the decimal point are stripped; e.g., it returns 7 */
  /* instead of 7.000000000000000. */
  vstring s;
  long i;
  s = tempAlloc(50);
  sprintf(s, "%f", f);
  if (strchr(s, '.') != 0) {              /* the string has a period in it */
    for (i = strlen(s) - 1; i > 0; i--) { /* scan string backwards */
      if (s[i] != '0') break;             /* 1st non-zero digit */
      s[i] = 0;                           /* delete the trailing 0 */
    }
    if (s[i] == '.') s[i] = 0;            /* delete trailing period */
  }
  return (s);
}


/* Return a number as an ASCII string */
vstring num1(double f)
{
  return (str(f));
}


/* Return a number as an ASCII string surrounded by spaces */
vstring num(double f)
{
  return (cat(" ",str(f)," ",NULL));
}


/*** NEW FUNCTIONS ADDED 11/25/98 ***/

/* Emulate PROGRESS "entry" and related string functions */
/* (PROGRESS is a 4-GL database language) */

/* A "list" is a string of comma-separated elements.  Example:
   "a,b,c" has 3 elements.  "a,b,c," has 4 elements; the last element is
   an empty string.  ",," has 3 elements; each is an empty string.
   In "a,b,c", the entry numbers of the elements are 1, 2 and 3 (i.e.
   the entry numbers start a 1, not 0). */

/* Returns a character string entry from a comma-separated
   list based on an integer position. */
/* If element is less than 1 or greater than number of elements
   in the list, a null string is returned. */
vstring entry(long element, vstring list)
{
  vstring sout;
  long commaCount, lastComma, i, len;
  if (element < 1) return ("");
  lastComma = -1;
  commaCount = 0;
  i = 0;
  while (list[i] != 0) {
    if (list[i] == ',') {
      commaCount++;
      if (commaCount == element) {
        break;
      }
      lastComma = i;
    }
    i++;
  }
  if (list[i] == 0) commaCount++;
  if (element > commaCount) return ("");
  len = i - lastComma - 1;
  if (len < 1) return ("");
  sout = tempAlloc(len + 1);
  strncpy(sout, list + lastComma + 1, len);
  sout[len] = 0;
  return (sout);
}

/* Emulate PROGRESS lookup function */
/* Returns an integer giving the first position of an expression
   in a comma-separated list. Returns a 0 if the expression
   is not in the list. */
long lookup(vstring expression, vstring list)
{
  long i, exprNum, exprPos;
  char match;

  match = 1;
  i = 0;
  exprNum = 0;
  exprPos = 0;
  while (list[i] != 0) {
    if (list[i] == ',') {
      exprNum++;
      if (match) {
        if (expression[exprPos] == 0) return exprNum;
      }
      exprPos = 0;
      match = 1;
      i++;
      continue;
    }
    if (match) {
      if (expression[exprPos] != list[i]) match = 0;
    }
    i++;
    exprPos++;
  }
  exprNum++;
  if (match) {
    if (expression[exprPos] == 0) return exprNum;
  }
  return 0;
}


/* Emulate PROGRESS num-entries function */
/* Returns the number of items in a comma-separated list. */
long numEntries(vstring list)
{
  long i, commaCount;
  i = 0;
  commaCount = 0;
  while (list[i] != 0) {
    if (list[i] == ',') commaCount++;
    i++;
  }
  return (commaCount + 1);
}

/* Returns the character position of the start of the
   element in a list - useful for manipulating
   the list string directly.  1 means the first string
   character. */
/* If element is less than 1 or greater than number of elements
   in the list, a 0 is returned.  If entry is null, a 0 is
   returned. */
long entryPosition(long element, vstring list)
{
  long commaCount, lastComma, i;
  if (element < 1) return 0;
  lastComma = -1;
  commaCount = 0;
  i = 0;
  while (list[i] != 0) {
    if (list[i] == ',') {
      commaCount++;
      if (commaCount == element) {
        break;
      }
      lastComma = i;
    }
    i++;
  }
  if (list[i] == 0) {
    if (i == 0) return 0;
    if (list[i - 1] == ',') return 0;
    commaCount++;
  }
  if (element > commaCount) return (0);
  if (list[lastComma + 1] == ',') return 0;
  return (lastComma + 2);
}


void print2(char* fmt,...)
{
  /* This performs the same operations as printf, except that if a log file is
    open, the characters will also be printed to the log file. */
  va_list ap;
  char printBuffer[10001];

  va_start(ap, fmt);
  vsprintf(printBuffer, fmt, ap); /* Put formatted string into buffer */
  va_end(ap);

  printf("%s", printBuffer); /* Terminal */

  if (fplog != NULL) {
    fprintf(fplog, "%s", printBuffer);  /* Print to log file */
  }
  return;
}

/* Opens files with error message; opens output files with
   backup of previous version.   Mode must be "r" or "w". */
FILE *fSafeOpen(vstring fileName, vstring mode)
{
  FILE *fp;
  vstring prefix = "";
  vstring postfix = "";
  vstring bakName = "";
  vstring newBakName = "";
  long v;

  if (!strcmp(mode, "r")) {
    fp = fopen(fileName, "r");
    if (!fp) {
      print2("?Sorry, couldn't open the file \"%s\".\n", fileName);
    }
    return (fp);
  }

  if (!strcmp(mode, "w")) {
    /* See if the file already exists. */
    fp = fopen(fileName, "r");

    if (fp) {
      fclose(fp);

#define VERSIONS 9
      /* The file exists.  Rename it. */

#if defined __WATCOMC__ /* MSDOS */
      /* Make sure file name before extension is 8 chars or less */
      i = instr(1, fileName, ".");
      if (i) {
        let(&prefix, left(fileName, i - 1));
        let(&postfix, right(fileName, i));
      } else {
        let(&prefix, fileName);
        let(&postfix, "");
      }
      let(&prefix, cat(left(prefix, 5), "~", NULL));
      let(&postfix, cat("~", postfix, NULL));
      if (0) goto skip_backup; /* Prevent compiler warning */

#elif defined __GNUC__ /* Assume unix */
      let(&prefix, cat(fileName, "~", NULL));
      let(&postfix, "");

#elif defined THINK_C /* Assume Macintosh */
      let(&prefix, cat(fileName, "~", NULL));
      let(&postfix, "");

#elif defined VAXC /* Assume VMS */
      /* For debugging on VMS: */
      /* let(&prefix, cat(fileName, "-", NULL));
         let(&postfix, "-"); */
      /* Normal: */
      goto skip_backup;

#else /* Unknown; assume unix standard */
      /*if (1) goto skip_backup;*/  /* [if no backup desired] */
      let(&prefix, cat(fileName, "~", NULL));
      let(&postfix, "");

#endif


      /* See if the lowest version already exists. */
      let(&bakName, cat(prefix, str(1), postfix, NULL));
      fp = fopen(bakName, "r");
      if (fp) {
        fclose(fp);
        /* The lowest version already exists; rename all to lower versions. */

        /* If version VERSIONS exists, delete it. */
        let(&bakName, cat(prefix, str(VERSIONS), postfix, NULL));
        fp = fopen(bakName, "r");
        if (fp) {
          fclose(fp);
          remove(bakName);
        }

        for (v = VERSIONS - 1; v >= 1; v--) {
          let(&bakName, cat(prefix, str(v), postfix, NULL));
          fp = fopen(bakName, "r");
          if (!fp) continue;
          fclose(fp);
          let(&newBakName, cat(prefix, str(v + 1), postfix, NULL));
          rename(bakName, newBakName);
        }

      }
      let(&bakName, cat(prefix, str(1), postfix, NULL));
      rename(fileName, bakName);

      /***
      printLongLine(cat("The file \"", fileName,
          "\" already exists.  The old file is being renamed to \"",
          bakName, "\".", NULL), "  ", " ");
      ***/
    } /* End if file already exists */
   /*skip_backup:*/

    fp = fopen(fileName, "w");
    if (!fp) {
      print2("?Sorry, couldn't open the file \"%s\".\n", fileName);
    }

    let(&prefix, "");
    let(&postfix, "");
    let(&bakName, "");
    let(&newBakName, "");

    return (fp);
  } /* End if mode = "w" */

  bug(1510); /* Illegal mode */
  return(NULL);

}


/* Bug check */
void bug(int bugNum)
{
  print2("?Error: Program bug # %d\n", bugNum);
  exit(0);
}
