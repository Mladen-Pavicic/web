/*****************************************************************************/
/* states.c - State tester for Greechie diagrams                             */
/*                                                                           */
/*       Copyright (C) 2000  NORMAN D. MEGILL  <nm@alum.mit.edu>             */
/*             License terms:  GNU General Public License                    */
/*****************************************************************************/
/*34567890123456 (79-character line to adjust text window width) 678901234567*/

/* Copyright notice:  This program contains code taken from the lp_solve
   GPL program lp_solve, version 3.0, available at:
         ftp://ftp.ics.ele.tue.nl/pub/lp_solve/ */

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
#define VAR_LIST "abcdefghjklmnpqrstuwxyzABCDEFGHJKLMNPQRSTUWXYZ"
/* MAX_VARS is the length of VAR_LIST */
#define MAX_VARS /*23*/ 46
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

/* Modes for state.c */
#define BAD_STATE_MODE 0
#define DISPERSION_FREE 1
#define STRONG 2
#define FULL 3
#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

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
char stateTest(void); /* states.c */
void statePrntEqn(long node1, long node2);  /* states.c */
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
long fileLattices = 0; /* Number of input file diagrams */
long fileLineNum = 0; /* Most recent line number (= lattice number) read */

/* The block stuff was made global for states.c */
/* Block information for Greechie diagrams */
long atoms;
long block[MAX_BLOCKS + 1][MAX_BLOCK_SIZE + 1];
long blockSize[MAX_BLOCKS + 1];
long block4Offset[MAX_BLOCKS + 1];
long blocks;

/* state.c - state mode */
char stateMode = BAD_STATE_MODE; /* Init to bad so user must specify it */


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

/* states.c */
char printEquation = 0; /* Set to 1 to print lattice or state
                           equation (-q,-qs options) */
char printStateEquation = 0; /* Set to 1 to print state equation (-qs option) */
char noStatesMode = 0; /* Scan only for lattices with no states (-x option) */


/*************** Start of headers for lp_solve *******************************/
/*************************** hash.h *********************************/

typedef struct _hashelem
{
  char             *name;
  struct _hashelem *next;
  struct _column   *col;
  struct _bound    *bnd;
  int               must_be_int;
  int               index; /* for row and column name hash tables */
} hashelem;

typedef struct _hashtable
{
  hashelem **table;
  int        size;
} hashtable;

hashtable *create_hash_table(int size);
void free_hash_table(hashtable *ht);
hashelem *findhash(const char *name, hashtable *ht);
hashelem *puthash(const char *name, hashtable *ht);
hashtable *copy_hash_table(hashtable *ht);


/*************************** lpkit.h ********************************/

/*
  Main header file of the LP_SOLVE toolkit.

  Original by Jeroen Dirks, 21-2-95
  Maintained by Michel Berkelaar

  include this file in your program and link with liblps.a
*/

/* #include <stdlib.h> */
/* #include <stdio.h> */
#include <math.h>
/* #include "hash.h" */

#ifndef NULL
#define NULL    0
#endif

#define FALSE   0
#define TRUE    1

#define DEFNUMINV 50
#define INITIAL_MAT_SIZE 10000

/* solve status values */
#define OPTIMAL         0
#define MILP_FAIL       1
#define INFEASIBLE      2
#define UNBOUNDED       3
#define FAILURE         4
#define RUNNING         5

/* lag_solve extra status values */
#define FEAS_FOUND      6
#define NO_FEAS_FOUND   7
#define BREAK_BB        8

#define FIRST_NI        0
#define RAND_NI         1

#define LE      0
#define EQ      1
#define GE      2
#define OF      3

#define my_abs(x)       ((x) < 0 ? -(x) : (x))
#define my_min(x, y)    ((x) < (y) ? (x) : (y))
#define my_max(x, y)    ((x) > (y) ? (x) : (y))

#define MAX_WARN_COUNT 20

#ifdef CHECK
#define my_round(val, eps) { \
        REAL absv; \
        absv = ((val) < 0 ? -(val) : (val)); \
        if(absv < (eps)) \
          val = 0; \
        if(Warn_count < MAX_WARN_COUNT) \
          { \
            if(absv > 0.5 * (eps) && absv < 2 * (eps)) \
              { \
                Warn_count++; \
                fprintf(stderr, \
                        "Warning Value close to epsilon V: %e E: %e\n", \
                        (double)absv, (double)(eps)); \
                if(Warn_count == MAX_WARN_COUNT) \
                  { \
                    fprintf(stderr, \
                            "*** Surpressing further rounding warnings\n"); \
                  } \
              } \
          } \
}

#else
#define my_round(val,eps) if (((val) < 0 ? -(val) : (val)) < (eps)) val = 0;
#endif


#define DEF_INFINITE  1e24 /* limit for dynamic range */
#define DEF_EPSB      5.01e-7 /* for rounding RHS values to 0 determine
                                 infeasibility basis */
#define DEF_EPSEL     1e-8 /* for rounding other values (vectors) to 0 */
#define DEF_EPSD      1e-6 /* for rounding reduced costs to zero */
#define DEF_EPSILON   1e-3 /* to determine if a float value is integer */

#define PREJ          1e-3  /* pivot reject (try others first) */

#ifndef REAL /* to allow -DREAL=<float type> while compiling */
#define REAL double
#endif

#define ETA_START_SIZE 10000 /* start size of array Eta. Realloced if needed */
#define FNAMLEN 64
#define NAMELEN 25
#define MAXSTRL (NAMELEN-1)
#define STD_ROW_NAME_PREFIX "r_"

#define CALLOC(ptr, nr)\
  if(!(ptr = calloc((size_t)(nr), sizeof(*ptr))) && nr) {\
    fprintf(stderr, "calloc of %d bytes failed on line %d of file %s\n",\
            nr * sizeof(*ptr), __LINE__, __FILE__);\
    exit(EXIT_FAILURE);\
  }

#define MALLOC(ptr, nr)\
  if(!(ptr = malloc((size_t)((nr) * sizeof(*ptr)))) && nr) {\
    fprintf(stderr, "malloc of %d bytes failed on line %d of file %s\n",\
            nr * sizeof(*ptr), __LINE__, __FILE__);\
    exit(EXIT_FAILURE);\
  }

#define REALLOC(ptr, nr)\
  if(!(ptr = realloc(ptr, (size_t)((nr) * sizeof(*ptr)))) && nr) {\
    fprintf(stderr, "realloc of %d bytes failed on line %d of file %s\n",\
            nr * sizeof(*ptr), __LINE__, __FILE__);\
    exit(EXIT_FAILURE);\
  }

#define MALLOCCPY(nptr, optr, nr)\
  {MALLOC(nptr, nr); memcpy(nptr, optr, (size_t)((nr) * sizeof(*optr)));}

#define MEMCPY(nptr, optr, nr)\
  memcpy(nptr, optr, (size_t)((nr) * sizeof(*optr)));

typedef char nstring[NAMELEN];

typedef struct _matrec
{
  int row_nr;
  REAL value;
} matrec;

typedef struct _column
{
  int            row;
  REAL           value;
  struct _column *next ;
} column;

typedef struct _constraint_name
{
  char                    name[NAMELEN];
  int                     row;
  struct _constraint_name *next;
} constraint_name;

typedef struct _bound
{
  REAL          upbo;
  REAL          lowbo;
} bound;

typedef struct _tmp_store_struct
{
  nstring name;
  int     row;
  REAL    value;
  REAL    rhs_value;
  short   relat;
} tmp_store_struct;

typedef struct _rside /* contains relational operator and rhs value */
{
  REAL          value;
  struct _rside *next;
  short         relat;
} rside;


/* fields indicated with ## may be modified directly */
/* pointers will have their array size in the comments */

typedef struct _lprec
{
  nstring   lp_name;            /* the name of the lp */

  short     verbose;            /* ## Verbose flag */
  short     print_duals;        /* ## PrintDuals flag for PrintSolution */
  short     print_sol;          /* ## used in lp_solve */
  short     debug;              /* ## Print B&B information */
  short     print_at_invert;    /* ## Print information at every reinversion */
  short     trace;              /* ## Print information on pivot selection */
  short     anti_degen;         /* ## Do perturbations */
  short     do_presolve;        /* perform matrix presolving */

  int       rows;               /* Nr of constraint rows in the problem */
  int       rows_alloc;         /* The allocated memory for Rows sized data */
  int       columns;            /* The number of columns (= variables) */
  int       columns_alloc;
  int       sum;                /* The size of the variables + the slacks */
  int       sum_alloc;

  short     names_used;         /* Flag to indicate if names for rows and
                                   columns are used */
  nstring   *row_name;          /* rows_alloc+1 */
  nstring   *col_name;          /* columns_alloc+1 */

 /* Row[0] of the sparce matrix is the objective function */

  int       non_zeros;          /* The number of elements in the sparce matrix*/
  int       mat_alloc;          /* The allocated size for matrix sized
                                   structures */
  matrec    *mat;               /* mat_alloc :The sparse matrix */
  int       *col_end;           /* columns_alloc+1 :Cend[i] is the index of the
                                   first element after column i.
                                   column[i] is stored in elements
                                   col_end[i-1] to col_end[i]-1 */
  int       *col_no;            /* mat_alloc :From Row 1 on, col_no contains the
                                   column nr. of the
                                   nonzero elements, row by row */
  short     row_end_valid;      /* true if row_end & col_no are valid */
  int       *row_end;           /* rows_alloc+1 :row_end[i] is the index of the
                                   first element in Colno after row i */
  REAL      *orig_rh;           /* rows_alloc+1 :The RHS after scaling & sign
                                  changing, but before `Bound transformation' */
  REAL      *rh;                /* rows_alloc+1 :As orig_rh, but after Bound
                                   transformation */
  REAL      *rhs;               /* rows_alloc+1 :The RHS of the current
                                   simplex tableau */
  short     *must_be_int;       /* sum_alloc+1 :TRUE if variable must be
                                   Integer */
  REAL      *orig_upbo;         /* sum_alloc+1 :Bound before transformations */
  REAL      *orig_lowbo;        /*  "       "                   */
  REAL      *upbo;              /*  " " :Upper bound after transformation &
                                   B&B work */
  REAL      *lowbo;             /*  "       "  :Lower bound after transformation
                                   & B&B work */

  short     basis_valid;        /* TRUE is the basis is still valid */
  int       *bas;               /* rows_alloc+1 :The basis column list */
  short     *basis;             /* sum_alloc+1 : basis[i] is TRUE if the column
                                   is in the basis */
  short     *lower;             /*  "       "  :TRUE is the variable is at its
                                   lower bound (or in the basis), it is FALSE
                                   if the variable is at its upper bound */

  short     eta_valid;          /* TRUE if current Eta structures are valid */
  int       eta_alloc;          /* The allocated memory for Eta */
  int       eta_size;           /* The number of Eta columns */
  int       num_inv;            /* The number of real pivots */
  int       max_num_inv;        /* ## The number of real pivots between
                                   reinversions */
  REAL      *eta_value;         /* eta_alloc :The Structure containing the
                                   values of Eta */
  int       *eta_row_nr;         /*  "     "  :The Structure containing the Row
                                   indexes of Eta */
  int       *eta_col_end;       /* rows_alloc + MaxNumInv : eta_col_end[i] is
                                   the start index of the next Eta column */

  short     bb_rule;            /* what rule for selecting B&B variables */

  short     break_at_int;       /* TRUE if stop at first integer better than
                                   break_value */
  REAL      break_value;

  REAL      obj_bound;          /* ## Objective function bound for speedup of
                                   B&B */
  int       iter;               /* The number of iterations in the simplex
                                   solver (LP) */
  int       total_iter;         /* The total number of iterations (B&B)
                                   (ILP) */
  int       max_level;          /* The Deepest B&B level of the last solution */
  int       total_nodes;        /* total number of nodes processed in b&b */
  REAL      *solution;          /* sum_alloc+1 :The Solution of the last LP,
                                   0 = The Optimal Value,
                                   1..rows The Slacks,
                                   rows+1..sum The Variables */
  REAL      *best_solution;     /*  "       "  :The Best 'Integer' Solution */
  REAL      *duals;             /* rows_alloc+1 :The dual variables of the
                                   last LP */

  short     maximise;           /* TRUE if the goal is to maximise the
                                   objective function */
  short     floor_first;        /* TRUE if B&B does floor bound first */
  short     *ch_sign;           /* rows_alloc+1 :TRUE if the Row in the matrix
                                   has changed sign
                                   (a`x > b, x>=0) is translated to
                                   s + -a`x = -b with x>=0, s>=0) */

  short     scaling_used;       /* TRUE if scaling is used */
  short     columns_scaled;     /* TRUE is the columns are scaled too, Only use
                                   if all variables are non-integer */
  REAL      *scale;             /* sum_alloc+1:0..Rows the scaling of the Rows,
                                   Rows+1..Sum the scaling of the columns */

  int       nr_lagrange;        /* Nr. of Langrangian relaxation constraints */
  REAL      **lag_row;          /* NumLagrange, columns+1:Pointer to pointer of
                                   rows */
  REAL      *lag_rhs;           /* NumLagrange :Pointer to pointer of Rhs */
  REAL      *lambda;            /* NumLagrange :Lambda Values */
  short     *lag_con_type;      /* NumLagrange :TRUE if constraint type EQ */
  REAL      lag_bound;          /* the lagrangian lower bound */

  short     valid;              /* Has this lp pased the 'test' */
  REAL      infinite;           /* ## numerical stuff */
  REAL      epsilon;            /* ## */
  REAL      epsb;               /* ## */
  REAL      epsd;               /* ## */
  REAL      epsel;              /* ## */
  hashtable *rowname_hashtab;   /* hash table to store row names */
  hashtable *colname_hashtab;   /* hash table to store column names */
} lprec;



/* function interface for the user */

lprec *make_lp(int rows, int columns);
/* create and initialise a lprec structure
   defaults:
   Empty (Rows * Columns) matrix,
   Minimise the objective function
   constraints all type <=
   Upperbounds all Infinite
   no integer variables
   floor first in B&B
   no scaling
   default basis */

lprec *read_lp_file(FILE *input, short verbose, nstring lp_name);
/* create and read an .lp file from input (input must be open) */

void delete_lp(lprec *lp);
/* Remove problem from memory */

lprec *copy_lp(lprec *lp);
/* copy a lp structure */

void set_mat(lprec *lp, int row, int column, REAL value);
/* fill in element (Row,Column) of the matrix
   Row in [0..Rows] and Column in [1..Columns] */

void set_obj_fn(lprec *lp, REAL *row);
/* set the objective function (Row 0) of the matrix */
void str_set_obj_fn(lprec *lp, char *row);
/* The same, but with string input */

void add_constraint(lprec *lp, REAL *row, short constr_type, REAL rh);
/* Add a constraint to the problem,
   row is the constraint row,
   rh is the right hand side,
   constr_type is the type of constraint (LE (<=), GE(>=), EQ(=)) */
void str_add_constraint(lprec *lp, char *row_string ,short constr_type, REAL rh);
/* The same, but with string input */

void del_constraint(lprec *lp,int del_row);
/* Remove constrain nr del_row from the problem */

void add_lag_con(lprec *lp, REAL *row, short con_type, REAL rhs);
/* add a Lagrangian constraint of form Row' x contype Rhs */
void str_add_lag_con(lprec *lp, char *row, short con_type, REAL rhs);
/* The same, but with string input */

void add_column(lprec *lp, REAL *column);
/* Add a Column to the problem */
void str_add_column(lprec *lp, char *col_string);
/* The same, but with string input */

void del_column(lprec *lp, int column);
/* Delete a column */

void set_upbo(lprec *lp, int column, REAL value);
/* Set the upperbound of a variable */

void set_lowbo(lprec *lp, int column, REAL value);
/* Set the lowerbound of a variable */

void set_int(lprec *lp, int column, short must_be_int);
/* Set the type of variable, if must_be_int = TRUE then the variable must be integer */

void set_rh(lprec *lp, int row, REAL value);
/* Set the right hand side of a constraint row */

void set_rh_vec(lprec *lp, REAL *rh);
/* Set the right hand side vector */
void str_set_rh_vec(lprec *lp, char *rh_string);
/* The same, but with string input */

void set_maxim(lprec *lp);
/* maximise the objective function */

void set_minim(lprec *lp);
/* minimise the objective function */

void set_constr_type(lprec *lp, int row, short con_type);
/* Set the type of constraint in row Row (LE, GE, EQ) */

void set_row_name(lprec *lp, int row, nstring new_name);
/* Set the name of a constraint row, make sure that the name has < 25 characters */

void set_col_name(lprec *lp, int column, nstring new_name);
/* Set the name of a varaible column, make sure that the name has < 25 characters */

void auto_scale(lprec *lp);
/* Automatic scaling of the problem */

void unscale(lprec *lp);
/* Remove all scaling from the problem */

int solve(lprec *lp);
/* Solve the problem */

int lag_solve(lprec *lp, REAL start_bound, int num_iter, short verbose);
/* Do NumIter iterations with Lagrangian relaxation constraints */

void reset_basis(lprec *lp);
/* Reset the basis of a problem, can be usefull in case of degeneracy - JD */

REAL mat_elm(lprec *lp, int row, int column);
/* get a single element from the matrix */

void get_row(lprec *lp, int row_nr, REAL *row);
/* fill row with the row row_nr from the problem */

void get_column(lprec *lp, int col_nr, REAL *column);
/* fill column with the column col_nr from the problem */

void get_reduced_costs(lprec *lp, REAL *rc);
/* get the reduced costs vector */

short is_feasible(lprec *lp, REAL *values);
/* returns TRUE if the vector in values is a feasible solution to the lp */

short column_in_lp(lprec *lp, REAL *column);
/* returns TRUE if column is already present in lp. (Does not look at bounds
   and types, only looks at matrix values */

lprec *read_mps(FILE *input, short verbose);
/* read a MPS file */

void write_MPS(lprec *lp, FILE *output);
/* write a MPS file to output */

void write_LP(lprec *lp, FILE *output);
/* write a LP file to output */

void print_lp(lprec *lp);
/* Print the current problem, only usefull in very small (test) problems.
  Shows the effect of scaling */

void print_solution(lprec *lp);
/* Print the solution to stdout */

void print_duals(lprec *lp);
/* Print the dual variables of the solution */

void print_scales(lprec *lp);
/* If scaling is used, print the scaling factors */




/* functions used internaly by the lp toolkit */
void error(char *format, ...);
void inc_mat_space(lprec *lp, int max_extra);
void inc_row_space(lprec *lp);
void inc_col_space(lprec *lp);
void unscale_columns(lprec *lp);
void btran(lprec *lp, REAL *row);
void invert(lprec *lp);
void presolve(lprec *lp);


/* define yyparse() to make compilers happy. There should be some system
   include file for this */
int yyparse(void);

/************************** lpglob.h ********************************/

/* #include <stdio.h> */

/* Globals */
extern int     Level;
extern int     Warn_count;

extern REAL    Trej;
extern REAL    Extrad;

extern short just_inverted;
extern short status;
extern short do_iter;
extern short do_invert;


/* Globals for parser */
extern int     Rows;
extern int     Columns;
extern int     Sum;
extern int     Non_zeros;

extern FILE       *yyin;
extern FILE       *lpfilename;
extern short      Maximise;
extern short      *relat;
extern int        Verbose;
extern int        yylineno;
extern int        yyleng;
extern int        Lin_term_count;
extern int        Sign;
extern constraint_name *First_constraint_name;
/* I hate #ifdefs, but there seems to be no "standard" way to do this */
#if defined(__hpux) || defined(__apollo) || defined(_AIX) || defined(_OSF_SOURCE)
/* for HP/UX, Apollo, AIX, DEC OSF  */
extern unsigned char       yytext[];
#else
/* For other computers */
extern char    yytext[];
#endif

extern rside      *First_rside;
extern short      Ignore_decl;

extern tmp_store_struct tmp_store;

/*************************** debug.h ********************************/

/* prototypes for debug printing by other files */

void debug_print(lprec *lp, char *format, ...);
void debug_print_solution(lprec *lp);
void debug_print_bounds(lprec *lp, REAL *upbo, REAL *lowbo);

/*************** End of headers for lp_solve *********************************/



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
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-ns")) {
      argOffset++;
      argOffsetChanged = 1;
      noStatesMode = 1;
    }
    if (argc - argOffset > 1 &&
        (!strcmp(argv[1 + argOffset], "-q")
        || !strcmp(argv[1 + argOffset], "-qs"))) {
      if (!strcmp(argv[1 + argOffset], "-qs")) printStateEquation = 1;
      argOffset++;
      argOffsetChanged = 1;
      printEquation = 1;
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

    /* states.c - get mode */
    if (argc - argOffset > 1 && !strcmp(argv[1 + argOffset], "-m")) {
      if (argc - argOffset > 2) {
        if (!strcmp(argv[2 + argOffset], "d")){
          stateMode = DISPERSION_FREE;
        } else if (!strcmp(argv[2 + argOffset], "s")){
          stateMode = STRONG;
        } else if (!strcmp(argv[2 + argOffset], "f")){
          stateMode = FULL;
        } else {
          print2(
"?Expected -m mode to be d (disp.-free), s (strong), or f (full)\n");
          return 0;
        }
      } else {
        print2("?No argument after -m.\n");
        return 0;
      }
      argOffset += 2;
      argOffsetChanged = 1;
    }

  }

  /* states.c - There is no default state mode */
  if (stateMode == BAD_STATE_MODE) {
    print2(
"?You must specify a state mode with -m <mode> where <mode> is\n");
    print2(
"        d (disp.-free), s (strong), or f (full)\n");
    exit(0);
  }
  if (stateMode != STRONG && printStateEquation) {
    print2("?The -qs option can only be used with -m s (strong)\n");
    exit(0);
  }
  if (noStatesMode && (showAllFailures || showVisits || printEquation)) {
    print2("?The -ns option cannot be used with -f, -v, -q, or -qs\n");
    exit(0);
  }

  /* states.c */
  if (argc - argOffset != 1) {
    print2("?You have specified unknown options.\n");
    exit(0);
  }

  init(); /* One-time initialization */

  if (printLatticeFlag) {
    /* Print lattice for user */
    i = 0;
    if (fp != NULL) i = i + 2;
    if (fplog != NULL) i = i + 2;
    if (argc != 3 + i) {
      print2(
          "?Only the -i and -o (or --o) options may be used with -p\n");
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

  /****** states.c ******/
  if (1) goto skip_equation;

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

  /****** states.c ********/
 skip_equation:

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
goto skip_to_states_help;  /* states.c */
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

/* Start of states.c */
skip_to_states_help:
print2("\n");
print2("states.c - State Tester for Greechie Diagrams\n");
print2(
   "Copyright (C) 2000 GPL Norman D. Megill <nm@alum.mit.edu> Version %s\n",
   VERSION);
print2("Usage: states -m <mode> [options]\n");
print2(
"           -m <mode> - (required) <mode> is one of d (dispersion-free),\n");
print2(
"                       s (strong), or f (full)\n");
print2("         options:\n");
print2(
"           -a - test all lattices (don't stop after first counterexample)\n");
print2(
"           -n <integer> - test only the program's <integer>th lattice\n");
print2(
"           -s <integer> - start at the program's <integer>th lattice\n");
print2(
"           -e <integer> - end at the program's <integer>th lattice\n");
print2(
"           -g - categorize Greechie diagrams (legs, pass/fail, etc.)\n");
print2("           -l - skip (ignore) Greechie diagrams with legs\n");
print2("           -v - show lp_solve problem when counterexample found\n");
print2("           -f - show all non-state counterexamples\n");
print2(
"           -ns - scan for lattices with no states (see notes next page)\n");
print2(
"           -q (-qs) - print (state) equation that fails in the lattice\n");
print2(
"           -i <file> - use Greechie lattices from <file> (vs. built-in)\n");
print2(
"           -o (--o) <file> - write (append) output to <file> as well as screen\n");
print2(
"       states -p <integer> - print the program's <integer>th lattice\n");
print2(
"           (you may use the -i and -o [or --o] options with -p)\n");
print2("       states --help (or no argument) - print this message\n");
print2("Notes: Only one of -a and -n should be used.  -s and -e are applied\n");
print2("before any others.\n");
linput(NULL,"Press Enter to continue, q to quit...",&a);
if (toupper(a[0]) == 'Q') goto returnPoint;
print2("\n");
print2("This program checks built-in lattices (hard-coded into the\n");
print2("function 'initLattice') for the existence of a specified set of\n");
print2("states.\n");
print2("\n");
print2("The output is of the form \"<name> <result>\" where <name> is\n");
print2("the lattice name.  When there is no specified state, the diagram\n");
print2("atom pair (' means complement) without a state is shown.\n");
print2("\n");
print2("To compile for Unix:  Use 'gcc states.c -o states'.  The\n");
print2("entire program is contained in the single file states.c\n");
print2("\n");
print2("To run:  Type, at the Unix or DOS prompt, \'states <options>\' \n");
print2("with at least one option.  Example:\n");
print2("\'states -a' tests the built-in lattices.\n");
print2("\n");
print2("Note on -ns option: do not use with -q, -qs, -f, and -v options.\n");
print2("The modes for -ns mean: -m d, no 0/1 states; -m f, no states;\n");
print2("-m s, no state=1 on any node.\n");
print2("\n");
print2("\n");
print2("\n");
print2("\n");
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
/* End of states.c */

returnPoint:
let(&a, ""); /* Deallocate string */
return;
} /* printhelp() */


void testAllLattices(void)
{
  long latticeCase = 0;
  char result; /* states.c */
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

    /* states.c */
    result = stateTest();
    if (noStatesMode) {
      /* Looking for no states (-ns option) */
      if (result == 0) {
        switch (stateMode) {
          case DISPERSION_FREE:
            print2("%s has at least one {0,1} state\n",
                latticeName);
            break;
          case STRONG:
              print2("%s has at least one state = 1\n", latticeName);
            break;
          case FULL:
              print2("%s has at least one state\n", latticeName);
            break;
        }
      } else {
        switch (stateMode) {
          case DISPERSION_FREE:
            print2("%s has no {0,1} states\n",
                latticeName);
            break;
          case STRONG:
              print2("%s has no state = 1\n", latticeName);
            break;
          case FULL:
              print2("%s has no states\n", latticeName);
            break;
        }
      }
    } else {
      /* Normal mode (not -ns option) */
      if (result == 1 || result == 2) {
        if (result == 1) {
          if (!showAllFailures) {
            /* (If showAllFailure, counterexample was printed already by
               stateTest()) */
            switch (stateMode) {
              case DISPERSION_FREE:
                print2("%s There is no {0,1} state m s.t. %s\n", latticeName,
                    failingAssignment);
                break;
              case STRONG:
              case FULL:
                print2("%s There is no state m s.t. %s\n", latticeName,
                    failingAssignment);
                break;
            }
          }
        } else {
          /* Infeasible solution */
          switch (stateMode) {
            case DISPERSION_FREE:
              print2("%s The lattice has no {0,1} states at all!\n",
                  latticeName);
              break;
            case STRONG:
            case FULL:
              print2("%s The lattice has no states at all!\n",
                  latticeName);
              break;
          }
        } /* end if (result == 1) else */
      } else {
        switch (stateMode) {
          case DISPERSION_FREE:
            print2("%s has a full set of dispersion-free ({0,1}) states\n",
                latticeName);
            break;
          case STRONG:
              print2("%s has a strong set of states\n", latticeName);
            break;
          case FULL:
              print2("%s has a full set of states\n", latticeName);
            break;
        }
      } /* end if (result == 1 || result == 2) else */
    } /* end if (noStatesMode) else */
    if (result == 0) {
      if (showGreechie) print2("%s passed: %s\n", latticeName, greechieStmt);
    } else {
      if (showGreechie) print2("%s failed: %s\n", latticeName, greechieStmt);
      if (userArg == 0) break; /* Default: stop on first non-state */
    }

    if (1) goto xxxx;  /* states.c */
    if (test() == TRUE_CONST) {
      print2("Passed %s\n", latticeName);
      if (showGreechie) print2("%s passed: %s\n", latticeName, greechieStmt);
    } else {
      if (!showAllFailures) {
        /* (If showAllFaliure, failure was printed already by test()) */
        print2("FAILED %s at %s\n", latticeName, failingAssignment);
      }
      if (showGreechie) print2("%s failed: %s\n", latticeName, greechieStmt);
      if (userArg == 0) break; /* Default: stop on first failure */
    }
  xxxx:  /* states.c */

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
  let(&str1, cat(LOGIC_BIN_CONNECTIVES, chr(NEG_OPER),
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
  long atom, atom2, msize, legs, i, j, k, m, n;
  vstring glattice1 = "";
  vstring str1 = "";
  vstring str2 = "";
  /*** These are now global for states.c program
  long atoms;
  long block[MAX_BLOCKS + 1][MAX_BLOCK_SIZE + 1];
  long blockSize[MAX_BLOCKS + 1];
  long block4Offset[MAX_BLOCKS + 1];
  long blocks;
  ***/

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


/* states.c */
/* Returns 0 if there is a set of (d,s,f) states, 1 if there is no
   set of (d,s,f) states, 2 if there are no states at all.  The global
   variable failingAssignment
   has the first pair of Greechie atoms for which there is no (d,s,f)
   state. */
char stateTest(void)
{
  long i, j, k, l;
  vstring atom1 = "";
  vstring atom2 = "";
  vstring polWff = "";
  vstring printStr = "";
  vstring stmtStr = "";
  unsigned char e;
  long result;
  char retVal;
  long nodes1; /* Look at only nodes for atoms or complements */
  long lpCols; /* Number of LP variables */
  long lpRows; /* Number of LP constraints */
  long lpRow; /* Current row */

  /* For lp_solve */
  lprec *lp1;

  if (nodes != (nodes / 2) * 2) bug(1001); /* Sanity check - nodes must be
      even */

  /* This will be the same as nodes if all blocks are 3-atoms */
  nodes1 = (atoms * 2) + 2;

  j = 0; /* Prevent compiler warning */
  retVal = 0; /* 0 means a (d,s,or f) set of states was found */
  if (noStatesMode) retVal = 1; /* 1 means no states were found */
  for (i = 1; i <= nodes1; i++) {
    if (i == 1 || i == (nodes1 / 2) + 1) continue; /* Skip 0 and 1 */

    /* Get atom name for user information */
    if (i < (nodes1 / 2) + 1) {
      /* Get corresponding atom */
      let(&atom1, chr(ATOM_MAP[i - 2]));
    } else {
      /* Get corresponding atom complement */
      let(&atom1, cat(chr(ATOM_MAP[i - 2 - nodes1 / 2]), "'", NULL));
    }

    for (j = 1; j <= nodes1; j++) {
      if (j == 1 || j == (nodes1 / 2) + 1 || j == i) continue;
          /* Skip 0, 1, and same node */

      /* Get atom name for user information */
      if (j < (nodes1 / 2) + 1) {
        /* Get corresponding atom */
        let(&atom2, chr(ATOM_MAP[j - 2]));
      } else {
        /* Get corresponding atom complement */
        let(&atom2, cat(chr(ATOM_MAP[j - 2 - nodes1 / 2]), "'", NULL));
      }

      /* Build expression (polish) to be evaluated: a < b */
      let(&polWff, cat("<", chr(nodeNames[i - 1]), chr(nodeNames[j - 1]),
          NULL));
      e = eval(polWff, 3 /* length */);
      if (e != TRUE_CONST && e != FALSE_CONST) bug(1002);
      if (e == TRUE_CONST) continue;

      /*** The < relation fails, so try to find a (d,s, or f) state for it ***/

      /*** Setup for lp_solve call ***/
      /* We start by creating a new problem with lpRows constraints
         and lpCols variables */
      lpCols = (nodes1 / 2) - 1; /* Start with # of atoms */
      lpRows = blocks;
      lpRow = 0;
      if (i > (nodes1 / 2) + 1) { /* Complement atom for i */
        lpCols++;
        lpRows++;
      }
      if (j > (nodes1 / 2) + 1) {  /* Complement atom for j */
        lpCols++;
        lpRows++;
      }
      if (stateMode == STRONG || stateMode == DISPERSION_FREE) {
        lpRows++; /* To set 1st node state to 1 */
      }
      lp1 = make_lp(lpRows, lpCols); /* Call to lp_solve */

      /* These structure members are never initialized (bug in
         lp_solve?) */
      lp1->anti_degen = FALSE; /* Call to lp_solve */
      lp1->do_presolve = FALSE; /* Call to lp_solve */

      /* Initialize the objective function */
      for (k = 1; k <= lpCols; k++) {
        set_mat(lp1, 0, k, 0); /* Call to lp_solve */
      }
      if (stateMode == STRONG || stateMode == DISPERSION_FREE) {
        /* Set the objective function to minimize the 2nd node j
           in the < comparison for d or s state - if the minimum of
           the 2nd node is 1, there is no d or s state */
        if (j < (nodes1 / 2) + 1) {
          set_mat(lp1, 0, j - 1, 1);  /* Call to lp_solve */ /* node j */
        } else {
          set_mat(lp1, 0, lpCols, 1);  /* Call to lp_solve */ /* compl of j */
        }

        if (showVisits) { /* For -v option */
          let(&printStr, cat("min: m", atom2, ";\n", NULL));
        }

      } else if (stateMode == FULL) {
        /* Set the objective function to minimize the node j state
           minus node i state - if not less than 0, there is no
           full state */
        if (i < (nodes1 / 2) + 1) {
          set_mat(lp1, 0, i - 1, -1);  /* Call to lp_solve */ /* node i */
        } else {
          set_mat(lp1, 0, nodes1 / 2, -1);  /* Call to lp_solve */ /* compl i */
        }
        if (j < (nodes1 / 2) + 1) {
          set_mat(lp1, 0, j - 1, 1);  /* Call to lp_solve */ /* node j */
        } else {
          set_mat(lp1, 0, lpCols, 1);  /* Call to lp_solve */ /* compl of j */
        }

        if (showVisits) { /* For -v option */
          let(&printStr, cat("min: m", atom2, " - m", atom1, ";\n", NULL));
        }

      } else {
        bug(1003);
      }

      if (stateMode == STRONG || stateMode == DISPERSION_FREE) {
        /* Set constraint on the 1st node to be 1 */
        lpRow++;
        for (k = 1; k <= lpCols; k++) {
          set_mat(lp1, lpRow, k, 0);
        }
        if (i > (nodes1 / 2) + 1) {
          /* Set constraint on complement of atom */
          set_mat(lp1, lpRow, nodes1 / 2, 1); /* Call to lp_solve */
        } else {
          /* Set constraint on atom */
          set_mat(lp1, lpRow, i - 1, 1); /* Call to lp_solve */
        }
        set_constr_type(lp1, lpRow, EQ); /* Call to lp_solve */
        /* 1st node should equal 1 */
        set_rh(lp1, lpRow, 1); /* Call to lp_solve */

        if (showVisits) { /* For -v option */
          let(&printStr, cat(printStr, "m", atom1, " = 1;\n", NULL));
        }

      }

      /* Set constraints on complement node for i if i not an atom */
      if (i > (nodes1 / 2) + 1) {
        lpRow++;
        for (k = 1; k <= lpCols; k++) {
          if (k == i - (nodes1 / 2) - 1   /* node i */
              || k == nodes1 / 2) { /* node i complement */
            set_mat(lp1, lpRow, k, 1);
          } else {
            set_mat(lp1, lpRow, k, 0);
          }
        }
        set_constr_type(lp1, lpRow, EQ); /* Call to lp_solve */
        /* i and i complement should add to 1 */
        set_rh(lp1, lpRow, 1); /* Call to lp_solve */

        if (showVisits) { /* For -v option */
          let(&printStr, cat(printStr, "m", left(atom1, 1), " + m", atom1,
              " = 1;\n", NULL));
        }

      }

      /* Set constraints on complement node for j if j not an atom */
      if (j > (nodes1 / 2) + 1) {
        lpRow++;
        for (k = 1; k <= lpCols; k++) {
          if (k == j - (nodes1 / 2) - 1   /* node j */
              || k == lpCols) { /* node j complement */
            set_mat(lp1, lpRow, k, 1);
          } else {
            set_mat(lp1, lpRow, k, 0);
          }
        }
        set_constr_type(lp1, lpRow, EQ); /* Call to lp_solve */
        /* j and j complement should add to 1 */
        set_rh(lp1, lpRow, 1); /* Call to lp_solve */

        if (showVisits) { /* For -v option */
          let(&printStr, cat(printStr, "m", left(atom2, 1), " + m", atom2,
              " = 1;\n", NULL));
        }

      }

      /* Set constraint for each block: atom states should add to 1 */
      for (k = 1; k <= blocks; k++) {
        lpRow++;
        for (l = 1; l <= lpCols; l++) {
          set_mat(lp1, lpRow, l, 0); /* Initialize */
        }
        /*
        if (blockSize[k] != 3) {
          print2("?Error: Only block size 3 is currently implemented\n");
          exit(0);
        }
        */
        for (l = 1; l <= blockSize[k]; l++) {
          set_mat(lp1, lpRow, block[k][l], 1);
        }
        set_constr_type(lp1, lpRow, EQ); /* Call to lp_solve */
        /* j and j complement should add to 1 */
        set_rh(lp1, lpRow, 1); /* Call to lp_solve */

        if (showVisits) { /* For -v option */
          let(&stmtStr, "");
          for (l = 1; l <= blockSize[k]; l++) {
            let(&stmtStr, cat(stmtStr, "m", chr(ATOM_MAP[block[k][l] - 1]),
                NULL));
            if (l < blockSize[k])
              let(&stmtStr, cat(stmtStr, " + ", NULL));
            else
              let(&stmtStr, cat(stmtStr, " = 1;\n", NULL));
          }
          let(&printStr, cat(printStr, stmtStr, NULL));
        }

      } /* next k */

      if (lpRow != lpRows) bug(1004);

      if (stateMode == DISPERSION_FREE) {
        /* Specify integers for dispersion-free states */
        for (k = 1; k < nodes1 / 2; k++) {
          /* Do only atoms; complements will take care of themselves */
          set_int(lp1, k, TRUE); /* Call to lp_solve */
        }

        if (showVisits) { /* For -v option */
          let(&stmtStr, "");
          for (k = 2; k <= nodes1 / 2; k++) {
            if (k > 2) let(&stmtStr, cat(stmtStr, ",", NULL));
            let(&stmtStr, cat(stmtStr, "m", chr(ATOM_MAP[k - 2]), NULL));
          }
          let(&printStr, cat(printStr, "int ", stmtStr, ";\n", NULL));
        }

      }

      /* If we want to maximize the objective function use
         set_maxim(lp1) but do this JUST BEFORE solving.  Default
         is to minimize. */
      /* set_maxim(lp1); */

      /* For debugging: print the problem */
      /*print2("Atom 1 = %s, atom 2 = %s\n", atom1, atom2);*/
      /*print_lp(lp1);*/ /* Call to lp_solve */

      /* Solve the problem */
      result = solve(lp1); /* Call to lp_solve */

      /* Handle -ns option */
      if (noStatesMode) {
        if (result == 0) {
          /* A state was found; skip further scanning */
          retVal = 0;
          /* Free up memory */
          delete_lp(lp1); /* Call to lp_solve */
          goto return_point;
        } else {
          /* We're only interested in infeasible; others should never
             happen (run without -ns option to see result meaning) */
          if (result != 2) bug(1014);
          /* No state was found. */
          if (stateMode == STRONG || stateMode == DISPERSION_FREE) {
            /* Check next i node; skip j scan */
            /* Free up memory */
            delete_lp(lp1); /* Call to lp_solve */
            goto continue_i_point; /* Skip rest of atom2 scan */
          } else {
            /* If -m f, we're done (there are no special
               (constraints on nodes) */
            /* The lattice has no states; skip further scanning */
            retVal = 1;
            /* Free up memory */
            delete_lp(lp1); /* Call to lp_solve */
            goto return_point;
          }
        }
      }

      if (result != 0) {
        if (result == 2) {
          /* INFEASIBLE solution - no states at all */

          if (showVisits && !printEquation) { /* For -v option */
            print2("\nProblem statement for lp_solve:\n");
            print2("%s\n", printStr);
          }

          if (stateMode == STRONG || stateMode == DISPERSION_FREE) {
            /* If the lattice has no "large" set of states (i.e. no m s.t.
               m(atom1)=1) -- which could also be the case for no states at
               all -- the solutions will be infeasible for this atom1.
               Don't call it a "no states at all" because there might be
               another atom1 with m(atom1)=1. */
            /* (Future - put in "large" states detection) */
            if (stateMode == STRONG) {
              let(&failingAssignment, cat("m(", atom1,
                  ") = 1 (or no states at all; use -m f to find out)", NULL));
            } else {
              let(&failingAssignment, cat("m(", atom1,
                  ") = 1 (or no {0,1} states at all; use -f to find out)", NULL));
            }
            /* showAllFailures is set by -f option */
            if (showAllFailures) {
              if (printEquation) statePrntEqn(i, j); /* Print all possible eqns */
              if (stateMode == STRONG) {
                print2("%s There is no state m s.t. %s\n", latticeName,
                    failingAssignment);
              } else {
                print2("%s There is no {0,1} state m s.t. %s\n", latticeName,
                    failingAssignment);
              }
              retVal = 1;
              if (printEquation) {
                goto continue_j_point; /* Scan all atom2s to get all equations */
              } else {
                /* Free up memory */
                delete_lp(lp1); /* Call to lp_solve */
                goto continue_i_point; /* Skip rest of atom2 scan */
              }
            } else {
              /* Exit loop on first counterexample */
              retVal = 1;
              /* Free up memory */
              delete_lp(lp1); /* Call to lp_solve */
              goto return_point;
            }
          } else {
            retVal = 2; /* No states at all */
            /* Free up memory */
            delete_lp(lp1); /* Call to lp_solve */
            goto return_point;
          }
        }
        /* Some other problem */
        print2("?No solution: result = %ld\n",
            result);
        print2(
          "Result values mean: OPTIMAL = 0, MILP_FAIL = 1, INFEASIBLE = 2,\n");
        print2("    UNBOUNDED = 3, FAILURE = 4, RUNNING = 5\n");
        print2("Atom 1 = %s, atom 2 = %s\n", atom1, atom2);
        /*print_lp(lp1);*/
        exit(0);
      }

      /* For debugging: print the solution */
      /*print_solution(lp1);*/ /* Call to lp_solve */

      if (stateMode == DISPERSION_FREE || stateMode == STRONG) {
        /* Look at minimum of state(node2) when state(node1)=1 */
        if (lp1->best_solution[0] > 1.0001) {
          print2("?Rounding error too big\n");
          print2("%8g\n", (double)(lp1->best_solution[0] - 1.0));
          bug(1005);
        }
        if (lp1->best_solution[0] >= /*1.0*/ 0.9999) {
          /* A d or s set of states was not found */
          if (stateMode == STRONG) {
              let(&failingAssignment, cat("(m(", atom1, ") = 1 => m(",
                  atom2, ") = 1) => ",
                  atom1, " =< ", atom2, NULL));
          } else { /* DISPERSION_FREE */
              let(&failingAssignment, cat("m(", atom1, ") =< m(",
                  atom2, ") => ",
                  atom1, " =< ", atom2, NULL));
          }

          if (showVisits && !printEquation) { /* For -v option */
            print2("\nProblem statement for lp_solve:\n");
            print2("%s\n", printStr);
          }

          retVal = 1;

          /* showAllFailures is set by -f option */
          if (showAllFailures) {
            if (printEquation) statePrntEqn(i, j); /* Print all possible eqns */
            switch (stateMode) {
              case DISPERSION_FREE:
                print2("%s There is no {0,1} state m s.t. %s\n", latticeName,
                    failingAssignment);
                break;
              case STRONG:
                print2("%s There is no state m s.t. %s\n", latticeName,
                    failingAssignment);
                break;
            }
          } else {
            /* Exit loop on first counterexample */
            /* Free up memory */
            delete_lp(lp1); /* Call to lp_solve */
            goto return_point;
          }
        }
      }

      if (stateMode == FULL) {
        /* Look at minimum of state(node2)-state(node1) */
        if (lp1->best_solution[0] > 0.0001) {
          print2("?Rounding error too big\n");
          print2("%8g\n", (double)(lp1->best_solution[0]));
          if (showVisits && !printEquation) { /* For -v option */
            print2("\nProblem statement for lp_solve:\n");
            print2("%s\n", printStr);
            print2("Solution:\n");
            print_solution(lp1); /* Call to lp_solve */
          }
          bug(1006);
        }
        if (lp1->best_solution[0] >= -0.0001) {
          /* A full set of states was not found */
          let(&failingAssignment, cat("m(", atom1, ") =< m(",
              atom2, ") => ",
              atom1, " =< ", atom2, NULL));

          if (showVisits && !printEquation) { /* For -v option */
            print2("\nProblem statement for lp_solve:\n");
            print2("%s\n", printStr);
          }

          retVal = 1;

          /* showAllFailures is set by -f option */
          if (showAllFailures) {
            if (printEquation) statePrntEqn(i, j); /* Print all possible eqns */
            print2("%s There is no state m s.t. %s\n", latticeName,
                failingAssignment);
          } else {
            /* Exit loop on first counterexample */
            /* Free up memory */
            delete_lp(lp1); /* Call to lp_solve */
            goto return_point;
          }
        }
      }

     continue_j_point:
      /* Free up memory */
      delete_lp(lp1); /* Call to lp_solve */
    } /* next j */
   continue_i_point: ;
  } /* next i */

 return_point:
  if (retVal >= 1 && printEquation) {
    /* In the case of showAllFailures (-f option) and -q, the equations
       were printed as failures were found. */
    if (!showAllFailures) {
      statePrntEqn(i, j);
    }
    if (retVal > 1 && showAllFailures) {
      /* For no state at all, do all possible equations for -f option, since
         the main loop exited without showing all failures. */
      for (i = 1; i <= nodes1; i++) {
        if (i == 1 || i == (nodes1 / 2) + 1) continue; /* Skip 0 and 1 */
        for (j = 1; j <= nodes1; j++) {
          if (j == 1 || j == (nodes1 / 2) + 1 || j == i) continue;
              /* Skip 0, 1, and same node */
          /* Build expression (polish) to be evaluated: a < b */
          let(&polWff, cat("<", chr(nodeNames[i - 1]), chr(nodeNames[j - 1]),
              NULL));
          e = eval(polWff, 3 /* length */);
          if (e != TRUE_CONST && e != FALSE_CONST) bug(1007);
          if (e == TRUE_CONST) continue;
          /*** The < relation fails, so print eqn for (d,s, or f) state ***/
          print2("(For debugging) i = %ld j = %ld\n", i, j);
          statePrntEqn(i, j);
        }
      }
    }
  }

  /* Deallocate memory */
  let(&atom1, "");
  let(&atom2, "");
  let(&polWff, "");
  let(&printStr, "");
  let(&stmtStr, "");

  return retVal;
} /* stateTest */


/* Print the "raw" equation determined by a strong state failure */
void statePrntEqn(long node1, long node2)  /* states.c */
{

  long i, j, k, l, m, n, n1, n2, n3;
  vstring atom1 = "";
  vstring atom2 = "";
  vstring printStr = "";
  vstring stmtStr = "";
  /*unsigned char e;*/
  long result;
  char retVal;
  long nodes1; /* Look at only nodes for atoms or complements */
  long lpCols; /* Number of LP variables */
  long lpRows; /* Number of LP constraints */
  long lpRow; /* Current row */
  vstring enableConstraint = "";
  long constraint;
  long orthHyps;
  long orthHyp;
  long pass;
  vstring normalizedVarList = "";
  long varsUsed;
  vstring wfflhs = "";
  vstring wffrhs = "";
  vstring wffTmp = "";
  vstring latticeEqn = "";
  long latticeHyps;
  lprec *lp1; /* For lp_solve */

  long natom1, natom2;
  char tmpFlag;
  vstring suppressedAtoms = "";
  vstring stateEqAtoms = "";
  vstring stateEquation = "";
  vstring stateTerm = "";
  vstring lhAtomCount = "";
  vstring rhAtomCount = "";

  varsUsed = 0;
  latticeHyps = 0;

  /* This will be the same as nodes if all blocks are 3-atoms */
  nodes1 = (atoms * 2) + 2;

  retVal = 0; /* 0 means a (d,s,or f) set of states was found */

  /* These are the nodes that disprove the existence of (d,s,f) states */
  /* Use j and k so we can reuse some stateTest() code */
  i = node1;
  j = node2;

  /* Get atom name for user information */
  if (i < (nodes1 / 2) + 1) {
    /* Get corresponding atom */
    let(&atom1, chr(ATOM_MAP[i - 2]));
    natom1 = i - 1;
  } else {
    /* Get corresponding atom complement */
    let(&atom1, cat(chr(ATOM_MAP[i - 2 - nodes1 / 2]), "'", NULL));
    natom1 = i - 1 - nodes1 / 2;
  }

  /* Get atom name for user information */
  if (j < (nodes1 / 2) + 1) {
    /* Get corresponding atom */
    let(&atom2, chr(ATOM_MAP[j - 2]));
    natom2 = j - 1;
  } else {
    /* Get corresponding atom complement */
    let(&atom2, cat(chr(ATOM_MAP[j - 2 - nodes1 / 2]), "'", NULL));
    natom2 = j - 1 - nodes1 / 2;
  }

  /* Count the number of possible orthogonal hypotheses */
  orthHyps = 0;
  for (k = 1; k <= blocks; k++) {
    for (l = 1; l <= blockSize[k] - 1; l++) {
      for (m = l + 1; m <= blockSize[k]; m++) {
        orthHyps++;
      } /* next m */
    } /* next l */
  } /* next k */

  /* We assume atom1 is 1, so other atoms in its blocks will be 0 */
  /* Get atoms in blocks containing atom1 */
  for (k = 1; k <= blocks; k++) {
    tmpFlag = 0;
    for (l = 1; l <= blockSize[k]; l++) {
      if (block[k][l] == natom1) {
        tmpFlag = 1;
        break;
      }
    }
    if (tmpFlag) {
      for (l = 1; l <= blockSize[k]; l++) {
        if (block[k][l] != natom1
            && strchr(suppressedAtoms, block[k][l]) == NULL) {
          let(&suppressedAtoms, cat(suppressedAtoms, chr(block[k][l]), NULL));
        }
      } /* next l */
    }
  } /* next k */



  /* Start off by enabling all constraints */
  /* In the loop below for pass 2, we will disable them one by one to obtain
     a weakest possible hypothesis set */
  let(&enableConstraint, string(blocks + orthHyps, 'Y'));

  /* Initialize list of vars to all blanks, filled in as needed */
  let(&normalizedVarList, space(atoms));

  /* Pass 1: confirm (bug check) */
  /* Pass 2: delete unnecessary hypotheses */
  /* Pass 3: print equation */
  for (pass = 1; pass <= 3; pass++) {
    for (constraint = 1; constraint <= blocks + orthHyps; constraint++) {
      if (constraint > 1 && pass != 2) break;

      if (pass == 2) {
        /* Try disabling the constraint */
        enableConstraint[constraint - 1] = 'N';
      }

      /*** Setup for lp_solve call ***/
      /* We start by creating a new problem with lpRows constraints
         and lpCols variables */
      lpCols = (nodes1 / 2) - 1; /* Start with # of atoms */
      lpRows = blocks;
      lpRows = lpRows + orthHyps;
      lpRow = 0;
      if (i > (nodes1 / 2) + 1) { /* Complement atom for i */
        lpCols++;
        lpRows++;
      }
      if (j > (nodes1 / 2) + 1) {  /* Complement atom for j */
        lpCols++;
        lpRows++;
      }
      if (stateMode == STRONG || stateMode == DISPERSION_FREE) {
        lpRows++; /* To set 1st node state to 1 */
      }
      lp1 = make_lp(lpRows, lpCols); /* Call to lp_solve */

      /* These structure members are never initialized (bug in
         lp_solve?) */
      lp1->anti_degen = FALSE; /* Call to lp_solve */
      lp1->do_presolve = FALSE; /* Call to lp_solve */

      /* Initialize the objective function */
      for (k = 1; k <= lpCols; k++) {
        set_mat(lp1, 0, k, 0); /* Call to lp_solve */
      }
      if (stateMode == STRONG || stateMode == DISPERSION_FREE) {
        /* Set the objective function to minimize the 2nd node j
           in the < comparison for d or s state - if the minimum of
           the 2nd node is 1, there is no d or s state */
        if (j < (nodes1 / 2) + 1) {
          set_mat(lp1, 0, j - 1, 1);  /* Call to lp_solve */ /* node j */
        } else {
          set_mat(lp1, 0, lpCols, 1);  /* Call to lp_solve */ /* compl of j */
        }

        if (showVisits) { /* For -v option */
          let(&printStr, cat("min: m", atom2, ";\n", NULL));
        }

      } else if (stateMode == FULL) {
        /* Set the objective function to minimize the node j state
           minus node i state - if not less than 0, there is no
           full state */
        if (i < (nodes1 / 2) + 1) {
          set_mat(lp1, 0, i - 1, -1);  /* Call to lp_solve */ /* node i */
        } else {
          set_mat(lp1, 0, nodes1 / 2, -1);  /* Call to lp_solve */ /* compl i */
        }
        if (j < (nodes1 / 2) + 1) {
          set_mat(lp1, 0, j - 1, 1);  /* Call to lp_solve */ /* node j */
        } else {
          set_mat(lp1, 0, lpCols, 1);  /* Call to lp_solve */ /* compl of j */
        }

        if (showVisits) { /* For -v option */
          let(&printStr, cat("min: m", atom2, " - m", atom1, ";\n", NULL));
        }

      } else {
        bug(1008);
      }

      if (stateMode == STRONG || stateMode == DISPERSION_FREE) {
        /* Set constraint on the 1st node to be 1 */
        lpRow++;
        for (k = 1; k <= lpCols; k++) {
          set_mat(lp1, lpRow, k, 0);
        }
        if (i > (nodes1 / 2) + 1) {
          /* Set constraint on complement of atom */
          set_mat(lp1, lpRow, nodes1 / 2, 1); /* Call to lp_solve */
        } else {
          /* Set constraint on atom */
          set_mat(lp1, lpRow, i - 1, 1); /* Call to lp_solve */
        }
        set_constr_type(lp1, lpRow, EQ); /* Call to lp_solve */
        /* 1st node should equal 1 */
        set_rh(lp1, lpRow, 1); /* Call to lp_solve */

        if (showVisits) { /* For -v option */
          let(&printStr, cat(printStr, "m", atom1, " = 1;\n", NULL));
        }

      }

      /* 1s LHS var of final equation */
      if (pass == 3) {
        if (i > (nodes1 / 2) + 1) {
          k = i - (nodes1 / 2) - 1;  /* atom */
        } else {
          k = i - 1; /* atom */
        }
        if (normalizedVarList[k - 1] == ' ') {
          varsUsed++;
          if (varsUsed > strlen(VAR_LIST)) {
            print2("?More than %ld\n variables needed\n", varsUsed - 1);
            exit(0);
          }
          normalizedVarList[k - 1] = VAR_LIST[varsUsed - 1];
        }
        let(&wfflhs, chr(normalizedVarList[k - 1]));
        if (i > (nodes1 / 2) + 1)
          let(&wfflhs, cat("-", wfflhs, NULL));
      }

      /* Set constraints on complement node for i if i not an atom */
      if (i > (nodes1 / 2) + 1) {
        lpRow++;
        for (k = 1; k <= lpCols; k++) {
          if (k == i - (nodes1 / 2) - 1   /* node i */
              || k == nodes1 / 2) { /* node i complement */
            set_mat(lp1, lpRow, k, 1);
          } else {
            set_mat(lp1, lpRow, k, 0);
          }
        }
        set_constr_type(lp1, lpRow, EQ); /* Call to lp_solve */
        /* i and i complement should add to 1 */
        set_rh(lp1, lpRow, 1); /* Call to lp_solve */

        if (showVisits) { /* For -v option */
          let(&printStr, cat(printStr, "m", left(atom1, 1), " + m", atom1,
              " = 1;\n", NULL));
        }

      }

      /* RHS var of final equation */
      if (pass == 3) {
        if (j > (nodes1 / 2) + 1) {
          k = j - (nodes1 / 2) - 1;  /* atom */
        } else {
          k = j - 1; /* atom */
        }
        if (normalizedVarList[k - 1] == ' ') {
          varsUsed++;
          if (varsUsed > strlen(VAR_LIST)) {
            print2("?More than %ld\n variables needed\n", varsUsed - 1);
            exit(0);
          }
          normalizedVarList[k - 1] = VAR_LIST[varsUsed - 1];
        }
        let(&wffrhs, chr(normalizedVarList[k - 1]));
        if (j > (nodes1 / 2) + 1)
          let(&wffrhs, cat("-", wffrhs, NULL));
      }

      /* Set constraints on complement node for j if j not an atom */
      if (j > (nodes1 / 2) + 1) {
        lpRow++;
        for (k = 1; k <= lpCols; k++) {
          if (k == j - (nodes1 / 2) - 1   /* node j */
              || k == lpCols) { /* node j complement */
            set_mat(lp1, lpRow, k, 1);
          } else {
            set_mat(lp1, lpRow, k, 0);
          }
        }
        set_constr_type(lp1, lpRow, EQ); /* Call to lp_solve */
        /* j and j complement should add to 1 */
        set_rh(lp1, lpRow, 1); /* Call to lp_solve */

        if (showVisits) { /* For -v option */
          let(&printStr, cat(printStr, "m", left(atom2, 1), " + m", atom2,
              " = 1;\n", NULL));
        }

      }

      /* Set constraint for each block: atom states should add to 1 */
      for (k = 1; k <= blocks; k++) {
        lpRow++;
        for (l = 1; l <= lpCols; l++) {
          set_mat(lp1, lpRow, l, 0); /* Initialize */
        }
        /*
        if (blockSize[k] != 3) {
          print2("?Error: Only block size 3 is currently implemented\n");
          exit(0);
        }
        */
        for (l = 1; l <= blockSize[k]; l++) {
          set_mat(lp1, lpRow, block[k][l], 1);
        }
        if (enableConstraint[k - 1] == 'Y') {
          /* This is the normal constraint for the block */
          set_constr_type(lp1, lpRow, EQ); /* Call to lp_solve */
          /* atoms in block should add to 1 */
          set_rh(lp1, lpRow, 1); /* Call to lp_solve */
        } else {
          /* This is the disabled constraint for the block - just
             make the upper limit too large to ever be reached */
          set_constr_type(lp1, lpRow, LE); /* Call to lp_solve */
          if (!printStateEquation) {
            set_rh(lp1, lpRow, 100000); /* Call to lp_solve */
          } else {
            /* To get the right terms for the state equation, we
               must set the block sum to LE 1 from EQ 1.  I don't
               know why this seems to work; a theory must be developed
               to justify it later. */
            set_rh(lp1, lpRow, 1); /* Call to lp_solve */
          }
        }

        if (pass == 3) {
          if (enableConstraint[k - 1] == 'Y') {
            let(&wffTmp, "");
            for (l = 1; l <= blockSize[k]; l++) {
              if (normalizedVarList[block[k][l] - 1] == ' ') {
                varsUsed++;
                if (varsUsed > strlen(VAR_LIST)) {
                  print2("?More than %ld\n variables needed\n", varsUsed - 1);
                  exit(0);
                }
                normalizedVarList[block[k][l] - 1] = VAR_LIST[varsUsed - 1];
              }
              let(&wffTmp, cat(wffTmp, chr(normalizedVarList[block[k][l] - 1]),
                  NULL));
              if (l > 1)
                let(&wffTmp, cat("v", wffTmp, NULL));
            }
            let(&wfflhs, cat("^", wfflhs, wffTmp, NULL));
          }
        } /* if pass == 3 */

        if (showVisits) { /* For -v option */
          let(&stmtStr, "");
          for (l = 1; l <= blockSize[k]; l++) {
            let(&stmtStr, cat(stmtStr, "m", chr(ATOM_MAP[block[k][l] - 1]),
                NULL));
            if (l < blockSize[k])
              let(&stmtStr, cat(stmtStr, " + ", NULL));
            else
              if (enableConstraint[k - 1] == 'Y') {
                let(&stmtStr, cat(stmtStr, " = 1;\n", NULL));
              } else {
                if (!printStateEquation) {
                  let(&stmtStr, cat(stmtStr, " <= 100000;\n", NULL));
                } else {
                  /* To get the right terms for the state equation, we
                     must set the block sum to LE 1 from EQ 1.  I don't
                     know why this seems to work; a theory must be developed
                     to justify it later. */
                  let(&stmtStr, cat(stmtStr, " <= 1;\n", NULL));
                }
              }
          }
          let(&printStr, cat(printStr, stmtStr, NULL));
        }

      } /* next k */

      /* Set the constraint for each orthogonal pair */
      orthHyp = 0;
      for (k = 1; k <= blocks; k++) {
        for (l = 1; l <= blockSize[k] - 1; l++) {
          for (m = l + 1; m <= blockSize[k]; m++) {
            orthHyp++;
            lpRow++;
            for (n = 1; n <= lpCols; n++) {
              set_mat(lp1, lpRow, n, 0); /* Initialize */
            }
            set_mat(lp1, lpRow, block[k][l], 1);
            set_mat(lp1, lpRow, block[k][m], 1);

            /* If a block has an =1 constraint, we will also need
               all orthogonal hypotheses for it in the final equation,
               so override any constraint disabling here when that
               is the case */
            if (enableConstraint[k - 1] == 'Y') {
              enableConstraint[blocks - 1 + orthHyp] = 'Y';
            }
            /* Scan all earlier blocks that are enabled to see if
               hypothesis is redundant */
            for (n1 = 1; n1 < k; n1++) {
              if (enableConstraint[n1 -1] == 'Y') {
                for (n2 = 1; n2 <= blockSize[k] - 1; n2++) {
                  for (n3 = n2 + 1; n3 <= blockSize[n1]; n3++) {
                    if ((block[k][l] == block[n1][n2] &&
                         block[k][m] == block[n1][n3]) ||
                        (block[k][l] == block[n1][n3] &&
                         block[k][m] == block[n1][n2])) {
                      /* The hypothesis is redundant; disable it */
                      enableConstraint[blocks - 1 + orthHyp] = 'N';
                    }
                  }
                }
              }
            }

            if (enableConstraint[blocks - 1 + orthHyp] == 'Y') {
              /* This is the normal constraint for the block */
              set_constr_type(lp1, lpRow, LE); /* Call to lp_solve */
              /* j and j complement should add to 1 */
              set_rh(lp1, lpRow, 1); /* Call to lp_solve */
            } else {
              /* This is the disabled constraint for the orth pair - just
                 make the upper limit too large to ever be reached */
              set_constr_type(lp1, lpRow, LE); /* Call to lp_solve */
              /* j and j complement should add to 1 */
              set_rh(lp1, lpRow, 100000); /* Call to lp_solve */
            }

            if (pass == 3) {
              if (enableConstraint[blocks - 1 + orthHyp] == 'Y') {
                if (normalizedVarList[block[k][l] - 1] == ' ') {
                  varsUsed++;
                  if (varsUsed > strlen(VAR_LIST)) {
                    print2("?More than %ld\n variables needed\n", varsUsed - 1);
                    exit(0);
                  }
                  normalizedVarList[block[k][l] - 1] = VAR_LIST[varsUsed - 1];
                }
                if (normalizedVarList[block[k][m] - 1] == ' ') {
                  varsUsed++;
                  if (varsUsed > strlen(VAR_LIST)) {
                    print2("?More than %ld\n variables needed\n", varsUsed - 1);
                    exit(0);
                  }
                  normalizedVarList[block[k][m] - 1] = VAR_LIST[varsUsed - 1];
                }
                /* Put orthogonality hypotheses variables in alphabetical
                   order for easier reading */
                if (normalizedVarList[block[k][l] - 1] <
                    normalizedVarList[block[k][m] - 1]) /* alph order */
                  let(&wffTmp,
                      cat(chr(normalizedVarList[block[k][l] - 1]), "<-",
                      chr(normalizedVarList[block[k][m] - 1]), NULL));
                else
                  let(&wffTmp,
                      cat(chr(normalizedVarList[block[k][m] - 1]), "<-",
                      chr(normalizedVarList[block[k][l] - 1]), NULL));
                let(&latticeEqn, cat(latticeEqn, wffTmp, "\n", NULL));
                latticeHyps++;
              }
            } /* if pass == 3 */

            if (showVisits) { /* For -v option */
              let(&stmtStr, "");
              let(&stmtStr, cat(stmtStr, "m", chr(ATOM_MAP[block[k][l] - 1]),
                  NULL));
              let(&stmtStr, cat(stmtStr, " + ", NULL));
              let(&stmtStr, cat(stmtStr, "m", chr(ATOM_MAP[block[k][m] - 1]),
                  NULL));
              if (enableConstraint[blocks - 1 + orthHyp] == 'Y') {
                let(&stmtStr, cat(stmtStr, " <= 1;\n", NULL));
              } else {
                let(&stmtStr, cat(stmtStr, " <= 100000;\n", NULL));
              }
              let(&printStr, cat(printStr, stmtStr, NULL));
            }

          } /* next m */
        } /* next l */
      } /* next k */

      if (lpRow != lpRows) bug(1009);
      if (orthHyp != orthHyps) bug(1010);

      if (stateMode == DISPERSION_FREE) {
        /* Specify integers for dispersion-free states */
        for (k = 1; k < nodes1 / 2; k++) {
          /* Do only atoms; complements will take care of themselves */
          set_int(lp1, k, TRUE); /* Call to lp_solve */
        }

        if (showVisits) { /* For -v option */
          let(&stmtStr, "");
          for (k = 2; k <= nodes1 / 2; k++) {
            if (k > 2) let(&stmtStr, cat(stmtStr, ",", NULL));
            let(&stmtStr, cat(stmtStr, "m", chr(ATOM_MAP[k - 2]), NULL));
          }
          let(&printStr, cat(printStr, "int ", stmtStr, ";\n", NULL));
        }

      }

      /* If we want to maximize the objective function use
         set_maxim(lp1) but do this JUST BEFORE solving.  Default
         is to minimize. */
      /* set_maxim(lp1); */

      /* For debugging: print the problem */
      /*print2("Atom 1 = %s, atom 2 = %s\n", atom1, atom2);*/
      /*print_lp(lp1);*/ /* Call to lp_solve */

      /* Solve the problem */
      result = solve(lp1); /* Call to lp_solve */
      if (result != 0) {
        if (result == 2) {
          /* INFEASIBLE solution - no states at all */

          if (/*showVisits*/0) { /* For -v option */
            print2("\nProblem statement for lp_solve:\n");
            print2("%s\n", printStr);
          }

          retVal = 2;

          /* Treat no states as lack of (d,s,f) state */
          if (1) goto continue_point;

          goto return_point;
        }
        /* Some other problem */
        print2("?No solution: result = %ld\n",
            result);
        print2(
          "Result values mean: OPTIMAL = 0, MILP_FAIL = 1, INFEASIBLE = 2,\n");
        print2("    UNBOUNDED = 3, FAILURE = 4, RUNNING = 5\n");
        print2("Atom 1 = %s, atom 2 = %s\n", atom1, atom2);
        /*print_lp(lp1);*/
        exit(0);
      }

      /* For debugging: print the solution */
      /*print_solution(lp1);*/ /* Call to lp_solve */

      if (stateMode == DISPERSION_FREE || stateMode == STRONG) {
        /* Look at minimum of state(node2) when state(node1)=1 */
        if (lp1->best_solution[0] > 1.0001) {
          print2("?Rounding error too big\n");
          print2("%8g\n", (double)(lp1->best_solution[0] - 1.0));
          bug(1011);
        }
        if (lp1->best_solution[0] >= /*1.0*/ 0.9999) {
          /* A d or s set of states was not found */
          if (stateMode == STRONG) {
              let(&failingAssignment, cat("(m(", atom1, ") = 1 => m(",
                  atom2, ") = 1) => ",
                  atom1, " =< ", atom2, NULL));
          } else { /* DISPERSION_FREE */
              let(&failingAssignment, cat("m(", atom1, ") =< m(",
                  atom2, ") => ",
                  atom1, " =< ", atom2, NULL));
          }

          if (/*showVisits*/0) { /* For -v option */
            print2("\nProblem statement for lp_solve:\n");
            print2("%s\n", printStr);
          }

          retVal = 1;

          if (1) goto continue_point;

          /* showAllFailures is set by -f option */
          if (showAllFailures) {
            switch (stateMode) {
              case DISPERSION_FREE:
                print2("%s There is no {0,1} state m s.t. %s\n", latticeName,
                    failingAssignment);
                break;
              case STRONG:
                print2("%s There is no state m s.t. %s\n", latticeName,
                    failingAssignment);
                break;
            }
          } else {
            /* Exit loop on first counterexample */
            goto return_point;
          }
        }
      }

      if (stateMode == FULL) {
        /* Look at minimum of state(node2)-state(node1) */
        if (lp1->best_solution[0] > 0.0001) {
          print2("?Rounding error too big\n");
          print2("%8g\n", (double)(lp1->best_solution[0]));
          bug(1012);
        }
        if (lp1->best_solution[0] >= -0.0001) {
          /* A full set of states was not found */
          let(&failingAssignment, cat("m(", atom1, ") =< m(",
              atom2, ") => ",
              atom1, " =< ", atom2, NULL));

          if (/*showVisits*/0) { /* For -v option */
            print2("\nProblem statement for lp_solve:\n");
            print2("%s\n", printStr);
          }

          retVal = 1;

          if (1) goto continue_point;

          /* showAllFailures is set by -f option */
          if (showAllFailures) {
            print2("%s There is no state m s.t. %s\n", latticeName,
                failingAssignment);
          } else {
            /* Exit loop on first counterexample */
            goto return_point;
          }
        }
      }

      /* The trial constraint disabling resulted in a state - so
         the constraint should be enabled again */
      if (pass != 2) bug(1013);
      enableConstraint[constraint - 1] = 'Y';

     continue_point:
      /* Free up memory */
      delete_lp(lp1); /* Call to lp_solve */
    } /* next constraint */

    if (pass == 3 && showVisits) { /* For -v option */
      print2("\nProblem statement for lp_solve:\n");
      print2("%s", printStr);
    }

    if (pass == 3 && !printStateEquation) {
      /* Print the regular lattice equation info */
      print2("%s %s\n", latticeName, greechieStmt);
      print2("Equation variables:  %s\n", normalizedVarList);
      print2("Corresponding atoms: %s\n", left(ATOM_MAP, atoms));
      print2("Equation for input to lattice.c:\n\nlattice");
      for (k = 1; k <= latticeHyps + 1; k++) {
        print2(" $%c", ATOM_MAP[k - 1]);
      }
      print2("\n%s", latticeEqn);
      let(&wffTmp, "");
      wffTmp = fromPolish(cat("<", wfflhs, wffrhs, NULL));
      let(&wffTmp, mid(wffTmp, 2, strlen(wffTmp) - 2));
      print2("%s\n\n", wffTmp);
    }

    /* Get and print the state equation */
    if (pass == 3 && printStateEquation) {
      print2("%s %s\n", latticeName, greechieStmt);
      /* Create the left-hand side of the state equation from the
         enabled blocks, excluding suppressed atoms */
      let(&stateEquation, "");
      /* Initialize atom counts to 1, not 0, because 0 is end-of-string */
      let(&lhAtomCount, string(atoms, 1));
      let(&rhAtomCount, string(atoms, 1));
      /* Create the left-hand side */
      for (k = 1; k <= blocks; k++) {
        if (enableConstraint[k - 1] != 'Y') continue;
        if (stateEquation[0])
          let(&stateEquation, cat(stateEquation, "+", NULL));
        for (l = 1; l <= blockSize[k]; l++) {
          if (strchr(suppressedAtoms, block[k][l]) == NULL) {
            let(&stateEquation, cat(stateEquation,
                chr(ATOM_MAP[block[k][l] - 1]), NULL));
            (lhAtomCount[block[k][l] - 1])++;
            if (strchr(stateEqAtoms, block[k][l]) == NULL) {
              let(&stateEqAtoms, cat(stateEqAtoms, chr(block[k][l]), NULL));
            }
          }
        } /* next l */
      } /* next k */

      let(&stateEquation, cat(stateEquation, "=", NULL));

      /* Create the right-hand side */
      /* We collect all terms from disabled blocks that have >1 stateEqAtoms. */
      for (k = 1; k <= blocks; k++) {
        if (enableConstraint[k - 1] == 'Y') continue;
        let(&stateTerm, "");
        for (l = 1; l <= blockSize[k]; l++) {
          if (strchr(stateEqAtoms, block[k][l]) != NULL) {
            let(&stateTerm, cat(stateTerm,
                chr(block[k][l]), NULL));
          }
        } /* next l */
        if (strlen(stateTerm) > 1) {
          for (m = 0; m < strlen(stateTerm); m++) {
            (rhAtomCount[stateTerm[m] - 1])++;
            stateTerm[m] = ATOM_MAP[stateTerm[m] - 1];
          }
          if (stateEquation[strlen(stateEquation) - 1] != '=')
            let(&stateEquation, cat(stateEquation, "+", NULL));
          let(&stateEquation, cat(stateEquation, stateTerm, NULL));
        }
      } /* next k */

      /*goto skip_norm;*/  /* If actual block atoms desired for debugging */
      /* Normalize the variables in the state equation */
      let(&normalizedVarList, space(256));
      varsUsed = 0;
      for (k = 0; k < strlen(stateEquation); k++) {
        if (stateEquation[k] == '=' || stateEquation[k] == '+') continue;
        if (normalizedVarList[(long)(stateEquation[k])] == ' ') {
          varsUsed++;
          if (varsUsed > strlen(VAR_LIST)) {
            print2("?More than %ld\n variables needed\n", varsUsed - 1);
            exit(0);
          }
          normalizedVarList[(long)(stateEquation[k])] = VAR_LIST[varsUsed - 1];
        }
        stateEquation[k] = normalizedVarList[(long)(stateEquation[k])];
      }
     skip_norm:


      for (k = 0; k < atoms; k++) {
        if (lhAtomCount[k] != rhAtomCount[k]) {
          if (lhAtomCount[k] == 1 || rhAtomCount[k] == 1) {
            let(&stateEquation, cat(stateEquation, " (bad)", NULL));
          } else {
            let(&stateEquation, cat(stateEquation, " (degenerate)", NULL));
          }
          break;
        }
      }
      /* Check that for non-degenerate ones, the number of terms match */
      if (stateEquation[strlen(stateEquation) - 1] != ')') {
        tmpFlag = 0;
        l = 0;
        for (k = 0; k < strlen(stateEquation); k++) {
          if (stateEquation[k] == '=') tmpFlag = 1;
          if (stateEquation[k] == '+') {
            if (tmpFlag) l--;
            else l++;
          }
        }
        if (l != 0)
          let(&stateEquation, cat(stateEquation, " (bad)", NULL));
      }

      print2("State eqn: %s\n", stateEquation);

    } /* if pass==3 */

  } /* next pass */

 return_point:
  /* Deallocate memory */
  let(&atom1, "");
  let(&atom2, "");
  /*let(&polWff, "");*/
  let(&printStr, "");
  let(&stmtStr, "");
  let(&enableConstraint, "");
  let(&wfflhs, "");
  let(&wffrhs, "");
  let(&wffTmp, "");
  let(&normalizedVarList, "");
  let(&latticeEqn, "");

  let(&suppressedAtoms, "");
  let(&stateEqAtoms, "");
  let(&stateEquation, "");
  let(&stateTerm, "");
  let(&lhAtomCount, "");
  let(&rhAtomCount, "");

  return;
} /* statePrntEqn */


/* Returns TRUE_CONST if matrix test passed, FALSE_CONST if failed */
/* When FALSE_CONST is returned, the string failingAssignment has the
   assignment that violates the lattice */
unsigned char test(void)
{
  long i, j, k, n, conclusionLen, p, e, maxDepth, loopDepth;
  vstring trialConclusion = "";
  vstring trialHypList[MAX_HYPS];
  long loopVar[MAX_VARS];
  vstring fromPol = "";
  long hypLen[MAX_HYPS];
  long maxHypVar[MAX_HYPS];
  long maxConclusionVar;
  vstring tmpStr = "";
  vstring tmpStr2 = "";
  vstring tmpStr3 = "";
  vstring tmpStr4 = "";
  vstring tmpStr5 = "";
  vstring varOrder = "";
  char atLeastOneFailure = 0; /* Used in conjunction with showAllFailures */

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
  let(&tmpStr3, ""); /* Deallocate string */
  let(&tmpStr4, ""); /* Deallocate string */
  let(&tmpStr5, ""); /* Deallocate string */
  let(&varOrder, ""); /* Deallocate string */
  for (i = 0; i < hypotheses; i++) {
    let(&(trialHypList[i]), "");  /* Deallocate string */
  }
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


/******************* Start of lp_solve functions *****************************/

/******************** lpkit.c *******************************************/

/* #include "lpkit.h" */
/* #include "lpglob.h" */
/* #include <stdlib.h> */
/* #include <stdarg.h> */
/* #include <stdio.h> */
/* #include <string.h> */

#define HASHSIZE 10007

/* Globals */
int     Rows;
int     Columns;
int     Sum;
int     Non_zeros;
int     Level;

REAL    Trej;

short   Maximise;
REAL    Extrad;

int     Warn_count; /* used in CHECK version of rounding macro */

void error(char *format, ...)
{
  va_list ap;

  va_start(ap, format);
  vfprintf(stderr, format, ap);
  fputc('\n', stderr);
  va_end(ap);

  exit(EXIT_FAILURE);
}

lprec *make_lp(int rows, int columns)
{
  lprec *newlp;
  int i, sum;

  if(rows < 0 || columns < 0)
    error("rows < 0 or columns < 0");

  sum = rows + columns;

  CALLOC(newlp, 1);

  strcpy(newlp->lp_name, "unnamed");

  newlp->verbose = FALSE;
  newlp->print_duals = FALSE;
  newlp->print_sol = FALSE;
  newlp->debug = FALSE;
  newlp->print_at_invert = FALSE;
  newlp->trace = FALSE;

  newlp->rows = rows;
  newlp->columns = columns;
  newlp->sum = sum;
  newlp->rows_alloc = rows;
  newlp->columns_alloc = columns;
  newlp->sum_alloc = sum;
  newlp->names_used = FALSE;

  newlp->obj_bound = DEF_INFINITE;
  newlp->infinite = DEF_INFINITE;
  newlp->epsilon = DEF_EPSILON;
  newlp->epsb = DEF_EPSB;
  newlp->epsd = DEF_EPSD;
  newlp->epsel = DEF_EPSEL;
  newlp->non_zeros = 0;
  newlp->mat_alloc = 1;
  CALLOC(newlp->mat, newlp->mat_alloc);
  CALLOC(newlp->col_no, newlp->mat_alloc + 1);
  CALLOC(newlp->col_end, columns + 1);
  CALLOC(newlp->row_end, rows + 1);
  newlp->row_end_valid = FALSE;
  CALLOC(newlp->orig_rh, rows + 1);
  CALLOC(newlp->rh, rows + 1);
  CALLOC(newlp->rhs, rows + 1);

  CALLOC(newlp->must_be_int, sum + 1);
  for(i = 0; i <= sum; i++)
    newlp->must_be_int[i] = FALSE;

  CALLOC(newlp->orig_upbo, sum + 1);
  for(i = 0; i <= sum; i++)
    newlp->orig_upbo[i] = newlp->infinite;

  CALLOC(newlp->upbo, sum + 1);
  CALLOC(newlp->orig_lowbo, sum + 1);
  CALLOC(newlp->lowbo, sum + 1);

  newlp->basis_valid = TRUE;
  CALLOC(newlp->bas, rows+1);
  CALLOC(newlp->basis, sum + 1);
  CALLOC(newlp->lower, sum + 1);

  for(i = 0; i <= rows; i++) {
    newlp->bas[i] = i;
    newlp->basis[i] = TRUE;
  }

  for(i = rows + 1; i <= sum; i++)
    newlp->basis[i] = FALSE;


  for(i = 0 ; i <= sum; i++)
    newlp->lower[i] = TRUE;

  newlp->eta_valid = TRUE;
  newlp->eta_size = 0;
  newlp->eta_alloc = INITIAL_MAT_SIZE;
  newlp->max_num_inv = DEFNUMINV;

  newlp->nr_lagrange = 0;

  CALLOC(newlp->eta_value, newlp->eta_alloc);
  CALLOC(newlp->eta_row_nr, newlp->eta_alloc);

  /* +1 reported by Christian Rank */
  CALLOC(newlp->eta_col_end, newlp->rows_alloc + newlp->max_num_inv + 1);

  newlp->bb_rule = FIRST_NI;
  newlp->break_at_int = FALSE;
  newlp->break_value = 0;

  newlp->iter = 0;
  newlp->total_iter = 0;

  CALLOC(newlp->solution, sum + 1);
  CALLOC(newlp->best_solution, sum + 1);
  CALLOC(newlp->duals, rows + 1);

  newlp->maximise = FALSE;
  newlp->floor_first = TRUE;

  newlp->scaling_used = FALSE;
  newlp->columns_scaled = FALSE;

  CALLOC(newlp->ch_sign, rows + 1);

  for(i = 0; i <= rows; i++)
    newlp->ch_sign[i] = FALSE;

  newlp->valid = FALSE;

  /* create two hash tables for names */
  newlp->rowname_hashtab = create_hash_table(HASHSIZE);
  newlp->colname_hashtab = create_hash_table(HASHSIZE);

  return(newlp);
}

void delete_lp(lprec *lp)
{
  int i;

  if(lp->names_used) {
    free(lp->row_name);
    free(lp->col_name);
  }

  free(lp->mat);
  free(lp->col_no);
  free(lp->col_end);
  free(lp->row_end);
  free(lp->orig_rh);
  free(lp->rh);
  free(lp->rhs);
  free(lp->must_be_int);
  free(lp->orig_upbo);
  free(lp->orig_lowbo);
  free(lp->upbo);
  free(lp->lowbo);
  free(lp->bas);
  free(lp->basis);
  free(lp->lower);
  free(lp->eta_value);
  free(lp->eta_row_nr);
  free(lp->eta_col_end);
  free(lp->solution);
  free(lp->best_solution);
  free(lp->duals);
  free(lp->ch_sign);
  if(lp->scaling_used)
    free(lp->scale);
  if(lp->nr_lagrange > 0) {
    free(lp->lag_rhs);
    free(lp->lambda);
    free(lp->lag_con_type);
    for(i = 0; i < lp->nr_lagrange; i++)
      free(lp->lag_row[i]);
    free(lp->lag_row);
  }

  free_hash_table(lp->rowname_hashtab);
  free_hash_table(lp->colname_hashtab);

  free(lp);

}

lprec *copy_lp(lprec *lp)
{
  lprec *newlp;
  int i, rowsplus, colsplus, sumplus;

  rowsplus = lp->rows_alloc + 1;
  colsplus = lp->columns_alloc + 1;
  sumplus  = lp->sum_alloc + 1;

  MALLOCCPY(newlp, lp, 1); /* copy all non pointers */

  if(newlp->names_used) {
    MALLOCCPY(newlp->col_name, lp->col_name, colsplus);
    MALLOCCPY(newlp->row_name, lp->row_name, rowsplus);
  }

  newlp->rowname_hashtab = copy_hash_table(lp->rowname_hashtab);
  newlp->colname_hashtab = copy_hash_table(lp->colname_hashtab);

  MALLOCCPY(newlp->mat, lp->mat, newlp->mat_alloc);
  MALLOCCPY(newlp->col_end, lp->col_end, colsplus);
  MALLOCCPY(newlp->col_no, lp->col_no, newlp->mat_alloc + 1);
  MALLOCCPY(newlp->row_end, lp->row_end, rowsplus);
  MALLOCCPY(newlp->orig_rh, lp->orig_rh, rowsplus);
  MALLOCCPY(newlp->rh, lp->rh, rowsplus);
  MALLOCCPY(newlp->rhs, lp->rhs, rowsplus);
  MALLOCCPY(newlp->must_be_int, lp->must_be_int, sumplus);
  MALLOCCPY(newlp->orig_upbo, lp->orig_upbo, sumplus);
  MALLOCCPY(newlp->orig_lowbo, lp->orig_lowbo, sumplus);
  MALLOCCPY(newlp->upbo, lp->upbo, sumplus);
  MALLOCCPY(newlp->lowbo, lp->lowbo, sumplus);
  MALLOCCPY(newlp->bas, lp->bas, rowsplus);
  MALLOCCPY(newlp->basis, lp->basis, sumplus);
  MALLOCCPY(newlp->lower, lp->lower, sumplus);
  MALLOCCPY(newlp->eta_value, lp->eta_value, lp->eta_alloc);
  MALLOCCPY(newlp->eta_row_nr, lp->eta_row_nr, lp->eta_alloc);
  MALLOCCPY(newlp->eta_col_end, lp->eta_col_end,
            lp->rows_alloc + lp->max_num_inv + 1);
  MALLOCCPY(newlp->solution, lp->solution, sumplus);
  MALLOCCPY(newlp->best_solution, lp->best_solution, sumplus);
  MALLOCCPY(newlp->duals, lp->duals, rowsplus);
  MALLOCCPY(newlp->ch_sign, lp->ch_sign, rowsplus);

  if(newlp->scaling_used)
    MALLOCCPY(newlp->scale, lp->scale, sumplus);

  if(newlp->nr_lagrange > 0) {
    MALLOCCPY(newlp->lag_rhs, lp->lag_rhs, newlp->nr_lagrange);
    MALLOCCPY(newlp->lambda, lp->lambda, newlp->nr_lagrange);
    MALLOCCPY(newlp->lag_con_type, lp->lag_con_type, newlp->nr_lagrange);
    MALLOC(newlp->lag_row, newlp->nr_lagrange);
    for(i = 0; i < newlp->nr_lagrange; i++)
      MALLOCCPY(newlp->lag_row[i], lp->lag_row[i], colsplus);
  }
  return(newlp);
}

void inc_mat_space(lprec *lp, int maxextra)
{
   if(lp->non_zeros + maxextra >= lp->mat_alloc) {
     /* let's allocate at least INITIAL_MAT_SIZE  entries */
     if(lp->mat_alloc < INITIAL_MAT_SIZE)
       lp->mat_alloc = INITIAL_MAT_SIZE;

     /* increase the size by 50% each time it becomes too small */
     while(lp->non_zeros + maxextra >= lp->mat_alloc)
       lp->mat_alloc *= 1.5;

     REALLOC(lp->mat, lp->mat_alloc);
     REALLOC(lp->col_no, lp->mat_alloc + 1);
   }
}

void inc_row_space(lprec *lp)
{
  if(lp->rows > lp->rows_alloc) {
    lp->rows_alloc = lp->rows+10;
    lp->sum_alloc  = lp->rows_alloc + lp->columns_alloc;
    REALLOC(lp->orig_rh, lp->rows_alloc + 1);
    REALLOC(lp->rh, lp->rows_alloc + 1);
    REALLOC(lp->rhs, lp->rows_alloc + 1);
    REALLOC(lp->orig_upbo, lp->sum_alloc + 1);
    REALLOC(lp->upbo, lp->sum_alloc + 1);
    REALLOC(lp->orig_lowbo, lp->sum_alloc + 1);
    REALLOC(lp->lowbo, lp->sum_alloc + 1);
    REALLOC(lp->solution, lp->sum_alloc + 1);
    REALLOC(lp->best_solution, lp->sum_alloc + 1);
    REALLOC(lp->row_end, lp->rows_alloc + 1);
    REALLOC(lp->basis, lp->sum_alloc + 1);
    REALLOC(lp->lower, lp->sum_alloc + 1);
    REALLOC(lp->must_be_int, lp->sum_alloc + 1);
    REALLOC(lp->bas, lp->rows_alloc + 1);
    REALLOC(lp->duals, lp->rows_alloc + 1);
    REALLOC(lp->ch_sign, lp->rows_alloc + 1);
    REALLOC(lp->eta_col_end, lp->rows_alloc + lp->max_num_inv + 1);
    if(lp->names_used)
      REALLOC(lp->row_name, lp->rows_alloc + 1);
    if(lp->scaling_used)
      REALLOC(lp->scale, lp->sum_alloc + 1);
  }
}

void inc_col_space(lprec *lp)
{
  if(lp->columns >= lp->columns_alloc) {
    lp->columns_alloc = lp->columns + 10;
    lp->sum_alloc = lp->rows_alloc + lp->columns_alloc;
    REALLOC(lp->must_be_int, lp->sum_alloc + 1);
    REALLOC(lp->orig_upbo, lp->sum_alloc + 1);
    REALLOC(lp->upbo, lp->sum_alloc + 1);
    REALLOC(lp->orig_lowbo, lp->sum_alloc + 1);
    REALLOC(lp->lowbo, lp->sum_alloc + 1);
    REALLOC(lp->solution, lp->sum_alloc + 1);
    REALLOC(lp->best_solution, lp->sum_alloc + 1);
    REALLOC(lp->basis, lp->sum_alloc + 1);
    REALLOC(lp->lower, lp->sum_alloc + 1);
    if(lp->names_used)
      REALLOC(lp->col_name, lp->columns_alloc + 1);
    if(lp->scaling_used)
      REALLOC(lp->scale, lp->sum_alloc + 1);
    REALLOC(lp->col_end, lp->columns_alloc + 1);
  }
}

void set_mat(lprec *lp, int Row, int Column, REAL Value)
{
  int elmnr, lastelm, i;

  /* This function is very inefficient if used to add new matrix entries in
     other places than at the end of the matrix. OK for replacing existing
     non-zero values */

  if(Row > lp->rows || Row < 0)
    error("Row out of range");
  if(Column > lp->columns || Column < 1)
    error("Column out of range");

  /* scaling is performed twice? MB */
  if(lp->scaling_used)
    Value *= lp->scale[Row] * lp->scale[lp->rows + Column];

  if (lp->basis[Column] == TRUE && Row > 0)
    lp->basis_valid = FALSE;
  lp->eta_valid = FALSE;

  /* find out if we already have such an entry */
  elmnr = lp->col_end[Column - 1];
  while((elmnr < lp->col_end[Column]) && (lp->mat[elmnr].row_nr != Row))
    elmnr++;

  if((elmnr != lp->col_end[Column]) && (lp->mat[elmnr].row_nr == Row)) {
    /* there is an existing entry */
    if(my_abs(Value) > lp->epsilon) { /* we replace it by something non-zero */
      if (lp->scaling_used) {
        if(lp->ch_sign[Row])
          lp->mat[elmnr].value = -Value * lp->scale[Row] * lp->scale[Column];
        else
          lp->mat[elmnr].value = Value * lp->scale[Row] * lp->scale[Column];
      }
      else { /* no scaling */
        if(lp->ch_sign[Row])
          lp->mat[elmnr].value = -Value;
        else
          lp->mat[elmnr].value = Value;
      }
    }
    else { /* setting existing non-zero entry to zero. Remove the entry */
      /* this might remove an entire column, or leave just a bound. No
         nice solution for that yet */

      /* Shift the matrix */
      lastelm = lp->non_zeros;
      for(i = elmnr; i < lastelm ; i++)
        lp->mat[i] = lp->mat[i + 1];
      for(i = Column; i <= lp->columns; i++)
        lp->col_end[i]--;

      lp->non_zeros--;
    }
  }
  else if(my_abs(Value) > lp->epsilon) {
    /* no existing entry. make new one only if not nearly zero */
    /* check if more space is needed for matrix */
    inc_mat_space(lp, 1);

    /* Shift the matrix */
    lastelm = lp->non_zeros;
    for(i = lastelm; i > elmnr ; i--)
      lp->mat[i] = lp->mat[i - 1];
    for(i = Column; i <= lp->columns; i++)
      lp->col_end[i]++;

    /* Set new element */
    lp->mat[elmnr].row_nr = Row;

    if (lp->scaling_used) {
      if(lp->ch_sign[Row])
        lp->mat[elmnr].value = -Value * lp->scale[Row] * lp->scale[Column];
      else
        lp->mat[elmnr].value = Value * lp->scale[Row] * lp->scale[Column];
    }
    else /* no scaling */
      {
        if(lp->ch_sign[Row])
          lp->mat[elmnr].value = -Value;
        else
          lp->mat[elmnr].value = Value;
      }

    lp->row_end_valid = FALSE;

    lp->non_zeros++;
  }
}

void set_obj_fn(lprec *lp, REAL *row)
{
  int i;
  for(i = 1; i <= lp->columns; i++)
    set_mat(lp, 0, i, row[i]);
}

void str_set_obj_fn(lprec *lp, char *row)
{
  int  i;
  REAL *arow;
  char *p, *newp;
  CALLOC(arow, lp->columns + 1);
  p = row;
  for(i = 1; i <= lp->columns; i++) {
    arow[i] = (REAL) strtod(p, &newp);
    if(p == newp)
      error("Bad string in str_set_obj_fn");
    else
      p = newp;
  }
  set_obj_fn(lp, arow);
  free(arow);
}


void add_constraint(lprec *lp, REAL *row, short constr_type, REAL rh)
{
  matrec *newmat;
  int  i, j;
  int  elmnr;
  int  stcol;
  int  *addtoo;

  MALLOC(addtoo, lp->columns + 1);

    for(i = 1; i <= lp->columns; i++)
      if(row[i] != 0) {
        addtoo[i] = TRUE;
        lp->non_zeros++;
      }
      else
        addtoo[i] = FALSE;

  MALLOC(newmat, lp->non_zeros);
  inc_mat_space(lp, 0);
  lp->rows++;
  lp->sum++;
  inc_row_space(lp);

  if(lp->scaling_used) {
    /* shift scale */
    for(i = lp->sum; i > lp->rows; i--)
      lp->scale[i] = lp->scale[i - 1];
    lp->scale[lp->rows] = 1;
  }

  if(lp->names_used)
    sprintf(lp->row_name[lp->rows], "r_%d", lp->rows);

  if(lp->scaling_used && lp->columns_scaled)
    for(i = 1; i <= lp->columns; i++)
      row[i] *= lp->scale[lp->rows + i];

  if(constr_type == GE)
    lp->ch_sign[lp->rows] = TRUE;
  else
    lp->ch_sign[lp->rows] = FALSE;

  elmnr = 0;
  stcol = 0;
  for(i = 1; i <= lp->columns; i++) {
    for(j = stcol; j < lp->col_end[i]; j++) {
      newmat[elmnr].row_nr = lp->mat[j].row_nr;
      newmat[elmnr].value = lp->mat[j].value;
      elmnr++;
    }
    if(addtoo[i]) {
      if(lp->ch_sign[lp->rows])
        newmat[elmnr].value = -row[i];
      else
        newmat[elmnr].value = row[i];
      newmat[elmnr].row_nr = lp->rows;
      elmnr++;
    }
    stcol = lp->col_end[i];
    lp->col_end[i] = elmnr;
  }

  memcpy(lp->mat, newmat, lp->non_zeros * sizeof(matrec));

  free(newmat);
  free(addtoo);

  for(i = lp->sum; i > lp->rows; i--) {
    lp->orig_upbo[i]   = lp->orig_upbo[i - 1];
    lp->orig_lowbo[i]  = lp->orig_lowbo[i - 1];
    lp->basis[i]       = lp->basis[i - 1];
    lp->lower[i]       = lp->lower[i - 1];
    lp->must_be_int[i] = lp->must_be_int[i - 1];
  }

  /* changed from i <= lp->rows to i < lp->rows, MB */
  for(i = 1 ; i < lp->rows; i++)
    if(lp->bas[i] >= lp->rows)
      lp->bas[i]++;

  if(constr_type == LE || constr_type == GE) {
    lp->orig_upbo[lp->rows] = lp->infinite;
  }
  else if(constr_type == EQ) {
    lp->orig_upbo[lp->rows] = 0;
  }
  else {
    fprintf(stderr, "Wrong constraint type\n");
    exit(EXIT_FAILURE);
  }

  lp->orig_lowbo[lp->rows] = 0;

  if(constr_type == GE && rh != 0)
    lp->orig_rh[lp->rows] = -rh;
  else
    lp->orig_rh[lp->rows] = rh;

  lp->row_end_valid = FALSE;

  lp->bas[lp->rows] = lp->rows;
  lp->basis[lp->rows] = TRUE;
  lp->lower[lp->rows] = TRUE;
  lp->eta_valid = FALSE;
}

void str_add_constraint(lprec *lp,
                        char *row_string,
                        short constr_type,
                        REAL rh)
{
  int  i;
  REAL *aRow;
  char *p, *newp;
  CALLOC(aRow, lp->columns + 1);
  p = row_string;

  for(i = 1; i <= lp->columns; i++) {
    aRow[i] = (REAL) strtod(p, &newp);
    if(p == newp)
      error("Bad string in str_add_constr");
    else
      p = newp;
  }
  add_constraint(lp, aRow, constr_type, rh);
  free(aRow);
}

void del_constraint(lprec *lp, int del_row)
{
  int i, j;
  unsigned elmnr;
  int startcol;

  if(del_row<1 || del_row>lp->rows)
    {
      fprintf(stderr, "There is no constraint nr. %d\n", del_row);
      exit(EXIT_FAILURE);
    }

  elmnr = 0;
  startcol = 0;

  for(i = 1; i <= lp->columns; i++) {
    for(j = startcol; j < lp->col_end[i]; j++) {
      if(lp->mat[j].row_nr != del_row) {
        lp->mat[elmnr] = lp->mat[j];
        if(lp->mat[elmnr].row_nr > del_row)
          lp->mat[elmnr].row_nr--;
        elmnr++;
      }
      else
        lp->non_zeros--;
    }
    startcol = lp->col_end[i];
    lp->col_end[i] = elmnr;
  }
  for(i = del_row; i < lp->rows; i++) {
    lp->orig_rh[i] = lp->orig_rh[i + 1];
    lp->ch_sign[i] = lp->ch_sign[i + 1];
    lp->bas[i] = lp->bas[i + 1];
    if(lp->names_used)
      strcpy(lp->row_name[i], lp->row_name[i + 1]);
  }
  for(i = 1; i < lp->rows; i++)
    if(lp->bas[i] >  del_row)
      lp->bas[i]--;

  for(i = del_row; i < lp->sum; i++) {
    lp->lower[i] = lp->lower[i + 1];
    lp->basis[i] = lp->basis[i + 1];
    lp->orig_upbo[i] = lp->orig_upbo[i + 1];
    lp->orig_lowbo[i] = lp->orig_lowbo[i + 1];
    lp->must_be_int[i] = lp->must_be_int[i + 1];
    if(lp->scaling_used)
      lp->scale[i] = lp->scale[i + 1];
  }

  lp->rows--;
  lp->sum--;

  lp->row_end_valid = FALSE;
  lp->eta_valid     = FALSE;
  lp->basis_valid   = FALSE;
}

void add_lag_con(lprec *lp, REAL *row, short con_type, REAL rhs)
{
  int i;
  REAL sign;
  if(con_type == LE || con_type == EQ)
    sign = 1;
  else if(con_type == GE)
    sign = -1;
  else
    error("con_type not implemented\n");

  lp->nr_lagrange++;
  if(lp->nr_lagrange == 1) {
    CALLOC(lp->lag_row, lp->nr_lagrange);
    CALLOC(lp->lag_rhs, lp->nr_lagrange);
    CALLOC(lp->lambda, lp->nr_lagrange);
    CALLOC(lp->lag_con_type, lp->nr_lagrange);
  }
  else {
    REALLOC(lp->lag_row, lp->nr_lagrange);
    REALLOC(lp->lag_rhs, lp->nr_lagrange);
    REALLOC(lp->lambda, lp->nr_lagrange);
    REALLOC(lp->lag_con_type, lp->nr_lagrange);
  }
  CALLOC(lp->lag_row[lp->nr_lagrange-1], lp->columns+1);
  lp->lag_rhs[lp->nr_lagrange-1] = rhs * sign;
  for( i = 1; i <= lp->columns; i++)
    lp->lag_row[lp->nr_lagrange-1][i] = row[i] * sign;
  lp->lambda[lp->nr_lagrange-1] = 0;
  lp->lag_con_type[lp->nr_lagrange-1]=(con_type == EQ);
}

void str_add_lag_con(lprec *lp, char *row, short con_type, REAL rhs)
{
  int  i;
  REAL *a_row;
  char *p, *new_p;
  CALLOC(a_row, lp->columns + 1);
  p = row;

  for(i = 1; i <= lp->columns; i++) {
    a_row[i] = (REAL) strtod(p, &new_p);
    if(p == new_p)
      error("Bad string in str_add_lag_con");
    else
      p = new_p;
  }
  add_lag_con(lp, a_row, con_type, rhs);
  free(a_row);
}


void add_column(lprec *lp, REAL *column)
{
  int i, elmnr;

  /* if the column has only one entry, this should be handled as a bound, but
     this currently is not the case */

  lp->columns++;
  lp->sum++;
  inc_col_space(lp);
  inc_mat_space(lp, lp->rows + 1);

  if(lp->scaling_used) {
    for(i = 0; i <= lp->rows; i++)
      column[i] *= lp->scale[i];
    lp->scale[lp->sum] = 1;
  }

  elmnr = lp->col_end[lp->columns - 1];
  for(i = 0 ; i <= lp->rows ; i++)
    if(column[i] != 0) {
      lp->mat[elmnr].row_nr = i;
      if(lp->ch_sign[i])
        lp->mat[elmnr].value = -column[i];
      else
        lp->mat[elmnr].value = column[i];
      lp->non_zeros++;
      elmnr++;
    }
  lp->col_end[lp->columns] = elmnr;
  lp->orig_lowbo[lp->sum] = 0;
  lp->orig_upbo[lp->sum] = lp->infinite;
  lp->lower[lp->sum] = TRUE;
  lp->basis[lp->sum] = FALSE;
  lp->must_be_int[lp->sum] = FALSE;
  if(lp->names_used)
    sprintf(lp->col_name[lp->columns], "var_%d", lp->columns);

  lp->row_end_valid = FALSE;
}

void str_add_column(lprec *lp, char *col_string)
{
  int  i;
  REAL *aCol;
  char *p, *newp;
  CALLOC(aCol, lp->rows + 1);
  p = col_string;

  for(i = 0; i <= lp->rows; i++) {
    aCol[i] = (REAL) strtod(p, &newp);
    if(p == newp)
      error("Bad string in str_add_column");
    else
      p = newp;
  }
  add_column(lp, aCol);
  free(aCol);
}

void del_column(lprec *lp, int column)
{
  int i, j, from_elm, to_elm, elm_in_col;
  if(column > lp->columns || column < 1)
    error("Column out of range in del_column");
  for(i = 1; i <= lp->rows; i++) {
    if(lp->bas[i] == lp->rows + column)
      lp->basis_valid = FALSE;
    else if(lp->bas[i] > lp->rows + column)
      lp->bas[i]--;
  }
  for(i = lp->rows + column; i < lp->sum; i++) {
    if(lp->names_used)
      strcpy(lp->col_name[i - lp->rows], lp->col_name[i - lp->rows + 1]);
    lp->must_be_int[i] = lp->must_be_int[i + 1];
    lp->orig_upbo[i] = lp->orig_upbo[i + 1];
    lp->orig_lowbo[i] = lp->orig_lowbo[i + 1];
    lp->upbo[i] = lp->upbo[i + 1];
    lp->lowbo[i] = lp->lowbo[i + 1];
    lp->basis[i] = lp->basis[i + 1];
    lp->lower[i] = lp->lower[i + 1];
    if(lp->scaling_used)
      lp->scale[i] = lp->scale[i + 1];
  }
  for(i = 0; i < lp->nr_lagrange; i++)
    for(j = column; j <= lp->columns; j++)
      lp->lag_row[i][j] = lp->lag_row[i][j+1];
  to_elm = lp->col_end[column-1];
  from_elm = lp->col_end[column];
  elm_in_col = from_elm-to_elm;
  for(i = from_elm; i < lp->non_zeros; i++) {
    lp->mat[to_elm] = lp->mat[i];
    to_elm++;
  }
  for(i = column; i < lp->columns; i++)
    lp->col_end[i] = lp->col_end[i + 1] - elm_in_col;
  lp->non_zeros -= elm_in_col;
  lp->row_end_valid = FALSE;
  lp->eta_valid = FALSE;

  lp->sum--;
  lp->columns--;
}

void set_upbo(lprec *lp, int column, REAL value)
{
  if(column > lp->columns || column < 1)
    error("Column out of range");
  if(lp->scaling_used)
    value /= lp->scale[lp->rows + column];
  if(value < lp->orig_lowbo[lp->rows + column])
    error("Upperbound must be >= lowerbound");
  lp->eta_valid = FALSE;
  lp->orig_upbo[lp->rows + column] = value;
}

void set_lowbo(lprec *lp, int column, REAL value)
{
  if(column > lp->columns || column < 1)
    error("Column out of range");
  if(lp->scaling_used)
    value /= lp->scale[lp->rows + column];
  if(value > lp->orig_upbo[lp->rows + column])
    error("Upperbound must be >= lowerbound");
  /*
    if(value < 0)
    error("Lower bound cannot be < 0");
    */
  lp->eta_valid = FALSE;
  lp->orig_lowbo[lp->rows + column] = value;
}

void set_int(lprec *lp, int column, short must_be_int)
{
  if(column > lp->columns || column < 1)
    error("Column out of range");
  lp->must_be_int[lp->rows + column] = must_be_int;
  if(must_be_int == TRUE)
    if(lp->columns_scaled)
      unscale_columns(lp);
}

void set_rh(lprec *lp, int row, REAL value)
{
  if(row > lp->rows || row < 0)
    error("Row out of Range");

  if ((row == 0) && (!lp->maximise))  /* setting of RHS of OF IS meaningful */
    value = -value;
  if(lp->scaling_used) {
    if(lp->ch_sign[row])
      lp->orig_rh[row] = -value * lp->scale[row];
    else
      lp->orig_rh[row] = value * lp->scale[row];
  }
  else
    if(lp->ch_sign[row])
      lp->orig_rh[row] = -value;
    else
      lp->orig_rh[row] = value;
  lp->eta_valid = FALSE;
}

void set_rh_vec(lprec *lp, REAL *rh)
{
  int i;
  if(lp->scaling_used) {
    for(i = 1; i <= lp->rows; i++)
      if(lp->ch_sign[i])
        lp->orig_rh[i] = -rh[i]*lp->scale[i];
      else
        lp->orig_rh[i] = rh[i]*lp->scale[i];
  }
  else
    for(i = 1; i <= lp->rows; i++)
      if(lp->ch_sign[i])
        lp->orig_rh[i] = -rh[i];
      else
        lp->orig_rh[i] = rh[i];
  lp->eta_valid = FALSE;
}

void str_set_rh_vec(lprec *lp, char *rh_string)
{
  int  i;
  REAL *newrh;
  char *p, *newp;
  CALLOC(newrh, lp->rows + 1);
  p = rh_string;

  for(i = 1; i <= lp->rows; i++) {
    newrh[i] = (REAL) strtod(p, &newp);
    if(p == newp)
      error("Bad string in str_set_rh_vec");
    else
      p = newp;
  }
  set_rh_vec(lp, newrh);
  free(newrh);
}


void set_maxim(lprec *lp)
{
  int i;
  if(lp->maximise == FALSE) {
    for(i = 0; i < lp->non_zeros; i++)
      if(lp->mat[i].row_nr == 0)
        lp->mat[i].value *= -1;
    lp->eta_valid = FALSE;
    lp->orig_rh[0] *= -1;
  }
  lp->maximise = TRUE;
  lp->ch_sign[0] = TRUE;
}

void set_minim(lprec *lp)
{
  int i;
  if(lp->maximise == TRUE) {
    for(i = 0; i < lp->non_zeros; i++)
      if(lp->mat[i].row_nr == 0)
        lp->mat[i].value = -lp->mat[i].value;
    lp->eta_valid = FALSE;
    lp->orig_rh[0] *= -1;
  }
  lp->maximise = FALSE;
  lp->ch_sign[0] = FALSE;
}

void set_constr_type(lprec *lp, int row, short con_type)
{
  int i;
  if(row > lp->rows || row < 1)
    error("Row out of Range");
  if(con_type == EQ) {
    lp->orig_upbo[row] = 0;
    lp->basis_valid = FALSE;
    if(lp->ch_sign[row]) {
      for(i = 0; i < lp->non_zeros; i++)
        if(lp->mat[i].row_nr == row)
          lp->mat[i].value *= -1;
      lp->eta_valid = FALSE;
      lp->ch_sign[row] = FALSE;
      if(lp->orig_rh[row] != 0)
        lp->orig_rh[row] *= -1;
    }
  }
  else if(con_type == LE) {
    lp->orig_upbo[row] = lp->infinite;
    lp->basis_valid = FALSE;
    if(lp->ch_sign[row]) {
      for(i = 0; i < lp->non_zeros; i++)
        if(lp->mat[i].row_nr == row)
          lp->mat[i].value *= -1;
      lp->eta_valid = FALSE;
      lp->ch_sign[row] = FALSE;
      if(lp->orig_rh[row] != 0)
        lp->orig_rh[row] *= -1;
    }
  }
  else if(con_type == GE) {
    lp->orig_upbo[row] = lp->infinite;
    lp->basis_valid = FALSE;
    if(!lp->ch_sign[row]) {
      for(i = 0; i < lp->non_zeros; i++)
        if(lp->mat[i].row_nr == row)
          lp->mat[i].value *= -1;
      lp->eta_valid = FALSE;
      lp->ch_sign[row] = TRUE;
      if(lp->orig_rh[row] != 0)
        lp->orig_rh[row] *= -1;
    }
  }
  else
    error("Constraint type not (yet) implemented");
}

REAL mat_elm(lprec *lp, int row, int column)
{
  REAL value;
  int elmnr;
  if(row < 0 || row > lp->rows)
    error("Row out of range in mat_elm");
  if(column < 1 || column > lp->columns)
    error("Column out of range in mat_elm");
  value = 0;
  elmnr = lp->col_end[column-1];
  while(lp->mat[elmnr].row_nr != row && elmnr < lp->col_end[column])
    elmnr++;
  if(elmnr != lp->col_end[column]) {
    value = lp->mat[elmnr].value;
    if(lp->ch_sign[row])
      value = -value;
    if(lp->scaling_used)
      value /= lp->scale[row] * lp->scale[lp->rows + column];
  }
  return(value);
}


void get_row(lprec *lp, int row_nr, REAL *row)
{
  int i, j;

  if(row_nr <0 || row_nr > lp->rows)
    error("Row nr. out of range in get_row");
  for(i = 1; i <= lp->columns; i++) {
    row[i] = 0;
    for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++)
      if(lp->mat[j].row_nr == row_nr)
        row[i] = lp->mat[j].value;
    if(lp->scaling_used)
      row[i] /= lp->scale[lp->rows + i] * lp->scale[row_nr];
  }
  if(lp->ch_sign[row_nr])
    for(i = 0; i <= lp->columns; i++)
      if(row[i] != 0)
        row[i] = -row[i];
}

void get_column(lprec *lp, int col_nr, REAL *column)
{
  int i;

  if(col_nr < 1 || col_nr > lp->columns)
    error("Col. nr. out of range in get_column");
  for(i = 0; i <= lp->rows; i++)
    column[i] = 0;
  for(i = lp->col_end[col_nr - 1]; i < lp->col_end[col_nr]; i++)
    column[lp->mat[i].row_nr] = lp->mat[i].value;
  for(i = 0; i <= lp->rows; i++)
    if(column[i] != 0) {
      if(lp->ch_sign[i])
        column[i] *= -1;
      if(lp->scaling_used)
        column[i] /= (lp->scale[i] * lp->scale[lp->rows + col_nr]);
    }
}

void get_reduced_costs(lprec *lp, REAL *rc)
{
  int varnr, i, j;
  REAL f;

  if(!lp->basis_valid)
    error("Not a valid basis in get_reduced_costs");

  if(!lp->eta_valid)
    invert(lp);

  for(i = 1; i <= lp->sum; i++)
    rc[i] = 0;
  rc[0] = 1;

  btran(lp, rc);

  for(i = 1; i <= lp->columns; i++) {
    varnr = lp->rows + i;
    if(!lp->basis[varnr])
      if(lp->upbo[varnr] > 0) {
        f = 0;
        for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++)
          f += rc[lp->mat[j].row_nr] * lp->mat[j].value;
        rc[varnr] = f;
      }
  }
  for(i = 1; i <= lp->sum; i++)
    my_round(rc[i], lp->epsd);
}

short is_feasible(lprec *lp, REAL *values)
{
  int i, elmnr;
  REAL *this_rhs;
  REAL dist;

  if(lp->scaling_used) {
    for(i = lp->rows + 1; i <= lp->sum; i++)
      if(   values[i - lp->rows] < lp->orig_lowbo[i] * lp->scale[i]
         || values[i - lp->rows] > lp->orig_upbo[i]  * lp->scale[i])
        return(FALSE);
  }
  else {
    for(i = lp->rows + 1; i <= lp->sum; i++)
      if(   values[i - lp->rows] < lp->orig_lowbo[i]
         || values[i - lp->rows] > lp->orig_upbo[i])
        return(FALSE);
  }
  CALLOC(this_rhs, lp->rows + 1);
  if (lp->columns_scaled) {
    for(i = 1; i <= lp->columns; i++)
      for(elmnr = lp->col_end[i - 1]; elmnr < lp->col_end[i]; elmnr++)
        this_rhs[lp->mat[elmnr].row_nr] += lp->mat[elmnr].value * values[i] /
          lp->scale[lp->rows + i];
  }
  else {
    for(i = 1; i <= lp->columns; i++)
      for(elmnr = lp->col_end[i - 1]; elmnr < lp->col_end[i]; elmnr++)
        this_rhs[lp->mat[elmnr].row_nr] += lp->mat[elmnr].value * values[i];
  }
  for(i = 1; i <= lp->rows; i++) {
    dist = lp->orig_rh[i] - this_rhs[i];
    my_round(dist, 0.001); /* ugly constant, MB */
    if((lp->orig_upbo[i] == 0 && dist != 0) || dist < 0) {
      free(this_rhs);
      return(FALSE);
    }
  }
  free(this_rhs);
  return(TRUE);
}

/* fixed by Enrico Faggiolo */
short column_in_lp(lprec *lp, REAL *testcolumn)
{
  int i, j;
  int nz, ident;
  REAL value;

  for(nz = 0, i = 0; i <= lp->rows; i++)
    if(my_abs(testcolumn[i]) > lp->epsel) nz++;

  if(lp->scaling_used)
    for(i = 1; i <= lp->columns; i++) {
      ident = nz;
      for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++) {
        value = lp->mat[j].value;
        if(lp->ch_sign[lp->mat[j].row_nr])
          value = -value;
        value /= lp->scale[lp->rows + i];
        value /= lp->scale[lp->mat[j].row_nr];
        value -= testcolumn[lp->mat[j].row_nr];
        if(my_abs(value) > lp->epsel)
          break;
        ident--;
        if(ident == 0)
          return(TRUE);
      }
    }
  else
    for(i = 1; i <= lp->columns; i++) {
      ident = nz;
      for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++) {
        value = lp->mat[j].value;
        if(lp->ch_sign[lp->mat[j].row_nr])
          value = -value;
        value -= testcolumn[lp->mat[j].row_nr];
        if(my_abs(value) > lp->epsel)
          break;
        ident--;
        if(ident == 0)
          return(TRUE);
      }
    }
  return(FALSE);
}

void print_lp(lprec *lp)
{
  int i, j;
  REAL *fatmat;
  CALLOC(fatmat, (lp->rows + 1) * lp->columns);
  for(i = 1; i <= lp->columns; i++)
    for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++)
      fatmat[(i - 1) * (lp->rows + 1) + lp->mat[j].row_nr] = lp->mat[j].value;

  printf("problem name: %s\n", lp->lp_name);
  printf("          ");
  for(j = 1; j <= lp->columns; j++)
    if(lp->names_used)
      printf("%8s ", lp->col_name[j]);
    else
      printf("Var[%3d] ", j);
  if(lp->maximise)
    {
      printf("\nMaximise  ");
      for(j = 0; j < lp->columns; j++)
        printf("%8g ", (double) -fatmat[j * (lp->rows + 1)]);
    }
  else
    {
      printf("\nMinimize  ");
      for(j = 0; j < lp->columns; j++)
        printf("%8g ", (double) fatmat[j * (lp->rows + 1)]);
    }
  printf("\n");
  for(i = 1; i <= lp->rows; i++) {
    if(lp->names_used)
      printf("%-9s ", lp->row_name[i]);
    else
      printf("Row[%3d]  ", i);
    for(j = 0; j < lp->columns; j++)
      if(lp->ch_sign[i] && fatmat[j*(lp->rows + 1) + i] != 0)
        printf("%8g ", (double)-fatmat[j * (lp->rows+1) + i]);
      else
        printf("%8g ", (double)fatmat[j * (lp->rows + 1) + i]);
    if(lp->orig_upbo[i] != 0) {
      if(lp->ch_sign[i])
        printf(">= ");
      else
        printf("<= ");
    }
    else
      printf(" = ");
    if(lp->ch_sign[i])
      printf("%8g", (double)-lp->orig_rh[i]);
    else
      printf("%8g", (double)lp->orig_rh[i]);
    if(lp->orig_lowbo[i] != 0) {
      printf("  %s = %8g", (lp->ch_sign[i]) ? "lowbo" : "upbo",
             (double)lp->orig_lowbo[i]);
    }
    if((lp->orig_upbo[i] != lp->infinite) && (lp->orig_upbo[i] != 0.0)) {
      printf("  %s = %8g", (lp->ch_sign[i]) ? "upbo" : "lowbo",
             (double)lp->orig_upbo[i]);
    }
    printf("\n");
  }
  printf("Type      ");
  for(i = 1; i <= lp->columns; i++)
    if(lp->must_be_int[lp->rows + i] == TRUE)
      printf("     Int ");
    else
      printf("    Real ");
  printf("\nupbo      ");
  for(i = 1; i <= lp->columns; i++)
    if(lp->orig_upbo[lp->rows + i] == lp->infinite)
      printf(" Infinite");
    else
      printf("%8g ", (double)lp->orig_upbo[lp->rows + i]);
  printf("\nlowbo     ");
  for(i = 1; i <= lp->columns; i++)
    printf("%8g ", (double)lp->orig_lowbo[lp->rows + i]);
  printf("\n");
  for(i = 0; i < lp->nr_lagrange; i++) {
    printf("lag[%d]  ", i);
    for(j = 1; j <= lp->columns; j++)
      printf("%8g ", (double)lp->lag_row[i][j]);
    if(lp->orig_upbo[i] == lp->infinite) {
      if(lp->lag_con_type[i] == GE)
        printf(">= ");
      else if(lp->lag_con_type[i] == LE)
        printf("<= ");
      else if(lp->lag_con_type[i] == EQ)
        printf(" = ");
    }
    printf("%8g\n", (double)lp->lag_rhs[i]);
  }

  free(fatmat);
}

void set_row_name(lprec *lp, int row, nstring new_name)
{
  int i;
  hashelem *hp;

  if(!lp->names_used) {
    CALLOC(lp->row_name, lp->rows_alloc + 1);
    CALLOC(lp->col_name, lp->columns_alloc + 1);
    lp->names_used = TRUE;
    for(i = 0; i <= lp->rows; i++)
      sprintf(lp->row_name[i], "r_%d", i);
    for(i = 1; i <= lp->columns; i++)
      sprintf(lp->col_name[i], "var_%d", i);
  }
  strcpy(lp->row_name[row], new_name);
  hp = puthash(lp->row_name[row], lp->rowname_hashtab);
  hp->index = row;
}

void set_col_name(lprec *lp, int column, nstring new_name)
{
  int i;
  hashelem *hp;

  if(!lp->names_used) {
    CALLOC(lp->row_name, lp->rows_alloc + 1);
    CALLOC(lp->col_name, lp->columns_alloc + 1);
    lp->names_used = TRUE;
    for(i = 0; i <= lp->rows; i++)
      sprintf(lp->row_name[i], "r_%d", i);
    for(i = 1; i <= lp->columns; i++)
      sprintf(lp->col_name[i], "var_%d", i);
  }
  strcpy(lp->col_name[column], new_name);
  hp = puthash(lp->col_name[column], lp->colname_hashtab);
  hp->index = column;
}

static REAL minmax_to_scale(REAL min, REAL max)
{
  REAL scale;

  /* should do something sensible when min or max is 0, MB */
  if((min == 0) || (max == 0))
    return((REAL)1);

  /* scale = 1 / pow(10, (log10(min) + log10(max)) / 2); */
  /* Jon van Reet noticed: can be simplified to: */
  scale = 1 / sqrt(min * max);

  return(scale);
}

void unscale_columns(lprec *lp)
{
  int i, j;

  /* unscale mat */
  for(j = 1; j <= lp->columns; j++)
    for(i = lp->col_end[j - 1]; i < lp->col_end[j]; i++)
      lp->mat[i].value /= lp->scale[lp->rows + j];

  /* unscale bounds as well */
  for(i = lp->rows + 1; i <= lp->sum; i++) { /* was < */ /* changed by PN */
    if(lp->orig_lowbo[i] != 0)
      lp->orig_lowbo[i] *= lp->scale[i];
    if(lp->orig_upbo[i] != lp->infinite)
      lp->orig_upbo[i] *= lp->scale[i];
  }

  for(i = lp->rows + 1; i<= lp->sum; i++)
    lp->scale[i] = 1;
  lp->columns_scaled = FALSE;
  lp->eta_valid = FALSE;
}

void unscale(lprec *lp)
{
  int i, j;

  if(lp->scaling_used) {
    /* unscale mat */
    for(j = 1; j <= lp->columns; j++)
      for(i = lp->col_end[j - 1]; i < lp->col_end[j]; i++)
        lp->mat[i].value /= lp->scale[lp->rows + j];

    /* unscale bounds */
    for(i = lp->rows + 1; i <= lp->sum; i++) { /* was < */ /* changed by PN */
      if(lp->orig_lowbo[i] != 0)
        lp->orig_lowbo[i] *= lp->scale[i];
      if(lp->orig_upbo[i] != lp->infinite)
        lp->orig_upbo[i] *= lp->scale[i];
    }

    /* unscale the matrix */
    for(j = 1; j <= lp->columns; j++)
      for(i = lp->col_end[j-1]; i < lp->col_end[j]; i++)
        lp->mat[i].value /= lp->scale[lp->mat[i].row_nr];

    /* unscale the rhs! */
    for(i = 0; i <= lp->rows; i++)
      lp->orig_rh[i] /= lp->scale[i];

    /* and don't forget to unscale the upper and lower bounds ... */
    for(i = 0; i <= lp->rows; i++) {
      if(lp->orig_lowbo[i] != 0)
        lp->orig_lowbo[i] /= lp->scale[i];
      if(lp->orig_upbo[i] != lp->infinite)
        lp->orig_upbo[i] /= lp->scale[i];
    }

    free(lp->scale);
    lp->scaling_used = FALSE;
    lp->eta_valid = FALSE;
  }
}


void auto_scale(lprec *lp)
{
  int i, j, row_nr;
  REAL *row_max, *row_min, *scalechange, absval;
  REAL col_max, col_min;

  if(!lp->scaling_used) {
    MALLOC(lp->scale, lp->sum_alloc + 1);
    for(i = 0; i <= lp->sum; i++)
      lp->scale[i] = 1;
  }

  MALLOC(row_max, lp->rows + 1);
  MALLOC(row_min, lp->rows + 1);
  MALLOC(scalechange, lp->sum + 1);

  /* initialise min and max values */
  for(i = 0; i <= lp->rows; i++) {
    row_max[i] = 0;
    row_min[i] = lp->infinite;
  }

  /* calculate min and max absolute values of rows */
  for(j = 1; j <= lp->columns; j++)
    for(i = lp->col_end[j - 1]; i < lp->col_end[j]; i++) {
      row_nr = lp->mat[i].row_nr;
      absval = my_abs(lp->mat[i].value);
      if(absval != 0) {
        row_max[row_nr] = my_max(row_max[row_nr], absval);
        row_min[row_nr] = my_min(row_min[row_nr], absval);
      }
    }
  /* calculate scale factors for rows */
  for(i = 0; i <= lp->rows; i++) {
    scalechange[i] = minmax_to_scale(row_min[i], row_max[i]);
    lp->scale[i] *= scalechange[i];
  }

  /* now actually scale the matrix */
  for(j = 1; j <= lp->columns; j++)
    for(i = lp->col_end[j - 1]; i < lp->col_end[j]; i++)
      lp->mat[i].value *= scalechange[lp->mat[i].row_nr];

  /* and scale the rhs and the row bounds (RANGES in MPS!!) */
  for(i = 0; i <= lp->rows; i++) {
    lp->orig_rh[i] *= scalechange[i];

    if((lp->orig_upbo[i] < lp->infinite) && (lp->orig_upbo[i] != 0))
      lp->orig_upbo[i] *= scalechange[i];

    if(lp->orig_lowbo[i] != 0)
      lp->orig_lowbo[i] *= scalechange[i];
  }

  free(row_max);
  free(row_min);

  /* calculate column scales */
  for(j = 1; j <= lp->columns; j++) {
    if(lp->must_be_int[lp->rows + j]) { /* do not scale integer columns */
      scalechange[lp->rows + j] = 1;
    }
    else {
      col_max = 0;
      col_min = lp->infinite;
      for(i = lp->col_end[j - 1]; i < lp->col_end[j]; i++) {
        if(lp->mat[i].value != 0) {
          col_max = my_max(col_max, my_abs(lp->mat[i].value));
          col_min = my_min(col_min, my_abs(lp->mat[i].value));
        }
      }
      scalechange[lp->rows + j]  = minmax_to_scale(col_min, col_max);
      lp->scale[lp->rows + j] *= scalechange[lp->rows + j];
    }
  }

  /* scale mat */
  for(j = 1; j <= lp->columns; j++)
    for(i = lp->col_end[j - 1]; i < lp->col_end[j]; i++)
      lp->mat[i].value *= scalechange[lp->rows + j];

  /* scale bounds as well */
  for(i = lp->rows + 1; i <= lp->sum; i++) { /* was < *//* changed by PN */
    if(lp->orig_lowbo[i] != 0)
      lp->orig_lowbo[i] /= scalechange[i];
    if(lp->orig_upbo[i] != lp->infinite)
      lp->orig_upbo[i] /= scalechange[i];
  }
  lp->columns_scaled = TRUE;

  free(scalechange);
  lp->scaling_used = TRUE;
  lp->eta_valid = FALSE;
}

void reset_basis(lprec *lp)
{
  lp->basis_valid = FALSE;
}

void print_solution(lprec *lp)
{
  int i;
  FILE *stream;

  stream = stdout;

  fprintf(stream, "Value of objective function: %g\n",
          (double)lp->best_solution[0]);

  /* print normal variables */
  for(i = 1; i <= lp->columns; i++)
    if(lp->names_used)
      fprintf(stream, "%-20s %g\n", lp->col_name[i],
              (double)lp->best_solution[lp->rows + i]);
    else
      fprintf(stream, "Var [%d] %g\n", i,
              (double)lp->best_solution[lp->rows + i]);

  /* print achieved constraint values */
  if(lp->verbose) {
    fprintf(stream, "\nActual values of the constraints:\n");
    for(i = 1; i <= lp->rows; i++)
      if(lp->names_used)
        fprintf(stream, "%-20s %g\n", lp->row_name[i],
                (double)lp->best_solution[i]);
      else
        fprintf(stream, "Row [%d] %g\n", i,
                (double)lp->best_solution[i]);
  }

  if((lp->verbose || lp->print_duals)) {
    if(lp->max_level != 1)
      fprintf(stream,
              "These are the duals from the node that gave the optimal solution.\n");
    else
      fprintf(stream, "\nDual values:\n");
    for(i = 1; i <= lp->rows; i++)
      if(lp->names_used)
        fprintf(stream, "%-20s %g\n", lp->row_name[i],
                (double)lp->duals[i]);
      else
        fprintf(stream, "Row [%d] %g\n", i, (double)lp->duals[i]);
  }
  fflush(stream);
} /* Printsolution */

void write_LP(lprec *lp, FILE *output)
{
  int i, j;
  REAL *row;

  MALLOC(row, lp->columns+1);
  if(lp->maximise)
    fprintf(output, "max:");
  else
    fprintf(output, "min:");

  get_row(lp, 0, row);
  for(i = 1; i <= lp->columns; i++)
    if(row[i] != 0) {
      if(row[i] == -1)
        fprintf(output, " -");
      else if(row[i] == 1)
        fprintf(output, " +");
      else
        fprintf(output, " %+g ", (double)row[i]);
      if(lp->names_used)
        fprintf(output, "%s", lp->col_name[i]);
      else
        fprintf(output, "x%d", i);
    }
  fprintf(output, ";\n");

  for(j = 1; j <= lp->rows; j++) {
    if(lp->names_used)
      fprintf(output, "%s:", lp->row_name[j]);
    get_row(lp, j, row);
    for(i = 1; i <= lp->columns; i++)
      if(row[i] != 0) {
        if(row[i] == -1)
          fprintf(output, " -");
        else if(row[i] == 1)
          fprintf(output, " +");
        else
          fprintf(output, " %+g ", (double)row[i]);
        if(lp->names_used)
          fprintf(output, "%s", lp->col_name[i]);
        else
          fprintf(output, "x%d", i);
      }
    if(lp->orig_upbo[j] == 0)
      fprintf(output, " =");
    else if(lp->ch_sign[j])
      fprintf(output, " >");
    else
      fprintf(output, " <");
    if(lp->ch_sign[j])
      fprintf(output, " %16g;\n", (double)-lp->orig_rh[j]);
    else
      fprintf(output, " %16g;\n", (double)lp->orig_rh[j]);
  }
  for(i = lp->rows + 1; i <= lp->sum; i++) {
    if(lp->orig_lowbo[i] != 0) {
      if(lp->names_used)
        fprintf(output, "%s > %16g;\n", lp->col_name[i - lp->rows],
                (double)lp->orig_lowbo[i]);
      else
        fprintf(output, "x%d > %16g;\n", i - lp->rows,
                (double)lp->orig_lowbo[i]);
    }
    if(lp->orig_upbo[i] != lp->infinite) {
      if(lp->names_used)
        fprintf(output, "%s < %16g;\n", lp->col_name[i - lp->rows],
                (double)lp->orig_upbo[i]);
      else
        fprintf(output, "x%d < %16g;\n", i - lp->rows,
                (double)lp->orig_upbo[i]);
    }
  }


  i = 1;
  while(!lp->must_be_int[lp->rows + i]  && i <= lp->columns)
    i++;
  if(i <= lp->columns) {
    if(lp->names_used)
      fprintf(output, "\nint %s", lp->col_name[i]);
    else
      fprintf(output, "\nint x%d", i);
    i++;
    for(; i <= lp->columns; i++)
      if(lp->must_be_int[lp->rows + i]) {
        if(lp->names_used)
          fprintf(output, ",%s", lp->col_name[i]);
        else
          fprintf(output, ", x%d", i);
      }
    fprintf(output, ";\n");
  }
  free(row);
}




void write_MPS(lprec *lp, FILE *output)
{
  int i, j, marker, putheader;
  REAL *column, a;


  MALLOC(column, lp->rows + 1);
  marker = 0;
  fprintf(output, "NAME          %s\n", lp->lp_name);
  fprintf(output, "ROWS\n");
  for(i = 0; i <= lp->rows; i++) {
    if(i == 0)
      fprintf(output, " N  ");
    else
      if(lp->orig_upbo[i] != 0) {
        if(lp->ch_sign[i])
          fprintf(output, " G  ");
        else
          fprintf(output, " L  ");
      }
      else
        fprintf(output, " E  ");
    if(lp->names_used)
      fprintf(output, "%s\n", lp->row_name[i]);
    else
      fprintf(output, "r_%d\n", i);
  }

  fprintf(output, "COLUMNS\n");

  for(i = 1; i <= lp->columns; i++) {
    if((lp->must_be_int[i + lp->rows]) && (marker % 2) == 0) {
      fprintf(output,
              "    MARK%04d  'MARKER'                 'INTORG'\n",
              marker);
      marker++;
    }
    if((!lp->must_be_int[i + lp->rows]) && (marker % 2) == 1) {
      fprintf(output,
              "    MARK%04d  'MARKER'                 'INTEND'\n",
              marker);
      marker++;
    }
    /* this gets slow for large LP problems. Implement a sparse version? */
    get_column(lp, i, column);
    j = 0;
    if(lp->maximise) {
      if(column[j] != 0) {
        if(lp->names_used)
          fprintf(output, "    %-8s  %-8s  %12g\n", lp->col_name[i],
                  lp->row_name[j], (double)-column[j]);
        else
          fprintf(output, "    var_%-4d  r_%-6d  %12g\n", i, j,
                  (double)-column[j]);
      }
    }
    else {
      if(column[j] != 0) {
        if(lp->names_used)
          fprintf(output, "    %-8s  %-8s  %12g\n", lp->col_name[i],
                  lp->row_name[j], (double)column[j]);
        else
          fprintf(output, "    var_%-4d  r_%-6d  %12g\n", i, j,
                  (double)column[j]);
      }
    }
    for(j = 1; j <= lp->rows; j++)
      if(column[j] != 0) {
        if(lp->names_used)
          fprintf(output, "    %-8s  %-8s  %12g\n", lp->col_name[i],
                  lp->row_name[j], (double)column[j]);
        else
          fprintf(output, "    var_%-4d  r_%-6d  %12g\n", i, j,
                  (double)column[j]);
      }
  }
  if((marker % 2) == 1) {
    fprintf(output, "    MARK%04d  'MARKER'                 'INTEND'\n",
            marker);
    /* marker++; */ /* marker not used after this */
  }

  fprintf(output, "RHS\n");
  for(i = 1; i <= lp->rows; i++) {
    a = lp->orig_rh[i];
    if(lp->scaling_used)
      a /= lp->scale[i];

    if(lp->ch_sign[i]) {
      if(lp->names_used)
        fprintf(output, "    RHS       %-8s  %12g\n", lp->row_name[i],
                (double)-a);
      else
        fprintf(output, "    RHS       r_%-6d  %12g\n", i, (double)-a);
    }
    else {
      if(lp->names_used)
        fprintf(output, "    RHS       %-8s  %12g\n", lp->row_name[i],
                (double)a);
      else
        fprintf(output, "    RHS       r_%-6d  %12g\n", i, (double)a);
    }
  }

  putheader = TRUE;
  for(i = 1; i <= lp->rows; i++)
    if((lp->orig_upbo[i] != lp->infinite) && (lp->orig_upbo[i] != 0.0)) {
      if(putheader) {
        fprintf(output, "RANGES\n");
        putheader = FALSE;
      }
      a = lp->orig_upbo[i];
      if(lp->scaling_used)
        a /= lp->scale[i];
      if(lp->names_used)
        fprintf(output, "    RGS       %-8s  %12g\n", lp->row_name[i],
                (double)a);
      else
        fprintf(output, "    RGS       r_%-6d  %12g\n", i,
                (double)a);
    }
    else if((lp->orig_lowbo[i] != 0.0)) {
      if(putheader) {
        fprintf(output, "RANGES\n");
        putheader = FALSE;
      }
      a = lp->orig_lowbo[i];
      if(lp->scaling_used)
        a /= lp->scale[i];
      if(lp->names_used)
        fprintf(output, "    RGS       %-8s  %12g\n", lp->row_name[i],
                (double)-a);
      else
        fprintf(output, "    RGS       r_%-6d  %12g\n", i,
                (double)-a);
    }

  fprintf(output, "BOUNDS\n");
  if(lp->names_used)
    for(i = lp->rows + 1; i <= lp->sum; i++) {
      if((lp->orig_lowbo[i] != 0) && (lp->orig_upbo[i] < lp->infinite) &&
         (lp->orig_lowbo[i] == lp->orig_upbo[i])) {
        a = lp->orig_upbo[i];
        if(lp->scaling_used)
          a *= lp->scale[i];
        fprintf(output, " FX BND       %-8s  %12g\n",
                lp->col_name[i - lp->rows], (double)a);
      }
      else {
        if(lp->orig_upbo[i] < lp->infinite) {
          a = lp->orig_upbo[i];
          if(lp->scaling_used)
            a *= lp->scale[i];
          fprintf(output, " UP BND       %-8s  %12g\n",
                  lp->col_name[i - lp->rows], (double)a);
        }
        if(lp->orig_lowbo[i] != 0) {
          a = lp->orig_lowbo[i];
          if(lp->scaling_used)
            a *= lp->scale[i];
          /* bug? should a be used instead of lp->orig_lowbo[i] MB */
          fprintf(output, " LO BND       %-8s  %12g\n",
                  lp->col_name[i - lp->rows], (double)lp->orig_lowbo[i]);
        }
      }
    }
  else
    for(i = lp->rows + 1; i <= lp->sum; i++) {
      if((lp->orig_lowbo[i] != 0) && (lp->orig_upbo[i] < lp->infinite) &&
         (lp->orig_lowbo[i] == lp->orig_upbo[i])) {
        a = lp->orig_upbo[i];
        if(lp->scaling_used)
          a *= lp->scale[i];
        fprintf(output, " FX BND       %-8s  %12g\n",
                lp->col_name[i - lp->rows], (double)a);
      }
      else {
        if(lp->orig_upbo[i] < lp->infinite) {
          a = lp->orig_upbo[i];
          if(lp->scaling_used)
            a *= lp->scale[i];
          fprintf(output, " UP BND       var_%-4d  %12g\n",
                  i - lp->rows, (double)a);
        }
        if(lp->orig_lowbo[i] != 0) {
          a = lp->orig_lowbo[i];
          if(lp->scaling_used)
            a *= lp->scale[i];
          fprintf(output, " LO BND       var_%-4d  %12g\n", i - lp->rows,
                  (double)a);
        }
      }
    }
  fprintf(output, "ENDATA\n");
  free(column);
}

void print_duals(lprec *lp)
{
  int i;
  for(i = 1; i <= lp->rows; i++)
    if(lp->names_used)
      fprintf(stdout, "%s [%d] %g\n", lp->row_name[i], i,
              (double)lp->duals[i]);
    else
      fprintf(stdout, "Dual [%d] %g\n", i, (double)lp->duals[i]);
}

void print_scales(lprec *lp)
{
  int i;
  if(lp->scaling_used) {
    for(i = 0; i <= lp->rows; i++)
      fprintf(stdout, "Row[%d]    scaled at %g\n", i,
              (double)lp->scale[i]);
    for(i = 1; i <= lp->columns; i++)
      fprintf(stdout, "Column[%d] scaled at %g\n", i,
              (double)lp->scale[lp->rows + i]);
  }
}


/***************************** hash.c ********************************/

/* #include "lpkit.h" */ /* only for MALLOC, CALLOC */
/* #include <string.h> */
#include <limits.h>

/* hash functions for open hashing */

hashtable *create_hash_table(int size)
{
  hashtable *ht;

  MALLOC(ht, 1);
  CALLOC(ht->table, size);
  ht->size = size;
  return(ht);
}

void free_hash_table(hashtable *ht)
{
  int i;
  hashelem *hp, *thp;

  for(i = 0; i < ht->size; i++) {
    hp = ht->table[i];
    while(hp != NULL) {
      thp = hp;
      hp = hp->next;
      free(thp->name);
      free(thp);
    }
  }
  free(ht->table);
  free(ht);
}

/* make a good hash function for any int size */
/* inspired by Aho, Sethi and Ullman, Compilers ..., p436 */
#define HASH_1 sizeof(unsigned int)
#define HASH_2 (sizeof(unsigned int) * 6)
#define HASH_3 (((unsigned int)0xF0) << ((sizeof(unsigned int) - 1) * CHAR_BIT))

static int hashval(const char *string, int size)
{
  unsigned int result = 0, tmp;

  for(; *string; string++) {
    result = (result << HASH_1) + *string;
    if((tmp = result & HASH_3) != 0) {
      /* if any of the most significant bits is on */
      result ^= tmp >> HASH_2; /* xor them in in a less significant part */
      result ^= tmp; /* and reset the most significant bits to 0 */
    }
  }
  return(result % size);
} /* hashval */

hashelem *findhash(const char *name, hashtable *ht)
{
  hashelem *h_tab_p;
  for(h_tab_p = ht->table[hashval(name, ht->size)];
      h_tab_p != NULL;
      h_tab_p = h_tab_p->next)
    if(strcmp(name, h_tab_p->name) == 0) /* got it! */
      break;
  return(h_tab_p);
} /* gethash */

hashelem *puthash(const char *name, hashtable *ht)
{
  hashelem *hp;
  int index;

  if((hp = findhash(name, ht)) == NULL) {
    index = hashval(name, ht->size);
    CALLOC(hp, 1);
    MALLOC(hp->name, strlen(name) + 1);
    strcpy(hp->name, name);
    hp->next = ht->table[index];
    ht->table[index] = hp;
  }
  return(hp);
}

hashtable *copy_hash_table(hashtable *ht)
{
  hashtable *copy;
  hashelem *elem, *new_elem;
  int i;

  copy = create_hash_table(ht->size);

  for(i = 0; i < ht->size; i++) {
    for(elem = ht->table[i]; elem != NULL; elem = elem->next) {
      CALLOC(new_elem, 1);
      /* copy entire struct */
      *new_elem = *elem;
      /* ... but link it into the new list */
      new_elem->next = copy->table[i];
      copy->table[i] = new_elem;
    }
  }

  return(copy);
}

/***************************** presolve.c ****************************/

/* #include "lpkit.h" */


void presolve(lprec *lp)
{
  fprintf(stderr, "Entering presolve\n");


}

/***************************** debug.c *******************************/

/* #include "lpkit.h" */
/* #include "lpglob.h" */
/* #include <stdarg.h> */


static void print_indent(void)
{
  int i;

  fprintf(stderr, "%2d", Level);
  if(Level < 50) /* useless otherwise */
    for(i = Level; i > 0; i--)
      fprintf(stderr, "--");
  else
    fprintf(stderr, " *** too deep ***");
  fprintf(stderr, "> ");
} /* print_indent */


void debug_print_solution(lprec *lp)
{
  int i;

  if(lp->debug)
    for (i = lp->rows + 1; i <= lp->sum; i++) {
      print_indent();
      if (lp->names_used)
        fprintf(stderr, "%s %g\n", lp->col_name[i - lp->rows],
                (double)lp->solution[i]);
      else
        fprintf(stderr, "Var [%d] %g\n", i - lp->rows,
                (double)lp->solution[i]);
    }
} /* debug_print_solution */


void debug_print_bounds(lprec *lp, REAL *upbo, REAL *lowbo)
{
  int i;

  if(lp->debug)
    for(i = lp->rows + 1; i <= lp->sum; i++) {
      if(lowbo[i] == upbo[i]) {
        print_indent();
        if (lp->names_used)
          fprintf(stderr, "%s = %g\n", lp->col_name[i - lp->rows],
                  (double)lowbo[i]);
        else
          fprintf(stderr, "Var [%d]  = %g\n", i - lp->rows,
                  (double)lowbo[i]);
      }
      else {
        if(lowbo[i] != 0) {
          print_indent();
          if (lp->names_used)
            fprintf(stderr, "%s > %g\n", lp->col_name[i - lp->rows],
                    (double)lowbo[i]);
          else
            fprintf(stderr, "Var [%d]  > %g\n", i - lp->rows,
                    (double)lowbo[i]);
        }
        if(upbo[i] != lp->infinite) {
          print_indent();
          if (lp->names_used)
            fprintf(stderr, "%s < %g\n", lp->col_name[i - lp->rows],
                    (double)upbo[i]);
          else
            fprintf(stderr, "Var [%d]  < %g\n", i - lp->rows,
                    (double)upbo[i]);
        }
      }
    }
} /* debug_print_bounds */


void debug_print(lprec *lp, char *format, ...)
{
  va_list ap;

  if(lp->debug) {
    va_start(ap, format);
    print_indent();
    vfprintf(stderr, format, ap);
    fputc('\n', stderr);
    va_end(ap);
  }
} /* debug_print */


/******************************** solve.c ******************************/

/* #include <string.h> */
/* #include "lpkit.h" */
/* #include "lpglob.h" */
/* #include "debug.h" */

/* Globals used by solver */
static short JustInverted;
static short Status;
static short Doiter;
static short DoInvert;
static short Break_bb;


static void ftran(lprec *lp, REAL *pcol)
{
  int  i, j, k, r, *rowp;
  REAL theta, *valuep;

  for(i = 1; i <= lp->eta_size; i++) {
    k = lp->eta_col_end[i] - 1;
    r = lp->eta_row_nr[k];
    theta = pcol[r];
    if(theta != 0) {
      j = lp->eta_col_end[i - 1];

      /* CPU intensive loop, let's do pointer arithmetic */
      for(rowp = lp->eta_row_nr + j, valuep = lp->eta_value + j;
          j < k;
          j++, rowp++, valuep++)
        pcol[*rowp] += theta * *valuep;

      pcol[r] *= lp->eta_value[k];
    }
  }

  /* round small values to zero */
  for(i = 0; i <= lp->rows; i++)
    my_round(pcol[i], lp->epsel);
} /* ftran */


void btran(lprec *lp, REAL *row)
{
  int  i, j, k, *rowp;
  REAL f, *valuep;

  for(i = lp->eta_size; i >= 1; i--) {
    f = 0;
    k = lp->eta_col_end[i] - 1;
    j = lp->eta_col_end[i - 1];

    for(rowp = lp->eta_row_nr + j, valuep = lp->eta_value + j;
        j <= k;
        j++, rowp++, valuep++)
      f += row[*rowp] * *valuep;

    my_round(f, lp->epsel);
    row[lp->eta_row_nr[k]] = f;
  }
} /* btran */


static short isvalid(lprec *lp)
{
  int i, j, *rownum, *colnum;
  int *num, row_nr;

  if(!lp->row_end_valid) {
    MALLOC(num, lp->rows + 1);
    MALLOC(rownum, lp->rows + 1);

    for(i = 0; i <= lp->rows; i++) {
      num[i] = 0;
      rownum[i] = 0;
    }

    for(i = 0; i < lp->non_zeros; i++)
      rownum[lp->mat[i].row_nr]++;

    lp->row_end[0] = 0;

    for(i = 1; i <= lp->rows; i++)
      lp->row_end[i] = lp->row_end[i - 1] + rownum[i];

    for(i = 1; i <= lp->columns; i++)
      for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++) {
        row_nr = lp->mat[j].row_nr;
        if(row_nr != 0) {
          num[row_nr]++;
          lp->col_no[lp->row_end[row_nr - 1] + num[row_nr]] = i;
        }
      }

    free(num);
    free(rownum);
    lp->row_end_valid = TRUE;
  }

  if(lp->valid)
    return(TRUE);

  CALLOC(rownum, lp->rows + 1);
  CALLOC(colnum, lp->columns + 1);

  for(i = 1 ; i <= lp->columns; i++)
    for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++) {
      colnum[i]++;
      rownum[lp->mat[j].row_nr]++;
    }

  for(i = 1; i <= lp->columns; i++)
    if(colnum[i] == 0) {
      if(lp->names_used)
        fprintf(stderr, "Warning: Variable %s not used in any constraints\n",
                lp->col_name[i]);
      else
        fprintf(stderr, "Warning: Variable %d not used in any constraints\n",
                i);
    }
  free(rownum);
  free(colnum);
  lp->valid = TRUE;
  return(TRUE);
}

static void resize_eta(lprec *lp)
{
  lp->eta_alloc *= 1.5;
  REALLOC(lp->eta_value, lp->eta_alloc);
  REALLOC(lp->eta_row_nr, lp->eta_alloc);
  /* fprintf(stderr, "resized eta to size %d\n", lp->eta_alloc); */
} /* resize_eta */


static void condensecol(lprec *lp,
                        int row_nr,
                        REAL *pcol)
{
  int i, elnr;

  elnr = lp->eta_col_end[lp->eta_size];

  if(elnr + lp->rows + 2 >= lp->eta_alloc) /* maximum local growth of Eta */
    resize_eta(lp);

  for(i = 0; i <= lp->rows; i++)
    if(i != row_nr && pcol[i] != 0) {
      lp->eta_row_nr[elnr] = i;
      lp->eta_value[elnr] = pcol[i];
      elnr++;
    }

  lp->eta_row_nr[elnr] = row_nr;
  lp->eta_value[elnr] = pcol[row_nr];
  elnr++;
  lp->eta_col_end[lp->eta_size + 1] = elnr;
} /* condensecol */


static void addetacol(lprec *lp)
{
  int  i, j, k;
  REAL theta;

  j = lp->eta_col_end[lp->eta_size];
  lp->eta_size++;
  k = lp->eta_col_end[lp->eta_size] - 1;
  theta = 1 / (REAL) lp->eta_value[k];
  lp->eta_value[k] = theta;
  for(i = j; i < k; i++)
    lp->eta_value[i] *= -theta;
  JustInverted = FALSE;
} /* addetacol */


static void setpivcol(lprec *lp,
                      short lower,
                      int   varin,
                      REAL *pcol)
{
  int  i, colnr;

  for(i = 0; i <= lp->rows; i++)
    pcol[i] = 0;

  if(lower) {
    if(varin > lp->rows) {
      colnr = varin - lp->rows;
      for(i = lp->col_end[colnr - 1]; i < lp->col_end[colnr]; i++)
        pcol[lp->mat[i].row_nr] = lp->mat[i].value;
      pcol[0] -= Extrad;
    }
    else
      pcol[varin] = 1;
  }
  else { /* !lower */
    if(varin > lp->rows) {
      colnr = varin - lp->rows;
      for(i = lp->col_end[colnr - 1]; i < lp->col_end[colnr]; i++)
        pcol[lp->mat[i].row_nr] = -lp->mat[i].value;
      pcol[0] += Extrad;
    }
    else
      pcol[varin] = -1;
  }

  ftran(lp, pcol);
} /* setpivcol */


static void minoriteration(lprec *lp,
                           int colnr,
                           int row_nr)
{
  int  i, j, k, wk, varin, varout, elnr;
  REAL piv = 0, theta;

  varin = colnr + lp->rows;
  elnr = lp->eta_col_end[lp->eta_size];
  wk = elnr;
  lp->eta_size++;

  if(Extrad != 0) {
    lp->eta_row_nr[elnr] = 0;
    lp->eta_value[elnr] = -Extrad;
    elnr++;
    if(elnr >= lp->eta_alloc)
      resize_eta(lp);
  }

  for(j = lp->col_end[colnr - 1] ; j < lp->col_end[colnr]; j++) {
    k = lp->mat[j].row_nr;

    if(k == 0 && Extrad != 0)
      lp->eta_value[lp->eta_col_end[lp->eta_size - 1]] += lp->mat[j].value;
    else if(k != row_nr) {
      lp->eta_row_nr[elnr] = k;
      lp->eta_value[elnr] = lp->mat[j].value;
      elnr++;
      if(elnr >= lp->eta_alloc)
        resize_eta(lp);
    }
    else
      piv = lp->mat[j].value;
  }

  lp->eta_row_nr[elnr] = row_nr;
  lp->eta_value[elnr] = 1 / piv;
  theta = lp->rhs[row_nr] / piv;
  lp->rhs[row_nr] = theta;

  for(i = wk; i < elnr; i++)
    lp->rhs[lp->eta_row_nr[i]] -= theta * lp->eta_value[i];

  varout = lp->bas[row_nr];
  lp->bas[row_nr] = varin;
  lp->basis[varout] = FALSE;
  lp->basis[varin] = TRUE;

  for(i = wk; i < elnr; i++)
    lp->eta_value[i] /= -piv;

  lp->eta_col_end[lp->eta_size] = elnr + 1;
} /* minoriteration */


static void rhsmincol(lprec *lp,
                      REAL theta,
                      int row_nr,
                      int varin)
{
  int  i, j, k, varout;
  REAL f;

  if(row_nr > lp->rows + 1) {
    fprintf(stderr, "Error: rhsmincol called with row_nr: %d, rows: %d\n",
            row_nr, lp->rows);
    fprintf(stderr, "This indicates numerical instability\n");
    exit(EXIT_FAILURE);
  }

  j = lp->eta_col_end[lp->eta_size];
  k = lp->eta_col_end[lp->eta_size + 1];
  for(i = j; i < k; i++) {
    f = lp->rhs[lp->eta_row_nr[i]] - theta * lp->eta_value[i];
    my_round(f, lp->epsb);
    lp->rhs[lp->eta_row_nr[i]] = f;
  }

  lp->rhs[row_nr] = theta;
  varout = lp->bas[row_nr];
  lp->bas[row_nr] = varin;
  lp->basis[varout] = FALSE;
  lp->basis[varin] = TRUE;
} /* rhsmincol */


void invert(lprec *lp)
{
  int    i, j, v, wk, numit, varnr, row_nr, colnr, varin;
  REAL   theta;
  REAL   *pcol;
  short  *frow;
  short  *fcol;
  int    *rownum, *col, *row;
  int    *colnum;

  if(lp->print_at_invert)
    fprintf(stderr, "Start Invert iter %d eta_size %d rhs[0] %g \n",
            lp->iter, lp->eta_size, (double) - lp->rhs[0]);

  CALLOC(rownum, lp->rows + 1);
  CALLOC(col, lp->rows + 1);
  CALLOC(row, lp->rows + 1);
  CALLOC(pcol, lp->rows + 1);
  CALLOC(frow, lp->rows + 1);
  CALLOC(fcol, lp->columns + 1);
  CALLOC(colnum, lp->columns + 1);

  for(i = 0; i <= lp->rows; i++)
    frow[i] = TRUE;

  for(i = 0; i < lp->columns; i++)
    fcol[i] = FALSE;

  for(i = 0; i < lp->rows; i++)
    rownum[i] = 0;

  for(i = 0; i <= lp->columns; i++)
    colnum[i] = 0;

  for(i = 0; i <= lp->rows; i++)
    if(lp->bas[i] > lp->rows)
      fcol[lp->bas[i] - lp->rows - 1] = TRUE;
    else
      frow[lp->bas[i]] = FALSE;

  for(i = 1; i <= lp->rows; i++)
    if(frow[i])
      for(j = lp->row_end[i - 1] + 1; j <= lp->row_end[i]; j++) {
        wk = lp->col_no[j];
        if(fcol[wk - 1]) {
          colnum[wk]++;
          rownum[i - 1]++;
        }
      }

  for(i = 1; i <= lp->rows; i++)
    lp->bas[i] = i;

  for(i = 1; i <= lp->rows; i++)
    lp->basis[i] = TRUE;

  for(i = 1; i <= lp->columns; i++)
    lp->basis[i + lp->rows] = FALSE;

  for(i = 0; i <= lp->rows; i++)
    lp->rhs[i] = lp->rh[i];

  for(i = 1; i <= lp->columns; i++) {
    varnr = lp->rows + i;
    if(!lp->lower[varnr]) {
      theta = lp->upbo[varnr];
      for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++)
        lp->rhs[lp->mat[j].row_nr] -= theta * lp->mat[j].value;
    }
  }

  for(i = 1; i <= lp->rows; i++)
    if(!lp->lower[i])
      lp->rhs[i] -= lp->upbo[i];

  lp->eta_size = 0;
  v = 0;
  row_nr = 0;
  lp->num_inv = 0;
  numit = 0;

  while(v < lp->rows) {
    row_nr++;
    if(row_nr > lp->rows)
      row_nr = 1;

    v++;

    if(rownum[row_nr - 1] == 1)
      if(frow[row_nr]) {
        v = 0;
        j = lp->row_end[row_nr - 1] + 1;

        while(!(fcol[lp->col_no[j] - 1]))
          j++;

        colnr = lp->col_no[j];
        fcol[colnr - 1] = FALSE;
        colnum[colnr] = 0;

        for(j = lp->col_end[colnr - 1]; j < lp->col_end[colnr]; j++)
          if(frow[lp->mat[j].row_nr])
            rownum[lp->mat[j].row_nr - 1]--;

        frow[row_nr] = FALSE;
        minoriteration(lp, colnr, row_nr);
      }
  }
  v = 0;
  colnr = 0;
  while(v < lp->columns) {
    colnr++;
    if(colnr > lp->columns)
      colnr = 1;

    v++;

    if(colnum[colnr] == 1)
      if(fcol[colnr - 1]) {
        v = 0;
        j = lp->col_end[colnr - 1] + 1;

        while(!(frow[lp->mat[j - 1].row_nr]))
          j++;

        row_nr = lp->mat[j - 1].row_nr;
        frow[row_nr] = FALSE;
        rownum[row_nr - 1] = 0;

        for(j = lp->row_end[row_nr - 1] + 1; j <= lp->row_end[row_nr]; j++)
          if(fcol[lp->col_no[j] - 1])
            colnum[lp->col_no[j]]--;

        fcol[colnr - 1] = FALSE;
        numit++;
        col[numit - 1] = colnr;
        row[numit - 1] = row_nr;
      }
  }
  for(j = 1; j <= lp->columns; j++)
    if(fcol[j - 1]) {
      fcol[j - 1] = FALSE;
      setpivcol(lp, lp->lower[lp->rows + j], j + lp->rows, pcol);
      row_nr = 1;

      while((row_nr <= lp->rows) && (!(frow[row_nr] && pcol[row_nr])))
        row_nr++;

      /* if(row_nr == lp->rows + 1) */
      if(row_nr > lp->rows) /* problems! */
        error("Inverting failed");

      frow[row_nr] = FALSE;
      condensecol(lp, row_nr, pcol);
      theta = lp->rhs[row_nr] / (REAL) pcol[row_nr];
      rhsmincol(lp, theta, row_nr, lp->rows + j);
      addetacol(lp);
    }

  for(i = numit - 1; i >= 0; i--) {
    colnr = col[i];
    row_nr = row[i];
    varin = colnr + lp->rows;

    for(j = 0; j <= lp->rows; j++)
      pcol[j] = 0;

    for(j = lp->col_end[colnr - 1]; j < lp->col_end[colnr]; j++)
      pcol[lp->mat[j].row_nr] = lp->mat[j].value;

    pcol[0] -= Extrad;
    condensecol(lp, row_nr, pcol);
    theta = lp->rhs[row_nr] / (REAL) pcol[row_nr];
    rhsmincol(lp, theta, row_nr, varin);
    addetacol(lp);
  }

  for(i = 1; i <= lp->rows; i++)
    my_round(lp->rhs[i], lp->epsb);

  if(lp->print_at_invert)
    fprintf(stderr,
            "End Invert                eta_size %d rhs[0] %g\n",
            lp->eta_size, (double) - lp->rhs[0]);

  JustInverted = TRUE;
  DoInvert = FALSE;
  free(rownum);
  free(col);
  free(row);
  free(pcol);
  free(frow);
  free(fcol);
  free(colnum);
} /* invert */

static short colprim(lprec *lp,
                     int *colnr,
                     short minit,
                     REAL   *drow)
{
  int  varnr, i, j;
  REAL f, dpiv;

  dpiv = -lp->epsd;
  (*colnr) = 0;
  if(!minit) {
    for(i = 1; i <= lp->sum; i++)
      drow[i] = 0;
    drow[0] = 1;
    btran(lp, drow);
    for(i = 1; i <= lp->columns; i++) {
      varnr = lp->rows + i;
      if(!lp->basis[varnr])
        if(lp->upbo[varnr] > 0) {
          f = 0;
          for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++)
            f += drow[lp->mat[j].row_nr] * lp->mat[j].value;
          drow[varnr] = f;
        }
    }
    for(i = 1; i <= lp->sum; i++)
      my_round(drow[i], lp->epsd);
  }
  for(i = 1; i <= lp->sum; i++)
    if(!lp->basis[i])
      if(lp->upbo[i] > 0) {
        if(lp->lower[i])
          f = drow[i];
        else
          f = -drow[i];
        if(f < dpiv) {
          dpiv = f;
          (*colnr) = i;
        }
      }
  if(lp->trace) {
    if((*colnr)>0)
      fprintf(stderr, "col_prim:%d, reduced cost: %g\n",
              (*colnr), (double)dpiv);
    else
      fprintf(stderr,
              "col_prim: no negative reduced costs found, optimality!\n");
  }
  if(*colnr == 0) {
    Doiter   = FALSE;
    DoInvert = FALSE;
    Status   = OPTIMAL;
  }
  return((*colnr) > 0);
} /* colprim */

static short rowprim(lprec *lp,
                     int colnr,
                     int *row_nr,
                     REAL *theta,
                     REAL *pcol)
{
  int  i;
  REAL f, quot;

  (*row_nr) = 0;
  (*theta) = lp->infinite;
  for(i = 1; i <= lp->rows; i++) {
    f = pcol[i];
    if(f != 0) {
      if(my_abs(f) < Trej) {
        debug_print(lp, "pivot %g rejected, too small (limit %g)\n",
                    (double)f, (double)Trej);
      }
      else { /* pivot alright */
        quot = 2 * lp->infinite;
        if(f > 0)
          quot = lp->rhs[i] / (REAL) f;
        else if(lp->upbo[lp->bas[i]] < lp->infinite)
          quot = (lp->rhs[i] - lp->upbo[lp->bas[i]]) / (REAL) f;
        my_round(quot, lp->epsel);
        if(quot < (*theta)) {
          (*theta) = quot;
          (*row_nr) = i;
        }
      }
    }
  }
  if((*row_nr) == 0)
    for(i = 1; i <= lp->rows; i++) {
      f = pcol[i];
      if(f != 0) {
        quot = 2 * lp->infinite;
        if(f > 0)
          quot = lp->rhs[i] / (REAL) f;
        else
          if(lp->upbo[lp->bas[i]] < lp->infinite)
            quot = (lp->rhs[i] - lp->upbo[lp->bas[i]]) / (REAL) f;
        my_round(quot, lp->epsel);
        if(quot < (*theta)) {
          (*theta) = quot;
          (*row_nr) = i;
        }
      }
    }

  if((*theta) < 0) {
    fprintf(stderr, "Warning: Numerical instability, qout = %g\n",
            (double)(*theta));
    fprintf(stderr, "pcol[%d] = %18g, rhs[%d] = %18g , upbo = %g\n",
            (*row_nr), (double)f, (*row_nr), (double)lp->rhs[(*row_nr)],
            (double)lp->upbo[lp->bas[(*row_nr)]]);
  }
  if((*row_nr) == 0) {
    if(lp->upbo[colnr] == lp->infinite) {
      Doiter   = FALSE;
      DoInvert = FALSE;
      Status   = UNBOUNDED;
    }
    else {
      i = 1;
      while(pcol[i] >= 0 && i <= lp->rows)
        i++;
      if(i > lp->rows) { /* empty column with upperbound! */
        lp->lower[colnr] = FALSE;
        lp->rhs[0] += lp->upbo[colnr]*pcol[0];
        Doiter = FALSE;
        DoInvert = FALSE;
      }
      else if(pcol[i]<0) {
        (*row_nr) = i;
      }
    }
  }
  if((*row_nr) > 0)
    Doiter = TRUE;
  if(lp->trace)
    fprintf(stderr, "row_prim:%d, pivot element:%18g\n", (*row_nr),
            (double)pcol[(*row_nr)]);

  return((*row_nr) > 0);
} /* rowprim */

static short rowdual(lprec *lp, int *row_nr)
{
  int   i;
  REAL  f, g, minrhs;
  short artifs;

  (*row_nr) = 0;
  minrhs = -lp->epsb;
  i = 0;
  artifs = FALSE;
  while(i < lp->rows && !artifs) {
    i++;
    f = lp->upbo[lp->bas[i]];
    if(f == 0 && (lp->rhs[i] != 0)) {
      artifs = TRUE;
      (*row_nr) = i;
    }
    else {
      if(lp->rhs[i] < f - lp->rhs[i])
        g = lp->rhs[i];
      else
        g = f - lp->rhs[i];
      if(g < minrhs) {
        minrhs = g;
        (*row_nr) = i;
      }
    }
  }

  if(lp->trace) {
    if((*row_nr) > 0) {
      fprintf(stderr,
              "row_dual:%d, rhs of selected row:           %18g\n",
              (*row_nr), (double)lp->rhs[(*row_nr)]);
      if(lp->upbo[lp->bas[(*row_nr)]] < lp->infinite)
        fprintf(stderr,
                "\t\tupper bound of basis variable:    %18g\n",
                (double)lp->upbo[lp->bas[(*row_nr)]]);
    }
    else
      fprintf(stderr, "row_dual: no infeasibilities found\n");
  }

  return((*row_nr) > 0);
} /* rowdual */

static short coldual(lprec *lp,
                     int row_nr,
                     int *colnr,
                     short minit,
                     REAL *prow,
                     REAL *drow)
{
  int  i, j, k, r, varnr, *rowp, row;
  REAL theta, quot, pivot, d, f, g, *valuep, value;

  Doiter = FALSE;
  if(!minit) {
    for(i = 0; i <= lp->rows; i++) {
      prow[i] = 0;
      drow[i] = 0;
    }

    drow[0] = 1;
    prow[row_nr] = 1;

    for(i = lp->eta_size; i >= 1; i--) {
      d = 0;
      f = 0;
      k = lp->eta_col_end[i] - 1;
      r = lp->eta_row_nr[k];
      j = lp->eta_col_end[i - 1];

      /* this is one of the loops where the program consumes a lot of CPU
         time */
      /* let's help the compiler by doing some pointer arithmetic instead
         of array indexing */
      for(rowp = lp->eta_row_nr + j, valuep = lp->eta_value + j;
          j <= k;
          j++, rowp++, valuep++) {
        f += prow[*rowp] * *valuep;
        d += drow[*rowp] * *valuep;
      }

      my_round(f, lp->epsel);
      prow[r] = f;
      my_round(d, lp->epsd);
      drow[r] = d;
    }

    for(i = 1; i <= lp->columns; i++) {
      varnr = lp->rows + i;
      if(!lp->basis[varnr]) {
        matrec *matentry;

        d = - Extrad * drow[0];
        f = 0;
        k = lp->col_end[i];
        j = lp->col_end[i - 1];

        /* this is one of the loops where the program consumes a lot
           of cpu time */
        /* let's help the compiler with pointer arithmetic instead
           of array indexing */
        for(matentry = lp->mat + j;
            j < k;
            j++, matentry++) {
          row = (*matentry).row_nr;
          value = (*matentry).value;
          d += drow[row] * value;
          f += prow[row] * value;
        }

        my_round(f, lp->epsel);
        prow[varnr] = f;
        my_round(d, lp->epsd);
        drow[varnr] = d;
      }
    }
  }

  if(lp->rhs[row_nr] > lp->upbo[lp->bas[row_nr]])
    g = -1;
  else
    g = 1;

  pivot = 0;
  (*colnr) = 0;
  theta = lp->infinite;

  for(i = 1; i <= lp->sum; i++) {
    if(lp->lower[i])
      d = prow[i] * g;
    else
      d = -prow[i] * g;

    if((d < 0) && (!lp->basis[i]) && (lp->upbo[i] > 0)) {
      if(lp->lower[i])
        quot = -drow[i] / (REAL) d;
      else
        quot = drow[i] / (REAL) d;
      if(quot < theta) {
        theta = quot;
        pivot = d;
        (*colnr) = i;
      }
      else if((quot == theta) && (my_abs(d) > my_abs(pivot))) {
        pivot = d;
        (*colnr) = i;
      }
    }
  }

  if(lp->trace)
    fprintf(stderr, "col_dual:%d, pivot element:  %18g\n", (*colnr),
            (double)prow[(*colnr)]);

  if((*colnr) > 0)
    Doiter = TRUE;

  return((*colnr) > 0);
} /* coldual */

static void iteration(lprec *lp,
                      int row_nr,
                      int varin,
                      REAL *theta,
                      REAL up,
                      short *minit,
                      short *low,
                      short primal)
{
  int  i, k, varout;
  REAL f;
  REAL pivot;

  lp->iter++;

  if(((*minit) = (*theta) > (up + lp->epsb))) {
    (*theta) = up;
    (*low) = !(*low);
  }

  k = lp->eta_col_end[lp->eta_size + 1];
  pivot = lp->eta_value[k - 1];

  for(i = lp->eta_col_end[lp->eta_size]; i < k; i++) {
    f = lp->rhs[lp->eta_row_nr[i]] - (*theta) * lp->eta_value[i];
    my_round(f, lp->epsb);
    lp->rhs[lp->eta_row_nr[i]] = f;
  }

  if(!(*minit)) {
    lp->rhs[row_nr] = (*theta);
    varout = lp->bas[row_nr];
    lp->bas[row_nr] = varin;
    lp->basis[varout] = FALSE;
    lp->basis[varin] = TRUE;

    if(primal && pivot < 0)
      lp->lower[varout] = FALSE;

    if(!(*low) && up < lp->infinite) {
      (*low) = TRUE;
      lp->rhs[row_nr] = up - lp->rhs[row_nr];
      for(i = lp->eta_col_end[lp->eta_size]; i < k; i++)
        lp->eta_value[i] = -lp->eta_value[i];
    }

    addetacol(lp);
    lp->num_inv++;
  }

  if(lp->trace) {
    fprintf(stderr, "Theta = %g ", (double)(*theta));
    if((*minit)) {
      if(!lp->lower[varin])
        fprintf(stderr,
                "Iteration: %d, variable %d changed from 0 to its upper bound of %g\n",
                lp->iter, varin, (double)lp->upbo[varin]);
      else
        fprintf(stderr,
                "Iteration: %d, variable %d changed its upper bound of %g to 0\n",
                lp->iter, varin, (double)lp->upbo[varin]);
    }
    else
      fprintf(stderr,
              "Iteration: %d, variable %d entered basis at: %g\n",
              lp->iter, varin, (double)lp->rhs[row_nr]);
    if(!primal) {
      f = 0;
      for(i = 1; i <= lp->rows; i++)
        if(lp->rhs[i] < 0)
          f -= lp->rhs[i];
        else
          if(lp->rhs[i] > lp->upbo[lp->bas[i]])
            f += lp->rhs[i] - lp->upbo[lp->bas[i]];
      fprintf(stderr, "feasibility gap of this basis: %g\n",
              (double)f);
    }
    else
      fprintf(stderr,
              "objective function value of this feasible basis: %g\n",
              (double)lp->rhs[0]);
  }
} /* iteration */


static int solvelp(lprec *lp)
{
  int    i, j, varnr;
  REAL   f, theta;
  short  primal;
  REAL   *drow, *prow, *Pcol;
  short  minit;
  int    colnr, row_nr;
  short  *test;

  if(lp->do_presolve)
    presolve(lp);

  CALLOC(drow, lp->sum + 1);
  CALLOC(prow, lp->sum + 1);
  CALLOC(Pcol, lp->rows + 1);
  CALLOC(test, lp->sum +1);

  lp->iter = 0;
  minit = FALSE;
  Status = RUNNING;
  DoInvert = FALSE;
  Doiter = FALSE;

  for(i = 1, primal = TRUE; (i <= lp->rows) && primal; i++)
    primal = (lp->rhs[i] >= 0) && (lp->rhs[i] <= lp->upbo[lp->bas[i]]);

  if(lp->trace) {
    if(primal)
      fprintf(stderr, "Start at feasible basis\n");
    else
      fprintf(stderr, "Start at infeasible basis\n");
  }

  if(!primal) {
    drow[0] = 1;

    for(i = 1; i <= lp->rows; i++)
      drow[i] = 0;

    Extrad = 0;

    for(i = 1; i <= lp->columns; i++) {
      varnr = lp->rows + i;
      drow[varnr] = 0;

      for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++)
        if(drow[lp->mat[j].row_nr] != 0)
          drow[varnr] += drow[lp->mat[j].row_nr] * lp->mat[j].value;

      if(drow[varnr] < Extrad)
        Extrad = drow[varnr];
    }
  }
  else
    Extrad = 0;

  if(lp->trace)
    fprintf(stderr, "Extrad = %g\n", (double)Extrad);

  minit = FALSE;

  while(Status == RUNNING) {
    Doiter = FALSE;
    DoInvert = FALSE;

    if(primal) {
      if(colprim(lp, &colnr, minit, drow)) {
        setpivcol(lp, lp->lower[colnr], colnr, Pcol);

        if(rowprim(lp, colnr, &row_nr, &theta, Pcol))
          condensecol(lp, row_nr, Pcol);
      }
    }
    else /* not primal */ {
      if(!minit)
        rowdual(lp, &row_nr);

      if(row_nr > 0 ) {
        if(coldual(lp, row_nr, &colnr, minit, prow, drow)) {
          setpivcol(lp, lp->lower[colnr], colnr, Pcol);

          /* getting div by zero here. Catch it and try to recover */
          if(Pcol[row_nr] == 0) {
            fprintf(stderr,
                    "An attempt was made to divide by zero (Pcol[%d])\n",
                    row_nr);
            fprintf(stderr,
                    "This indicates numerical instability\n");
            Doiter = FALSE;
            if(!JustInverted) {
              fprintf(stderr,
                      "Trying to recover. Reinverting Eta\n");
              DoInvert = TRUE;
            }
            else {
              fprintf(stderr, "Can't reinvert, failure\n");
              Status = FAILURE;
            }
          }
          else {
            condensecol(lp, row_nr, Pcol);
            f = lp->rhs[row_nr] - lp->upbo[lp->bas[row_nr]];

            if(f > 0) {
              theta = f / (REAL) Pcol[row_nr];
              if(theta <= lp->upbo[colnr])
                lp->lower[lp->bas[row_nr]] = !lp->lower[lp->bas[row_nr]];
            }
            else /* f <= 0 */
              theta = lp->rhs[row_nr] / (REAL) Pcol[row_nr];
          }
        }
        else
          Status = INFEASIBLE;
      }
      else {
        primal   = TRUE;
        Doiter   = FALSE;
        Extrad   = 0;
        DoInvert = TRUE;
      }
    }

    if(Doiter)
      iteration(lp, row_nr, colnr, &theta, lp->upbo[colnr], &minit,
                &lp->lower[colnr], primal);

    if(lp->num_inv >= lp->max_num_inv)
      DoInvert = TRUE;

    if(DoInvert) {
      if(lp->print_at_invert)
        fprintf(stderr, "Inverting: Primal = %d\n", primal);
      invert(lp);
    }
  }

  lp->total_iter += lp->iter;

  free(drow);
  free(prow);
  free(Pcol);
  free(test);

  return(Status);
} /* solvelp */


static short is_int(lprec *lp, int i)
{
  REAL   value, error;

  value = lp->solution[i];
  error = value - (REAL)floor((double)value);

  if(error < lp->epsilon)
    return(TRUE);

  if(error > (1 - lp->epsilon))
    return(TRUE);

  return(FALSE);
} /* is_int */


static void construct_solution(lprec *lp)
{
  int    i, j, basi;
  REAL   f;

  /* zero all results of rows */
  memset(lp->solution, '\0', (lp->rows + 1) * sizeof(REAL));

  lp->solution[0] = -lp->orig_rh[0];

  if(lp->scaling_used) {
    lp->solution[0] /= lp->scale[0];

    for(i = lp->rows + 1; i <= lp->sum; i++)
      lp->solution[i] = lp->lowbo[i] * lp->scale[i];

    for(i = 1; i <= lp->rows; i++) {
      basi = lp->bas[i];
      if(basi > lp->rows)
        lp->solution[basi] += lp->rhs[i] * lp->scale[basi];
    }
    for(i = lp->rows + 1; i <= lp->sum; i++)
      if(!lp->basis[i] && !lp->lower[i])
        lp->solution[i] += lp->upbo[i] * lp->scale[i];

    for(j = 1; j <= lp->columns; j++) {
      f = lp->solution[lp->rows + j];
      if(f != 0)
        for(i = lp->col_end[j - 1]; i < lp->col_end[j]; i++)
          lp->solution[lp->mat[i].row_nr] += (f / lp->scale[lp->rows+j])
            * (lp->mat[i].value / lp->scale[lp->mat[i].row_nr]);
    }

    for(i = 0; i <= lp->rows; i++) {
      if(my_abs(lp->solution[i]) < lp->epsb)
        lp->solution[i] = 0;
      else if(lp->ch_sign[i])
        lp->solution[i] = -lp->solution[i];
    }
  }
  else { /* no scaling */
    for(i = lp->rows + 1; i <= lp->sum; i++)
      lp->solution[i] = lp->lowbo[i];

    for(i = 1; i <= lp->rows; i++) {
      basi = lp->bas[i];
      if(basi > lp->rows)
        lp->solution[basi] += lp->rhs[i];
    }

    for(i = lp->rows + 1; i <= lp->sum; i++)
      if(!lp->basis[i] && !lp->lower[i])
        lp->solution[i] += lp->upbo[i];

    for(j = 1; j <= lp->columns; j++) {
      f = lp->solution[lp->rows + j];
      if(f != 0)
        for(i = lp->col_end[j - 1]; i < lp->col_end[j]; i++)
          lp->solution[lp->mat[i].row_nr] += f * lp->mat[i].value;
    }

    for(i = 0; i <= lp->rows; i++) {
      if(my_abs(lp->solution[i]) < lp->epsb)
        lp->solution[i] = 0;
      else if(lp->ch_sign[i])
        lp->solution[i] = -lp->solution[i];
    }
  }
} /* construct_solution */

static void calculate_duals(lprec *lp)
{
  int i;

  /* initialize */
  lp->duals[0] = 1;
  for(i = 1; i <= lp->rows; i++)
    lp->duals[i] = 0;

  btran(lp, lp->duals);

  if(lp->scaling_used)
    for(i = 1; i <= lp->rows; i++)
      lp->duals[i] *= lp->scale[i] / lp->scale[0];

  /* the dual values are the reduced costs of the slacks */
  /* When the slack is at its upper bound, change the sign. */
  for(i = 1; i <= lp->rows; i++) {
    if(lp->basis[i])
      lp->duals[i] = 0;
    /* added a test if variable is different from 0 because sometime you get
       -0 and this is different from 0 on for example INTEL processors (ie 0
       != -0 on INTEL !) PN */
    else if((lp->ch_sign[0] == lp->ch_sign[i]) && lp->duals[i])
      lp->duals[i] = - lp->duals[i];
  }
} /* calculate_duals */

static void check_if_less(REAL x,
                          REAL y,
                          REAL value)
{
  if(x >= y) {
    fprintf(stderr,
            "Error: new upper or lower bound is not more restrictive\n");
    fprintf(stderr, "bound 1: %g, bound 2: %g, value: %g\n",
            (double)x, (double)y, (double)value);
    /* exit(EXIT_FAILURE); */
  }
}

static void check_solution(lprec *lp,
                           REAL *upbo,
                           REAL *lowbo)
{
  int i;

  /* check if all solution values are within the bounds, but allow some margin
     for numerical errors */

#define CHECK_EPS 1e-2

  if(lp->columns_scaled)
    for(i = lp->rows + 1; i <= lp->sum; i++) {
      if(lp->solution[i] < lowbo[i] * lp->scale[i] - CHECK_EPS) {
        fprintf(stderr,
                "Error: variable %d (%s) has a solution (%g) smaller than its lower bound (%g)\n",
                i - lp->rows, lp->col_name[i - lp->rows],
                (double)lp->solution[i], (double)lowbo[i] * lp->scale[i]);
        /* abort(); */
      }

      if(lp->solution[i] > upbo[i] * lp->scale[i] + CHECK_EPS) {
        fprintf(stderr,
                "Error: variable %d (%s) has a solution (%g) larger than its upper bound (%g)\n",
                i - lp->rows, lp->col_name[i - lp->rows],
                (double)lp->solution[i], (double)upbo[i] * lp->scale[i]);
        /* abort(); */
      }
    }
  else /* columns not scaled */
    for(i = lp->rows + 1; i <= lp->sum; i++) {
      if(lp->solution[i] < lowbo[i] - CHECK_EPS) {
        fprintf(stderr,
                "Error: variable %d (%s) has a solution (%g) smaller than its lower bound (%g)\n",
                i - lp->rows, lp->col_name[i - lp->rows],
                (double)lp->solution[i], (double)lowbo[i]);
        /* abort(); */
      }

      if(lp->solution[i] > upbo[i] + CHECK_EPS) {
        fprintf(stderr,
                "Error: variable %d (%s) has a solution (%g) larger than its upper bound (%g)\n",
                i - lp->rows, lp->col_name[i - lp->rows],
                (double)lp->solution[i], (double)upbo[i]);
        /* abort(); */
      }
    }
} /* check_solution */


static int milpsolve(lprec *lp,
                     REAL   *upbo,
                     REAL   *lowbo,
                     short  *sbasis,
                     short  *slower,
                     int    *sbas,
                     int     recursive)
{
  int i, j, failure, notint, is_worse;
  REAL theta, tmpreal;

  if(Break_bb)
    return(BREAK_BB);

  Level++;
  lp->total_nodes++;

  if(Level > lp->max_level)
    lp->max_level = Level;

  debug_print(lp, "starting solve");

  /* make fresh copies of upbo, lowbo, rh as solving changes them */
  memcpy(lp->upbo,  upbo,    (lp->sum + 1)  * sizeof(REAL));
  memcpy(lp->lowbo, lowbo,   (lp->sum + 1)  * sizeof(REAL));
  memcpy(lp->rh,    lp->orig_rh, (lp->rows + 1) * sizeof(REAL));

  /* make shure we do not do memcpy(lp->basis, lp->basis ...) ! */
  if(recursive) {
    memcpy(lp->basis, sbasis,  (lp->sum + 1)  * sizeof(short));
    memcpy(lp->lower, slower,  (lp->sum + 1)  * sizeof(short));
    memcpy(lp->bas,   sbas,    (lp->rows + 1) * sizeof(int));
  }

  if(lp->anti_degen) { /* randomly disturb bounds */
    for(i = 1; i <= lp->columns; i++) {
      tmpreal = (REAL) (rand() % 100 * 0.00001);
      if(tmpreal > lp->epsb)
        lp->lowbo[i + lp->rows] -= tmpreal;
      tmpreal = (REAL) (rand() % 100 * 0.00001);
      if(tmpreal > lp->epsb)
        lp->upbo[i + lp->rows] += tmpreal;
    }
    lp->eta_valid = FALSE;
  }

  if(!lp->eta_valid) {
    /* transform to all lower bounds to zero */
    for(i = 1; i <= lp->columns; i++)
      if((theta = lp->lowbo[lp->rows + i]) != 0) {
        if(lp->upbo[lp->rows + i] < lp->infinite)
          lp->upbo[lp->rows + i] -= theta;
        for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++)
          lp->rh[lp->mat[j].row_nr] -= theta * lp->mat[j].value;
      }
    invert(lp);
    lp->eta_valid = TRUE;
  }

  failure = solvelp(lp);

  if(lp->anti_degen && (failure == OPTIMAL)) {
    /* restore to original problem, solve again starting from the basis found
       for the disturbed problem */

    /* restore original problem */
    memcpy(lp->upbo,  upbo,        (lp->sum + 1)  * sizeof(REAL));
    memcpy(lp->lowbo, lowbo,       (lp->sum + 1)  * sizeof(REAL));
    memcpy(lp->rh,    lp->orig_rh, (lp->rows + 1) * sizeof(REAL));

    /* transform to all lower bounds zero */
    for(i = 1; i <= lp->columns; i++)
      if((theta = lp->lowbo[lp->rows + i] != 0)) {
        if(lp->upbo[lp->rows + i] < lp->infinite)
          lp->upbo[lp->rows + i] -= theta;
        for(j = lp->col_end[i - 1]; j < lp->col_end[i]; j++)
          lp->rh[lp->mat[j].row_nr] -= theta * lp->mat[j].value;
      }
    invert(lp);
    lp->eta_valid = TRUE;
    failure = solvelp(lp); /* and solve again */
  }

  if(failure != OPTIMAL)
    debug_print(lp, "this problem has no solution, it is %s",
                (failure == UNBOUNDED) ? "unbounded" : "infeasible");

  if(failure == INFEASIBLE && lp->verbose)
    fprintf(stderr, "level %d INF\n", Level);

  if(failure == OPTIMAL) { /* there is a good solution */
    construct_solution(lp);

    /* because of reports of solution > upbo */
    /* check_solution(lp, upbo, lowbo); get too many hits ?? */

    debug_print(lp, "a solution was found");
    debug_print_solution(lp);

    /* if this solution is worse than the best sofar, this branch must die */

    /* if we can only have integer OF values, we might consider requiring to
       be at least 1 better than the best sofar, MB */

    if(lp->maximise)
      is_worse = lp->solution[0] <= lp->best_solution[0];
    else /* minimising! */
      is_worse = lp->solution[0] >= lp->best_solution[0];

    if(is_worse) {
      if(lp->verbose)
        fprintf(stderr, "level %d OPT NOB value %g bound %g\n",
                Level, (double)lp->solution[0],
                (double)lp->best_solution[0]);
      debug_print(lp, "but it was worse than the best sofar, discarded");
      Level--;
      return(MILP_FAIL);
    }

    /* check if solution contains enough ints */
    if(lp->bb_rule == FIRST_NI) {
      for(notint = 0, i = lp->rows + 1;
          i <= lp->sum && notint == 0;
          i++) {
        if(lp->must_be_int[i] && !is_int(lp, i)) {
          if(lowbo[i] == upbo[i]) { /* this var is already fixed */
            fprintf(stderr,
                    "Warning: integer var %d is already fixed at %d, but has non-integer value %g\n",
                    i - lp->rows, (int)lowbo[i],
                    (double)lp->solution[i]);
            fprintf(stderr, "Perhaps the -e option should be used\n");
          }
          else
            notint = i;
        }
      }
    }
    if(lp->bb_rule == RAND_NI) {
      int nr_not_int, select_not_int;
      nr_not_int = 0;

      for(i = lp->rows + 1; i <= lp->sum; i++)
        if(lp->must_be_int[i] && !is_int(lp, i))
          nr_not_int++;

      if(nr_not_int == 0)
        notint = 0;
      else {
        select_not_int = (rand() % nr_not_int) + 1;
        i = lp->rows + 1;
        while(select_not_int > 0) {
          if(lp->must_be_int[i] && !is_int(lp, i))
            select_not_int--;
          i++;
        }
        notint = i - 1;
      }
    }

    if(lp->verbose) {
      if(notint)
        fprintf(stderr, "level %d OPT     value %g\n", Level,
                (double)lp->solution[0]);
      else
        fprintf(stderr, "level %d OPT INT value %g\n", Level,
                (double)lp->solution[0]);
    }

    if(notint) { /* there is at least one value not yet int */
      /* set up two new problems */
      REAL   *new_upbo, *new_lowbo;
      REAL   new_bound;
      short  *new_lower,*new_basis;
      int    *new_bas;
      int     resone, restwo;

      /* allocate room for them */
      MALLOC(new_upbo,  lp->sum + 1);
      MALLOC(new_lowbo, lp->sum + 1);
      MALLOC(new_lower, lp->sum + 1);
      MALLOC(new_basis, lp->sum + 1);
      MALLOC(new_bas,   lp->rows + 1);
      memcpy(new_upbo,  upbo,      (lp->sum + 1)  * sizeof(REAL));
      memcpy(new_lowbo, lowbo,     (lp->sum + 1)  * sizeof(REAL));
      memcpy(new_lower, lp->lower, (lp->sum + 1)  * sizeof(short));
      memcpy(new_basis, lp->basis, (lp->sum + 1)  * sizeof(short));
      memcpy(new_bas,   lp->bas,   (lp->rows + 1) * sizeof(int));

      if(lp->names_used)
        debug_print(lp, "not enough ints. Selecting var %s, val: %g",
                    lp->col_name[notint - lp->rows],
                    (double)lp->solution[notint]);
      else
        debug_print(lp,
                    "not enough ints. Selecting Var [%d], val: %g",
                    notint, (double)lp->solution[notint]);
      debug_print(lp, "current bounds:\n");
      debug_print_bounds(lp, upbo, lowbo);

      if(lp->floor_first) {
        new_bound = ceil(lp->solution[notint]) - 1;

        /* this bound might conflict */
        if(new_bound < lowbo[notint]) {
          debug_print(lp,
                      "New upper bound value %g conflicts with old lower bound %g\n",
                      (double)new_bound, (double)lowbo[notint]);
          resone = MILP_FAIL;
        }
        else { /* bound feasible */
          check_if_less(new_bound, upbo[notint], lp->solution[notint]);
          new_upbo[notint] = new_bound;
          debug_print(lp, "starting first subproblem with bounds:");
          debug_print_bounds(lp, new_upbo, lowbo);
          lp->eta_valid = FALSE;
          resone = milpsolve(lp, new_upbo, lowbo, new_basis, new_lower,
                             new_bas, TRUE);
          lp->eta_valid = FALSE;
        }
        new_bound += 1;
        if(new_bound > upbo[notint]) {
          debug_print(lp,
                      "New lower bound value %g conflicts with old upper bound %g\n",
                      (double)new_bound, (double)upbo[notint]);
          restwo = MILP_FAIL;
        }
        else { /* bound feasible */
          check_if_less(lowbo[notint], new_bound, lp->solution[notint]);
          new_lowbo[notint] = new_bound;
          debug_print(lp, "starting second subproblem with bounds:");
          debug_print_bounds(lp, upbo, new_lowbo);
          lp->eta_valid = FALSE;
          restwo = milpsolve(lp, upbo, new_lowbo, new_basis, new_lower,
                             new_bas, TRUE);
          lp->eta_valid = FALSE;
        }
      }
      else { /* take ceiling first */
        new_bound = ceil(lp->solution[notint]);
        /* this bound might conflict */
        if(new_bound > upbo[notint]) {
          debug_print(lp,
                      "New lower bound value %g conflicts with old upper bound %g\n",
                      (double)new_bound, (double)upbo[notint]);
          resone = MILP_FAIL;
        }
        else { /* bound feasible */
          check_if_less(lowbo[notint], new_bound, lp->solution[notint]);
          new_lowbo[notint] = new_bound;
          debug_print(lp, "starting first subproblem with bounds:");
          debug_print_bounds(lp, upbo, new_lowbo);
          lp->eta_valid = FALSE;
          resone = milpsolve(lp, upbo, new_lowbo, new_basis, new_lower,
                             new_bas, TRUE);
          lp->eta_valid = FALSE;
        }
        new_bound -= 1;
        if(new_bound < lowbo[notint]) {
          debug_print(lp,
                      "New upper bound value %g conflicts with old lower bound %g\n",
                      (double)new_bound, (double)lowbo[notint]);
          restwo = MILP_FAIL;
        }
        else { /* bound feasible */
          check_if_less(new_bound, upbo[notint], lp->solution[notint]);
          new_upbo[notint] = new_bound;
          debug_print(lp, "starting second subproblem with bounds:");
          debug_print_bounds(lp, new_upbo, lowbo);
          lp->eta_valid = FALSE;
          restwo = milpsolve(lp, new_upbo, lowbo, new_basis, new_lower,
                             new_bas, TRUE);
          lp->eta_valid = FALSE;
        }
      }
      if(resone && restwo) /* both failed and must have been infeasible */
        failure = INFEASIBLE;
      else
        failure = OPTIMAL;

      free(new_upbo);
      free(new_lowbo);
      free(new_basis);
      free(new_lower);
      free(new_bas);
    }
    else { /* all required values are int */
      debug_print(lp, "--> valid solution found");

      if(lp->maximise)
        is_worse = lp->solution[0] < lp->best_solution[0];
      else
        is_worse = lp->solution[0] > lp->best_solution[0];

      if(!is_worse) { /* Current solution better */
        if(lp->debug || (lp->verbose && !lp->print_sol))
          fprintf(stderr,
                  "*** new best solution: old: %g, new: %g ***\n",
                  (double)lp->best_solution[0], (double)lp->solution[0]);
        memcpy(lp->best_solution, lp->solution, (lp->sum + 1) * sizeof(REAL));
        calculate_duals(lp);

        if(lp->print_sol)
          print_solution(lp);

        if(lp->break_at_int) {
          if(lp->maximise && (lp->best_solution[0] > lp->break_value))
            Break_bb = TRUE;

          if(!lp->maximise && (lp->best_solution[0] < lp->break_value))
            Break_bb = TRUE;
        }
      }
    }
  }

  Level--;

  /* failure can have the values OPTIMAL, UNBOUNDED and INFEASIBLE. */
  return(failure);
} /* milpsolve */


int solve(lprec *lp)
{
  int result, i;

  lp->total_iter  = 0;
  lp->max_level   = 1;
  lp->total_nodes = 0;

  if(isvalid(lp)) {
    if(lp->maximise && lp->obj_bound == lp->infinite)
      lp->best_solution[0] = -lp->infinite;
    else if(!lp->maximise && lp->obj_bound == -lp->infinite)
      lp->best_solution[0] = lp->infinite;
    else
      lp->best_solution[0] = lp->obj_bound;

    Level = 0;

    if(!lp->basis_valid) {
      for(i = 0; i <= lp->rows; i++) {
        lp->basis[i] = TRUE;
        lp->bas[i]   = i;
      }

      for(i = lp->rows + 1; i <= lp->sum; i++)
        lp->basis[i] = FALSE;

      for(i = 0; i <= lp->sum; i++)
        lp->lower[i] = TRUE;

      lp->basis_valid = TRUE;
    }

    lp->eta_valid = FALSE;
    Break_bb      = FALSE;
    result        = milpsolve(lp, lp->orig_upbo, lp->orig_lowbo, lp->basis,
                              lp->lower, lp->bas, FALSE);
    return(result);
  }

  /* if we get here, isvalid(lp) failed. I suggest we return FAILURE */
  fprintf(stderr, "Error, the current LP seems to be invalid\n");
  return(FAILURE);
} /* solve */

int lag_solve(lprec *lp, REAL start_bound, int num_iter, short verbose)
{
  int i, j, result, citer;
  short status, OrigFeas, AnyFeas, same_basis;
  REAL *OrigObj, *ModObj, *SubGrad, *BestFeasSol;
  REAL Zub, Zlb, Ztmp, pie;
  REAL rhsmod, Step, SqrsumSubGrad;
  int   *old_bas;
  short *old_lower;

  /* allocate mem */
  MALLOC(OrigObj, lp->columns + 1);
  CALLOC(ModObj, lp->columns + 1);
  CALLOC(SubGrad, lp->nr_lagrange);
  CALLOC(BestFeasSol, lp->sum + 1);
  MALLOCCPY(old_bas, lp->bas, lp->rows + 1);
  MALLOCCPY(old_lower, lp->lower, lp->sum + 1);

  get_row(lp, 0, OrigObj);

  pie = 2;

  if(lp->maximise) {
    Zub = DEF_INFINITE;
    Zlb = start_bound;
  }
  else {
    Zlb = -DEF_INFINITE;
    Zub = start_bound;
  }
  status   = RUNNING;
  Step     = 1;
  OrigFeas = FALSE;
  AnyFeas  = FALSE;
  citer    = 0;

  for(i = 0 ; i < lp->nr_lagrange; i++)
    lp->lambda[i] = 0;

  while(status == RUNNING) {
    citer++;

    for(i = 1; i <= lp->columns; i++) {
      ModObj[i] = OrigObj[i];
      for(j = 0; j < lp->nr_lagrange; j++) {
        if(lp->maximise)
          ModObj[i] -= lp->lambda[j] * lp->lag_row[j][i];
        else
          ModObj[i] += lp->lambda[j] * lp->lag_row[j][i];
      }
    }
    for(i = 1; i <= lp->columns; i++) {
      set_mat(lp, 0, i, ModObj[i]);
    }
    rhsmod = 0;
    for(i = 0; i < lp->nr_lagrange; i++)
      if(lp->maximise)
        rhsmod += lp->lambda[i] * lp->lag_rhs[i];
      else
        rhsmod -= lp->lambda[i] * lp->lag_rhs[i];

    if(verbose) {
      fprintf(stderr, "Zub: %10g Zlb: %10g Step: %10g pie: %10g Feas %d\n",
              (double)Zub, (double)Zlb, (double)Step, (double)pie, OrigFeas);
      for(i = 0; i < lp->nr_lagrange; i++)
        fprintf(stderr, "%3d SubGrad %10g lambda %10g\n", i,
                (double)SubGrad[i], (double)lp->lambda[i]);
    }

    if(verbose && lp->sum < 20)
      print_lp(lp);

    result = solve(lp);

    if(verbose && lp->sum < 20) {
      print_solution(lp);
    }

    same_basis = TRUE;
    i = 1;
    while(same_basis && i < lp->rows) {
      same_basis = (old_bas[i] == lp->bas[i]);
      i++;
    }
    i = 1;
    while(same_basis && i < lp->sum) {
      same_basis=(old_lower[i] == lp->lower[i]);
      i++;
    }
    if(!same_basis) {
      memcpy(old_lower, lp->lower, (lp->sum+1) * sizeof(short));
      memcpy(old_bas, lp->bas, (lp->rows+1) * sizeof(int));
      pie *= 0.95;
    }

    if(verbose)
      fprintf(stderr, "result: %d  same basis: %d\n", result, same_basis);

    if(result == UNBOUNDED) {
      for(i = 1; i <= lp->columns; i++)
        fprintf(stderr, "%g ", (double)ModObj[i]);
      exit(EXIT_FAILURE);
    }

    if(result == FAILURE)
      status = FAILURE;

    if(result == INFEASIBLE)
      status = INFEASIBLE;

    SqrsumSubGrad = 0;
    for(i = 0; i < lp->nr_lagrange; i++) {
      SubGrad[i]= -lp->lag_rhs[i];
      for(j = 1; j <= lp->columns; j++)
        SubGrad[i] += lp->best_solution[lp->rows + j] * lp->lag_row[i][j];
      SqrsumSubGrad += SubGrad[i] * SubGrad[i];
    }

    OrigFeas = TRUE;
    for(i = 0; i < lp->nr_lagrange; i++)
      if(lp->lag_con_type[i]) {
        if(my_abs(SubGrad[i]) > lp->epsb)
          OrigFeas = FALSE;
      }
      else if(SubGrad[i] > lp->epsb)
        OrigFeas = FALSE;

    if(OrigFeas) {
      AnyFeas = TRUE;
      Ztmp = 0;
      for(i = 1; i <= lp->columns; i++)
        Ztmp += lp->best_solution[lp->rows + i] * OrigObj[i];
      if((lp->maximise) && (Ztmp > Zlb)) {
        Zlb = Ztmp;
        for(i = 1; i <= lp->sum; i++)
          BestFeasSol[i] = lp->best_solution[i];
        BestFeasSol[0] = Zlb;
        if(verbose)
          fprintf(stderr, "Best feasible solution: %g\n", (double)Zlb);
      }
      else if(Ztmp < Zub) {
        Zub = Ztmp;
        for(i = 1; i <= lp->sum; i++)
          BestFeasSol[i] = lp->best_solution[i];
        BestFeasSol[0] = Zub;
        if(verbose)
          fprintf(stderr, "Best feasible solution: %g\n", (double)Zub);
      }
        }

    if(lp->maximise)
      Zub = my_min(Zub, rhsmod + lp->best_solution[0]);
    else
      Zlb = my_max(Zlb, rhsmod + lp->best_solution[0]);

    if(my_abs(Zub-Zlb)<0.001) {
      status = OPTIMAL;
    }
    Step = pie * ((1.05*Zub) - Zlb) / SqrsumSubGrad;

    for(i = 0; i < lp->nr_lagrange; i++) {
      lp->lambda[i] += Step * SubGrad[i];
      if(!lp->lag_con_type[i] && lp->lambda[i] < 0)
        lp->lambda[i] = 0;
    }

    if(citer == num_iter && status==RUNNING) {
      if(AnyFeas)
        status = FEAS_FOUND;
      else
        status = NO_FEAS_FOUND;
    }
  }

  for(i = 0; i <= lp->sum; i++)
    lp->best_solution[i] = BestFeasSol[i];

  for(i = 1; i <= lp->columns; i++)
    set_mat(lp, 0, i, OrigObj[i]);

  if(lp->maximise)
    lp->lag_bound = Zub;
  else
    lp->lag_bound = Zlb;
  free(BestFeasSol);
  free(SubGrad);
  free(OrigObj);
  free(ModObj);
  free(old_bas);
  free(old_lower);

  return(status);
}

