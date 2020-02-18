/*
     This astronomical program converts coordinates from sky to telescope
  and visa versa.  Before coordinate conversion can take place, the
  program needs to be initialized by pointing the telescope at two known
  positions in the sky.
     Horizon is inputed and displayed as increasing CW, but internally, the
  program treats the horizon as increasing CCW.
     The program runs in real time, continuously updating coordinates, if the
  system time and date are used.
     References: Sky and Telescope, February, 1989, pg. 194-196.
     Program by Mel Bartels, March 15, 1990.
*/

#include <stdio.h>
#include <conio.h>
#include <math.h>
#include <time.h>
#include <dos.h>
#include <string.h>

/* main screen window */

#define MAIN_WINDOW_LEFT    1
#define MAIN_WINDOW_RIGHT  80
#define MAIN_WINDOW_TOP     1
#define MAIN_WINDOW_BOTTOM 24

/* bright stars window */

#define STARS_LEFT        46
#define STARS_RIGHT       STARS_LEFT+27
#define STARS_TOP         11
#define STARS_BOTTOM      MAIN_WINDOW_BOTTOM

/* character codes for window border */

#define HORIZ_BORDER     205      /* double line borders */
#define VERT_BORDER      186
#define TOP_LEFT_CORNER  201
#define TOP_RIGHT_CORNER 187
#define BOT_LEFT_CORNER  200
#define BOT_RIGHT_CORNER 188
#define HORIZ_BORDER_1   196      /* single line borders */
#define LEFT_INTERSECT   199
#define RIGHT_INTERSECT  182

#define DOWN_CURSOR       80
#define UP_CURSOR         72
#define LEFT_CURSOR       75
#define RIGHT_CURSOR      77
#define ENTER             13

#define NO                 0
#define YES                1
#define QUIT               4      /* menu selection # for quitting program */

#define INIT1              1      /* value for initialize position #1 */
#define INIT2              2      /* value for initialize position #2 */
#define CURRENT            3      /* value for current position */

#define MAX_BRIGHT_STARS  24      /* # of stars in bright star catalog */
#define STAR_NAME_LEN     11      /* # of chars in a star name + '\0'*/
#define ROWS              12      /* # of rows in the bright star window */
#define ROW_START          2      /* row start position for star window */
#define LT_COL             3      /* left column for star window */
#define RT_COL            17      /* right column for star window */

#define FAB_ERR_CHAR       6      /* # of chars allowed in fabrication error
				     response (decimal degrees ) */
#define MAX_FAB_ERR       10      /* maximum fabrication error in degrees */
#define MAX_DIGITS         7      /* maximum # of digits allowed to enter */

#define PI     3.14159265358979
#define NEXT_TO_NOTHING 1e-38
double RADIAN = 180/PI;           /* radian conversion factor */
double BASE_YEAR = 2000;          /* year conversion formulas work in */

#define MONTHS            12

int DAYS [MONTHS] = {31,28,31,30,31,30,31,31,30,31,30,31};

double Q[4][4],              /* arrays to hold matrix values */
       V[4][4],              /* each array holds elements #0 through #3 */
       R[4][4],
       X[4][4],
       Y[4][4];



struct pos_struct {
   char name[STAR_NAME_LEN];      /* object name */
   double elev,                   /* elevation */
	  horiz,                  /* horizon (meas. CW) */
	  ra_2000_deg,            /* RA in degrees precessed to year 2000 */
	  dec_2000,               /* declination precessed to year 2000 */
	  sid_time,               /* sideral time of position */
	  coord_year,             /* year of coordinates */
	  ra_hr,                  /* RA hours */
	  ra_min,                 /* RA minutes */
	  ra_sec,                 /* RA seconds */
	  dec_deg,                /* declination degrees */
	  dec_min,                /* declination minutes */
	  Julian,                 /* Julian date of time and date */
	  Julian_0hrUT,           /* Julian date at 0 hrs Universal Time */
	  tm_hr,                  /* time hours */
	  tm_min,                 /* time minutes */
	  tm_sec,                 /* time seconds */
	  dt_year,                /* date year */
	  dt_mon,                 /* date month */
	  dt_day,                 /* date second */
	  timezone,               /* timezone (hrs) */
	  daysav;                 /* daylight savings (hrs) */
  };


struct time_date {
   double year,
	  mon,
	  day,
	  hr,
	  min,
	  sec;
  };


char BRIGHT_STARS[MAX_BRIGHT_STARS][STAR_NAME_LEN] = {
   "Adhara    ",
   "Alderbaran",
   "Alnilam   ",
   "Altair    ",
   "Antares   ",
   "Arcturus  ",
   "Bellatrix ",
   "Betelguese",
   "Capella   ",
   "Castor    ",
   "Deneb     ",
   "Dubhe     ",
   "Elnath    ",
   "Fomalhaut ",
   "Mirfak    ",
   "Polaris   ",
   "Pollux    ",
   "Procyon   ",
   "Regulus   ",
   "Rigel     ",
   "Shaula    ",
   "Sirius    ",
   "Spica     ",
   "Vega      ",
  };

double BRIGHT_STARS_COORD_YEAR = 2000;      /* bright stars' coord year */

float BRIGHT_STAR_POS[MAX_BRIGHT_STARS][5] = {
			{ 6, 58, 38, -28, -58.3},   /* "Adhara    " */
			{ 4, 35, 55,  16,  30.5},   /* "Alderbaran" */
			{ 5, 36, 13,  -1, -12.1},   /* "Alnilam   " */
			{19, 50, 47,   8,  52.1},   /* "Altair    " */
			{16, 29, 24, -26, -25.9},   /* "Antares   " */
			{14, 15, 40,  19,  11  },   /* "Arcturus  " */
			{ 5, 25,  8,   6,  21  },   /* "Bellatrix " */
			{ 5, 55, 10,   7,  24.4},   /* "Betelguese" */
			{ 5, 16, 41,  45,  59.9},   /* "Capella   " */
			{ 7, 34, 36,  31,  53.3},   /* "Castor    " */
			{20, 41, 26,  45,  16.8},   /* "Deneb     " */
			{11,  3, 44,  61,  45  },   /* "Dubhe     " */
			{ 5, 26, 18,  28,  36.5},   /* "Elnath    " */
			{22, 57, 39, -29, -37.3},   /* "Fomalhaut " */
			{ 3, 24, 19,  49,  51.7},   /* "Mirfak    " */
			{ 2, 31, 50,  89,  15.9},   /* "Polaris   " */
			{ 7, 45, 19,  28,   1.6},   /* "Pollux    " */
			{ 7, 39, 18,   5,  13.5},   /* "Procyon   " */
			{10,  8, 22,  11,  58  },   /* "Regulus   " */
			{ 5, 14, 32,  -8, -12.1},   /* "Rigel     " */
			{17, 33, 36, -37,  -6.2},   /* "Shaula    " */
			{ 6, 45,  9, -16, -43  },   /* "Sirius    " */
			{13, 25, 12, -11,  -9.7},   /* "Spica     " */
			{18, 36, 56,  38, 47   },   /* "Vega      " */
  };






void draw_main_screen(void);
void draw_border(int left, int top, int right, int bottom);
void get_select(int *selectptr);
int get_valid_key(void);
void write_menu(int *selectptr);
void check_init(int *init_flagptr);
void write_yes_no_choice(int col, int row, int *responseptr);
void cannot_convert_msg(void);
void get_yes_no_response(int col, int row, int *responseptr);
void get_fab_err(double *Z1ptr, double *Z2ptr, double *Z3ptr);
void check_change_fab_err(int *responseptr);
void change_fab_err(double *Z1ptr, double *Z2ptr, double *Z3ptr);
void display_fab_err(double *Z1ptr, double *Z2ptr, double *Z3ptr);
void get_init(int position, struct pos_struct *posptr);
void sav_screen(char *bufptr);
void put_screen(char *bufptr);
void select_time_option(int *responseptr);
void get_system_time_date(struct pos_struct *posptr);
void input_time(struct pos_struct *posptr);
void get_timezone(struct pos_struct *posptr);
void display_bright_stars(void);
void use_bright_stars(int *responseptr);
void get_bright_star_coord(struct pos_struct *posptr);
void get_coordinates(struct pos_struct *posptr);
int get_valid_star_key(void);
void refresh_bright_stars(int select);
void process_precess(struct pos_struct *ptr);
void calc_precess(double end_year, double beg_year, double *RA,
		  double *DEC);
void LT_UT(struct time_date *UTptr, struct pos_struct *posptr);
void Julian(struct time_date *UTptr, struct pos_struct *posptr);
void sidereal_time(struct time_date *UTptr, struct pos_struct *posptr);
void display_coord(int coord_type, struct pos_struct *posptr);
void get_scope(struct pos_struct *posptr);
void init_arrays(int init, struct pos_struct *posptr, double Z1, double Z2,
		 double Z3);
void subr_750(double F, double H, double Z1, double Z2);
void reinit_arrays(void);
void determinant_subr(double *Wptr);
void get_equat_to_scope(struct pos_struct *posptr, double Z1, double Z2,
			double Z3, int *run_real_time_flag_ptr);
void get_scope_to_equat(struct pos_struct *posptr, double Z1, double Z2,
			double Z3, int *run_real_time_flag_ptr);
void calc_scope(struct pos_struct *posptr, double Z1, double Z2, double Z3);
void angle_subr(double *Fptr, double *Hptr);
void subr_785(double F, double H, double Z1, double Z2);
void calc_equat(struct pos_struct *posptr, double Z1, double Z2, double Z3);
void decode_RA_dec(struct pos_struct *posptr);
void use_previous_equat(int *responseptr);
void use_previous_scope(int *responseptr);
void run_equat_to_scope_real_time(struct pos_struct *posptr, double Z1,
				  double Z2, double Z3);
void run_scope_to_equat_real_time(struct pos_struct *posptr, double Z1,
				  double Z2, double Z3);
void blink_message(void);
void highlight_selection(void);
void base_text_attr(void);













void main(void)
{
 char screen_buffer[25*80*2];     /* buffer to hold screen */
 int select=1,                    /* menu selection; set to option #1 */
     init_flag=0,                 /* initialize flag: if '1', program (scope)
				     has been set to two known positions */
 run_real_time_flag;              /* flag to set program in continuous real
				     time updating mode */
 double Z1=0,
	Z2=0,
	Z3=0;
 struct pos_struct init1,         /* initialized position #1 */
		   init2,         /* initialized position #2 */
		   current;       /* current position */



 current.coord_year=0;            /* set to 0.  current.coord_year is used as
				     as flag to detect if coordinates have
				     been placed in current */

 /* prepare screen */
 _setcursortype(_NOCURSOR);
 _wscroll = 0;                            /* cancel word wrapping */
 base_text_attr();
 clrscr();                                /* fill screen with attribute */

 draw_main_screen();
 display_fab_err(&Z1, &Z2, &Z3);          /* initial display of fab errors */
 write_menu(&select);                     /* initial writing of menu */
 get_select(&select);
 while (select != QUIT) {
    switch (select) {
       case 1 :
	  if (init_flag==YES) {             /* program already initialized */
	     sav_screen(screen_buffer);
	     check_init(&init_flag);
	     put_screen(screen_buffer);
	    }
	  if (init_flag==NO) {              /* if okay to initialize */
	     reinit_arrays();		    /* re-initialize arrays to 0 */

	     sav_screen(screen_buffer);
	     get_fab_err(&Z1, &Z2, &Z3);    /* get new fabrication errors */
	     put_screen(screen_buffer);
	     display_fab_err(&Z1, &Z2, &Z3);

	     sav_screen(screen_buffer);
	     get_init(INIT1, &init1);       /* get initial position #1 */
	     put_screen(screen_buffer);
	     init_arrays(INIT1, &init1,     /* initialize arrays */
			 Z1, Z2, Z3);
	     display_coord(INIT1, &init1);  /* display init1 values now */

	     sav_screen(screen_buffer);
	     get_init(INIT2, &init2);       /* get initial position #2 */
	     put_screen(screen_buffer);
	     init_arrays(INIT2, &init2,     /* initialize arrays */
			 Z1, Z2, Z3);
	     display_coord(INIT2, &init2);  /* display init2 values now */

	     init_flag=YES;                 /* set initialize flag to YES */
	    };
	  break;

       case 2 :
	  if (init_flag==NO) {
	     sav_screen(screen_buffer);
	     cannot_convert_msg();
	     put_screen(screen_buffer);
	    }
	  else {
             /* equat->scope */
	     sav_screen(screen_buffer);
	     get_equat_to_scope(&current, Z1, Z2, Z3, &run_real_time_flag);
	     put_screen(screen_buffer);
	     display_coord(CURRENT, &current);
	     if (run_real_time_flag)
		/* run and continuously update coordinates in real time */
		run_equat_to_scope_real_time(&current, Z1, Z2, Z3);
	     };     /* else */
	  break;

       case 3 :
	  if (init_flag==NO) {
	     sav_screen(screen_buffer);
	     cannot_convert_msg();
	     put_screen(screen_buffer);
	    }
	  else {
	     /* scope->equat */
	     sav_screen(screen_buffer);
	     get_scope_to_equat(&current, Z1, Z2, Z3, &run_real_time_flag);
	     put_screen(screen_buffer);
	     display_coord(CURRENT, &current);
	     if (run_real_time_flag)
		/* run and continuously update coordinates in real time */
		run_scope_to_equat_real_time(&current, Z1, Z2, Z3);
	    };     /* else */

      }     /* switch */
    get_select(&select);
   }     /* while */

 window(MAIN_WINDOW_LEFT, MAIN_WINDOW_TOP, MAIN_WINDOW_RIGHT,
	MAIN_WINDOW_BOTTOM+1);
 _setcursortype(_NORMALCURSOR);
 normvideo();
 clrscr();
}     /* main */






/* This function draws the main screen. */

void draw_main_screen(void)
{
 int col,
     row,
     count;

 window(MAIN_WINDOW_LEFT, MAIN_WINDOW_TOP, MAIN_WINDOW_RIGHT,
	MAIN_WINDOW_BOTTOM);
 draw_border(MAIN_WINDOW_LEFT, MAIN_WINDOW_TOP, MAIN_WINDOW_RIGHT,
	MAIN_WINDOW_BOTTOM);
 gotoxy(19,MAIN_WINDOW_TOP);           /* write title, author */
 cprintf(" COORDINATE TRANSLATION    by Mel Bartels ");

 /* write fabrication errors portion */

 gotoxy(col=2,row=20);                 /* write border */
 for (count=col; count < MAIN_WINDOW_RIGHT; count++)
    putch(HORIZ_BORDER_1);
 gotoxy(1,row);                        /* write left border edge */
 putch(LEFT_INTERSECT);
                                       /* write right border edge */
 gotoxy(MAIN_WINDOW_RIGHT - MAIN_WINDOW_LEFT + 1, row);
 putch(RIGHT_INTERSECT);
 gotoxy(15,row);
 cprintf(" Fabrication Errors ");     /* write title */
 gotoxy(col=5,row+=1);
 cprintf("offset of elevation to perpendicular of horizon:");
 gotoxy(col,row+=1);
 cprintf("optical axis pointing error in same plane:");
 gotoxy(col,row+=1);
 cprintf("correction to zero setting of elevation:");

 /* write 'initilized #2' screen portion */
 gotoxy(col=2,row=16);                 /* write border */
 for (count=col; count < MAIN_WINDOW_RIGHT; count++)
    putch(HORIZ_BORDER_1);
 gotoxy(1,row);                        /* write left border edge */
 putch(LEFT_INTERSECT);
                                       /* write right border edge */
 gotoxy(MAIN_WINDOW_RIGHT - MAIN_WINDOW_LEFT + 1, row);
 putch(RIGHT_INTERSECT);
 gotoxy(15,row);
 cprintf(" Initialized Position #2 ");   /* write title */
 gotoxy(5,row+=1);
 cprintf("ELEV:");
 gotoxy(23,row);
 cprintf("HORIZ:");
 gotoxy(43,row);
 cprintf("TIME:");
 gotoxy(5,row+=1);
 cprintf("YEAR:");
 gotoxy(17,row);
 cprintf("RA:");
 gotoxy(38,row);
 cprintf("DEC:");
 gotoxy(58,row);
 cprintf("DESCRPT:");

 /* write 'initilized #1' screen portion */

 gotoxy(col=2,row=12);                 /* write border */
 for (count=col; count < MAIN_WINDOW_RIGHT; count++)
    putch(HORIZ_BORDER_1);
 gotoxy(1,row);                        /* write left border edge */
 putch(LEFT_INTERSECT);
				       /* write right border edge */
 gotoxy(MAIN_WINDOW_RIGHT - MAIN_WINDOW_LEFT + 1, row);
 putch(RIGHT_INTERSECT);
 gotoxy(15,row);
 cprintf(" Initialized Position #1 ");   /* write title */
 gotoxy(5,row+=1);
 cprintf("ELEV:");
 gotoxy(23,row);
 cprintf("HORIZ:");
 gotoxy(43,row);
 cprintf("TIME:");
 gotoxy(5,row+=1);
 cprintf("YEAR:");
 gotoxy(17,row);
 cprintf("RA:");
 gotoxy(38,row);
 cprintf("DEC:");
 gotoxy(58,row);
 cprintf("DESCRPT:");

 /* write current position screen portion */

 gotoxy(col=2,row=7);                  /* write border */
 for (count=col; count < MAIN_WINDOW_RIGHT; count++)
    putch(HORIZ_BORDER_1);
 gotoxy(1,row);                        /* write left border edge */
 putch(LEFT_INTERSECT);
                                       /* write right border edge */
 gotoxy(MAIN_WINDOW_RIGHT - MAIN_WINDOW_LEFT + 1, row);
 putch(RIGHT_INTERSECT);
 gotoxy(15,row);
 cprintf(" Current Position ");      /* write title */
 gotoxy(5,row+=2);
 cprintf("ELEV:");
 gotoxy(23,row);
 cprintf("HORIZ:");
 gotoxy(43,row);
 cprintf("TIME:");
 gotoxy(5,row+=1);
 cprintf("YEAR:");
 gotoxy(17,row);
 cprintf("RA:");
 gotoxy(38,row);
 cprintf("DEC:");
 gotoxy(58,row);
 cprintf("DESCRPT:");

}     /* draw_main_screen */






/* This function draws a window's border. */

void draw_border(int left, int top, int right, int bottom)
{
 int col = right-left+1,
     row = bottom-top+1,
     count;

 gotoxy(2,1);
 for (count=2; count<col; count++)     /* draw the top border */
    putch(HORIZ_BORDER);

 gotoxy(2,row);
 for (count=2; count<col; count++)     /* draw the bottom border */
    putch(HORIZ_BORDER);

 for (count=2; count<row; count++) {   /* draw the right border */
    gotoxy(col,count);
    putch(VERT_BORDER);
   }

 for (count=2; count<row; count++) {   /* draw the left border */
    gotoxy(1,count);
    putch(VERT_BORDER);
   }

 gotoxy(1,1);                          /* draw the top left corner */
 putch(TOP_LEFT_CORNER);

 gotoxy(col,1);                        /* draw the top right corner */
 putch(TOP_RIGHT_CORNER);

 gotoxy(1,row);                        /* draw the bottom left corner */
 putch(BOT_LEFT_CORNER);

 gotoxy(col,row);                      /* draw the bottom right corner */
 putch(BOT_RIGHT_CORNER);
}     /* function draw_border */






/* This function processes the user's menu selection. */

void get_select(int *selectptr)
{
 char ch;                              /* keyboard response field */

 /* get keyboard response (either left cursor, right cursor, or return) */

 while ( (ch=get_valid_key()) !=ENTER ) {
    switch (ch) {
       case RIGHT_CURSOR :
	  *selectptr+=1;                 /* move marker one to the right */
	  if (*selectptr>QUIT)           /* wrap around from right to left */
	     *selectptr=1;
	  break;
       case LEFT_CURSOR :
	  *selectptr-=1;                 /* move marker one to the left */
	  if (*selectptr<1)              /* wrap around from left to right */
	     *selectptr=QUIT;
      }     /* switch */
   write_menu(selectptr);
   }     /* while */
 /* exit menu when ENTER response: set selection to current marker */
}     /* function get_select */






/* This function gets a valid keyboard response. */

int get_valid_key(void)
{
 char ch;


 while ( (ch=getch()) != ENTER        &&
		   ch != LEFT_CURSOR  &&
		   ch != RIGHT_CURSOR );
 return ch;
}     /* get_valid_key */






/* This function writes the main menu with the current option highlighted. */

void write_menu(int *selectptr)
{
 int col,
     row;

 /* write menu options with current option highlighted; 'textattr' function
    requires use of 'cprintf' function */

 base_text_attr();

 if (*selectptr==1)
    highlight_selection();
 gotoxy(col=5,row=3);   cprintf(" Initialize ");
 if (*selectptr==1)
    base_text_attr();

 if (*selectptr==2)
    highlight_selection();
 gotoxy(col+=20,row);   cprintf(" Equat->scope ");
 if (*selectptr==2)
    base_text_attr();

 if (*selectptr==3)
    highlight_selection();
 gotoxy(col+=22,row);   cprintf(" Scope->equat ");
 if (*selectptr==3)
    base_text_attr();

 if (*selectptr==4)
    highlight_selection();
 gotoxy(col+=22,row);   cprintf(" Quit ");
 if (*selectptr==4)
    base_text_attr();

 /* write menu option explanation */

 gotoxy(col=6,row+=2);
 switch (*selectptr) {
    case 1 :
       cprintf("initialize program by aligning scope on two known positions");
       break;
    case 2 :
       cprintf("transform equatorial coordinates to telescope coordinates  ");
       break;
    case 3 :
       cprintf("transform telescope coordinates to equatorial coordinates  ");
       break;
    case 4 :
       cprintf("quit program                                               ");
   }     /* switch */
}     /* function write_menu */






/* This function verifies re-initializing the program.  If YES response,
   set init_flag to NO, if NO response, keep init_flag = YES. */

void check_init(int *init_flagptr)
{
 int left   = 20,
     right  = 60,
     top    = 6,
     bottom = 13,
     col,
     row,
     response;                    /* yes or no response */


 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 gotoxy(col=9,row=3);   cprintf("Re-initialize program ?");

 response=NO;           /* set menu option to NO to start with */
 write_yes_no_choice(col+=6, row+=2, &response);
 get_yes_no_response(col, row, &response);
 if (response==YES)    /* reset flag to NO if YES option to re-init */
    *init_flagptr=NO;

}     /* check_init */






/* This function writes a 'cannot convert coordinates until program
   initialized' message. */

void cannot_convert_msg(void)
{
 int left   = 20,
     right  = 60,
     top    = 6,
     bottom = 13,
     col,
     row;
 float count;


 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 gotoxy(col=5,row=3);    cprintf("Cannot convert coordinates until");
 gotoxy(col,row+=2);     cprintf("      program initialized.      ");

 for (count=0; count<10000; count++);

}     /* cannot_convert_msg */






/* This function gets a yes or no response. */

void get_yes_no_response(int col, int row, int *responseptr)
{
 char ch;

 /* Get keyboard response (either left cursor, right cursor, or return).
    Display order is Yes on the left, then No on the right. */

 while ( (ch=get_valid_key()) !=ENTER ) {
    switch (ch) {
       case RIGHT_CURSOR :
	  *responseptr-=1;             /* move marker one to the right */
	  if (*responseptr<0)          /* wrap around from right to left */
	     *responseptr=1;
	  break;
       case LEFT_CURSOR :
	  *responseptr+=1;             /* move marker one to the left */
	  if (*responseptr>1)          /* wrap around from left to right */
	     *responseptr=0;
      }     /* switch */
    write_yes_no_choice(col, row, responseptr);
   }     /* while */

 /* exit while when ENTER response */
}     /* get_yes_no_response */






/* This function writes a 'yes' (*ptr==YES) or  'no' (*ptr==NO) menu. */

void write_yes_no_choice(int col, int row, int *responseptr)
{

 /* write menu options with current option highlighted; 'textattr' function
    requires use of 'cprintf' function */

 base_text_attr();

 if (*responseptr==YES)
    highlight_selection();
 gotoxy(col,row);   cprintf(" Yes ");
 if (*responseptr==YES)
    base_text_attr();

 if (*responseptr==NO)
    highlight_selection();
 gotoxy(col+=7,row);   cprintf(" No ");
 if (*responseptr==NO)
    base_text_attr();
}     /* write_yes_no_choice */






/* This function gets the mount's fabrication errors. */

void get_fab_err(double *Z1ptr, double *Z2ptr, double *Z3ptr)

{
 int response;                    /* yes or no response */


 check_change_fab_err(&response);
 if (response==YES)
    change_fab_err(Z1ptr, Z2ptr, Z3ptr);

}     /* get_fab_err */






/* This function checks to see if user wishes to change mounting
   mis-alignment parameters. */

void check_change_fab_err(int *responseptr)
{
 int left   = 20,
     right  = 60,
     top    = 6,
     bottom = 13,
     col,
     row;


 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 gotoxy(col=8,row=3); cprintf("Change fabrication errors ?");

 *responseptr=NO;                 /* set menu option to NO to start with */
 write_yes_no_choice(col+=7, row=5, responseptr);
 get_yes_no_response(col, row, responseptr);
}     /* check_change_fab_err */






/* This function changes the three fabrication error parameters. */

void change_fab_err(double *Z1ptr, double *Z2ptr, double *Z3ptr)
{
 char num_str[FAB_ERR_CHAR+3];
 int left   = 5,
     right  = 75,
     top    = 11,
     bottom = 18,
     col,
     row;


 window(left, top, right, bottom);
 clrscr();
 _setcursortype(_NORMALCURSOR);
 draw_border(left, top, right, bottom);

 gotoxy(col=19,row=2);
 cprintf("Change Fabrication Error Values");
 gotoxy(col=5,row+=2);
 cprintf("offset of elevation to perpendicular of horizon: [      ] deg");
 gotoxy(col,row+=1);
 cprintf("optical axis pointing error in same plane:       [      ] deg");
 gotoxy(col,row+=1);
 cprintf("correction to zero setting of elevation:         [      ] deg");

 /* get fabrication errors */

 num_str[0]=FAB_ERR_CHAR+1;
 do {
    gotoxy(col=55,row=4);
    cprintf("      ");
    gotoxy(col,row);
    } while ( (*Z1ptr = atof(cgets(num_str))) >= MAX_FAB_ERR   ||
	      *Z1ptr <= -MAX_FAB_ERR );
 row++;
 do {
    gotoxy(col,row);
    cprintf("      ");
    gotoxy(col,row);
    } while ( (*Z2ptr = atof(cgets(num_str))) >= MAX_FAB_ERR   ||
	      *Z2ptr <= -MAX_FAB_ERR );
 row++;
 do {
    gotoxy(col,row);
    cprintf("      ");
    gotoxy(col,row);
    } while ( (*Z3ptr = atof(cgets(num_str))) >= MAX_FAB_ERR   ||
	      *Z3ptr <= -MAX_FAB_ERR );

 _setcursortype(_NOCURSOR);

}     /* change_fab_err */






/* This function displays the fabrication errors. */

void display_fab_err(double *Z1ptr, double *Z2ptr, double *Z3ptr)

{
 int col=56,
     row=21;

 gotoxy(col,row);
 cprintf("%8.5f deg",*Z1ptr);
 gotoxy(col,row+=1);
 cprintf("%8.5f deg",*Z2ptr);
 gotoxy(col,row+=1);
 cprintf("%8.5f deg",*Z3ptr);
}     /* display_fab_err */






/* This function gets an initial position. */

void get_init(int position, struct pos_struct *posptr)

{
 char num_str[MAX_DIGITS+3];
 int left   = MAIN_WINDOW_LEFT+3,
     right  = MAIN_WINDOW_RIGHT-3,
     top    = 6,
     bottom = MAIN_WINDOW_BOTTOM+1,
     col    = 17,
     row    = 2,
     response;                    /* user response */
 struct time_date UT;             /* Universal Time */


 /* make window */

 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 /* write title */

 gotoxy(col,row);
 cprintf("INITIALIZE POSITION #%d:", position);

 /* get time */

 select_time_option(&response);
 get_timezone(posptr);                 /* get time zone (including daylight
					  savings) */
 if (response==YES)
    get_system_time_date(posptr);      /* use computer's time & date */
 else
    input_time(posptr);                /* enter new time & date */

 /* get coordinates */

 display_bright_stars();               /* don't highlight selection yet */
 use_bright_stars(&response);

 if (response==YES)
    get_bright_star_coord(posptr);
 else
    get_coordinates(posptr);

 /* calc times */

 LT_UT(&UT, posptr);
 Julian(&UT, posptr);
 sidereal_time(&UT, posptr);

 /* get scope coordinates */

 get_scope(posptr);

 /* restore screen */

 window(left, top, right, bottom);
 clrscr();

}     /* get_init */






/* This function saves the main screen. */

void sav_screen(char *bufptr)
{

 gettext(MAIN_WINDOW_LEFT, MAIN_WINDOW_TOP, MAIN_WINDOW_RIGHT,
	 MAIN_WINDOW_BOTTOM, bufptr);
}     /* sav_screen */






/* This function puts back the previously saved main screen. */

void put_screen(char *bufptr)
{
 window(MAIN_WINDOW_LEFT, MAIN_WINDOW_TOP, MAIN_WINDOW_RIGHT,
	MAIN_WINDOW_BOTTOM);
 puttext(MAIN_WINDOW_LEFT, MAIN_WINDOW_TOP, MAIN_WINDOW_RIGHT,
	 MAIN_WINDOW_BOTTOM, bufptr);
}     /* put_screen */






/* This function asks the user to choose between using system time or
   inputing a different time value. */

void select_time_option(int *responseptr)
{
 int left   = 9,
     right  = left+20,
     top    = 8,
     bottom = 11,
     col,
     row;


 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 gotoxy(col=3,row=2);   cprintf("Use system time?");

 *responseptr=YES;           /* set menu option to YES to start with */
 write_yes_no_choice(col+=2, row+=1, responseptr);
 get_yes_no_response(col, row, responseptr);

}     /* select_time_option */






/* This function gets the system time and date. */

void get_system_time_date(struct pos_struct *posptr)
{
 struct time sys_time;    /* struct that gettime function fills */
 struct date sys_date;    /* struct that getdate function fills */

 gettime(&sys_time);
 getdate(&sys_date);
 posptr->tm_hr   = sys_time.ti_hour;
 posptr->tm_min  = sys_time.ti_min;
 posptr->tm_sec  = sys_time.ti_sec;
 posptr->dt_year = sys_date.da_year;
 posptr->dt_mon  = sys_date.da_mon;
 posptr->dt_day  = sys_date.da_day;

}     /* get_system_time_date */






/* This function gets an inputed time. */

void input_time(struct pos_struct *posptr)
{
 char num_str[4+3];
 int leap_flag,                        /* '1' if leap year, '0' otherwise */
     left=9,
     top=16,
     right=left+20,
     bottom=top+8,
     col=3,
     row=2;

 window(left, top, right, bottom);
 clrscr();
 _setcursortype(_NORMALCURSOR);

 draw_border(left, top, right, bottom);

 gotoxy(col,row);   cprintf("Enter time:      ");
 gotoxy(col,++row); cprintf("hours        [  ]");
 gotoxy(col,++row); cprintf("minutes      [  ]");
 gotoxy(col,++row); cprintf("seconds    [    ]");
 gotoxy(col,++row); cprintf("year       [    ]");
 gotoxy(col,++row); cprintf("month (1-12) [  ]");
 gotoxy(col,++row); cprintf("day   (1-31) [  ]");

 num_str[0]=2+1;
 do {                                  /* get hours */
     gotoxy(col=17,row=3);
     cprintf("  ");
     gotoxy(col,row);
    } while ( (posptr->tm_hr = atoi(cgets(num_str))) <0 ||
	      posptr->tm_hr >=24 );

 row++;
 do {                                  /* get minutes */
     gotoxy(col,row);
     cprintf("  ");
     gotoxy(col,row);
    } while ( (posptr->tm_min = atoi(cgets(num_str))) <0 ||
	      posptr->tm_min >=60 );

 row++;
 num_str[0]=4+1;
 do {                                  /* get seconds */
     gotoxy(col-2,row);
     cprintf("    ");
     gotoxy(col-2,row);
    } while ( (posptr->tm_sec = atof(cgets(num_str))) <0 ||
	      posptr->tm_sec >=60 );

 row++;
 do {                                  /* get year */
     gotoxy(col-2,row);
     cprintf("    ");
     gotoxy(col-2,row);
    } while ( (posptr->dt_year = atoi(cgets(num_str))) <1900 ||
	      posptr->dt_year >3999 );

 row++;
 num_str[0]=3;
 do {                                  /* get month */
     gotoxy(col,row);
     cprintf("  ");
     gotoxy(col,row);
    } while ( (posptr->dt_mon = atoi(cgets(num_str))) <1 ||
	      posptr->dt_mon >12 );

 row++;
 leap_flag = (posptr->dt_mon==2 && (int)posptr->dt_year%4==0 &&
	      (int)posptr->dt_year%1000!=0);
 do {                                  /* get day */
     gotoxy(col,row);
     cprintf("  ");
     gotoxy(col,row);
    } while ( (posptr->dt_day = atoi(cgets(num_str))) <1 ||
	      posptr->dt_day > DAYS[posptr->dt_mon-1] +leap_flag );

 _setcursortype(_NOCURSOR);
 }    /* input_time */






/* This function gets the time zone which includes the daylight savings. */

void get_timezone(struct pos_struct *posptr)

{
 char num_str[2+3];
 int left=6,
     top=12,
     right=left+29,
     bottom=top+3,
     col=3,
     row=2;

 window(left, top, right, bottom);
 clrscr();
 _setcursortype(_NORMALCURSOR);
 draw_border(left, top, right, bottom);

 gotoxy(col,row);
 cprintf("Timezone (include daylight");
 gotoxy(col,row+=1);
 cprintf("   savings):    [  ]      ");

 num_str[0]=2+1;
 do {                                  /* get timezone + daylight savings */
     gotoxy(col=20,row);
     cprintf("  ");
     gotoxy(col,row);
    } while ( (posptr->timezone = atoi(cgets(num_str))) < 0 ||
	      posptr->timezone > 24);

 _setcursortype(_NOCURSOR);
 }     /* get_timezone */






/* This function displays the bright stars. */

void  display_bright_stars(void)

{
 int left   = STARS_LEFT,
     right  = STARS_RIGHT,
     top    = STARS_TOP,
     bottom = STARS_BOTTOM,
     col,
     row,
     pos;                    /* array position */


 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 for (col=LT_COL, row=ROW_START, pos=0 ; row<ROWS+ROW_START; row++, pos++) {
    gotoxy(col, row);
    cprintf("%-10s", BRIGHT_STARS[pos]);
   };

 for (col=RT_COL, row=ROW_START; row<ROWS+ROW_START; row++, pos++) {
    gotoxy(col, row);
    cprintf("%-10s", BRIGHT_STARS[pos]);
   };

}    /* display_bright_stars */






/* This function asks the user if they want to input coordinates from the
   bright star catalog. */

void use_bright_stars(int *responseptr)
{
 int left   = 48,
     right  = 75,
     top    = 7,
     bottom = 10,
     col,
     row;


 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 gotoxy(col=3,row=2); cprintf("Select 24 bright stars?");

 *responseptr=YES;                /* set menu option to YES to start with */
 write_yes_no_choice(col+=7, row=3, responseptr);
 get_yes_no_response(col, row, responseptr);

}     /* select_bright_stars */






/* This function fills a position structure with a bright star coordinate. */

void get_bright_star_coord(struct pos_struct *posptr)

{
 char ch;                         /* keyboard response */
 int select=0,                    /* user's bright star choice */
     pos;                         /* array position */


 /* get keyboard response (either left cursor, right cursor, or return) */

 refresh_bright_stars(select);    /* set initial selection to star '0' */
 while ( (ch=get_valid_star_key()) !=ENTER ) {
    switch (ch) {
       case DOWN_CURSOR :
	  select+=1;                        /* move marker down one */
	  if (select>MAX_BRIGHT_STARS-1)    /* wrap from bottom to top */
	     select=0;
	  break;
       case UP_CURSOR :
	  select-=1;                        /* move marker up one */
	  if (select<0)                     /* wrap from bottom to top */
	     select=MAX_BRIGHT_STARS-1;
	  break;
       case RIGHT_CURSOR :
	  select+=ROWS;                     /* move marker to the right */
	  if (select>MAX_BRIGHT_STARS-1)    /* wrap from right to left */
	     select-=ROWS*2;
	  break;
       case LEFT_CURSOR :
	  select-=ROWS;                     /* move marker one to the left */
	  if (select<0)                     /* wrap from left to right */
	     select+=ROWS*2;
      }     /* switch */
   refresh_bright_stars(select);
   };    /* while */              /* exit while when ENTER response */

 /* fill position structure with bright star's coordinates from array */

 strcpy(posptr->name, BRIGHT_STARS[select]);
 posptr->coord_year = BRIGHT_STARS_COORD_YEAR;
 posptr->ra_hr      = BRIGHT_STAR_POS[select][0];
 posptr->ra_min     = BRIGHT_STAR_POS[select][1];
 posptr->ra_sec     = BRIGHT_STAR_POS[select][2];
 posptr->dec_deg    = BRIGHT_STAR_POS[select][3];
 posptr->dec_min    = BRIGHT_STAR_POS[select][4];

 /* fill position structure with calculated RA and dec in degrees -
    precession not necessary since star coordinates are for year 2000 */

 posptr->ra_2000_deg = 15 * ( posptr->ra_hr + posptr->ra_min/60 +
		       posptr->ra_sec/3600 );
 posptr->dec_2000 = posptr->dec_deg + posptr->dec_min/60;

}     /* get_bright_star_coord */






/* This function refreshes the bright star display with the current bright
   star selection highlighted. */

void refresh_bright_stars(int select)

{
 int left   = STARS_LEFT,
     right  = STARS_RIGHT,
     top    = STARS_TOP,
     bottom = STARS_BOTTOM,
     col,
     row;
 static int previous;             /* previously highlighted selection */


 window(left, top, right, bottom);

 /* change previously highlighted selection to normal video display */

 base_text_attr();
 if (previous>ROWS-1) {
    col=RT_COL;
    row=previous+ROW_START-ROWS;
   }
 else {
    col=LT_COL;
    row=previous+ROW_START;
   }
 gotoxy(col,row);
 cprintf("%-10s", BRIGHT_STARS[previous]);

 /* write bright star options with current option highlighted;
    'textattr' function requires use of 'cprintf' function. */

 if (select>ROWS-1) {
    col=RT_COL;
    row=select+ROW_START-ROWS;
   }
 else {
    col=LT_COL;
    row=select+ROW_START;
   }
 gotoxy(col,row);
 highlight_selection();
 cprintf("%-10s", BRIGHT_STARS[select]);
 base_text_attr();

 previous=select;                 /* save selection */

}    /* refresh_bright_stars */






/* This function gets a valid key selection for the bright star catalog
   window. */

int get_valid_star_key(void)

{
 char ch;


 while ( (ch=getch()) != ENTER        &&
		   ch != LEFT_CURSOR  &&
		   ch != RIGHT_CURSOR &&
		   ch != UP_CURSOR    &&
		   ch != DOWN_CURSOR );
 return ch;

}     /* get_valid_star_key */






/* This function fills a position structure with inputed coordinates. */

void get_coordinates(struct pos_struct *posptr)

{
 char ch,
      num_str[6+3],
      description[STAR_NAME_LEN+2];
 int minus_flag,
     left   = STARS_LEFT,
     right  = STARS_RIGHT,
     top    = STARS_TOP,
     bottom = STARS_BOTTOM,
     col    =  3,
     row    =  2;


 window(left, top, right, bottom);
 clrscr();
 _setcursortype(_NORMALCURSOR);
 draw_border(left, top, right, bottom);

 gotoxy(col, row);      cprintf("Enter Coordinates:       ");
 gotoxy(col,++row);     cprintf("                         ");
 gotoxy(col,++row);     cprintf("Description [          ] ");
 gotoxy(col,++row);     cprintf("Coordinate's year [    ] ");
 gotoxy(col,++row);     cprintf("Right ascension:         ");
 gotoxy(col,++row);     cprintf("    hours       [  ]     ");
 gotoxy(col,++row);     cprintf("    minutes  [     ]     ");
 gotoxy(col,++row);     cprintf("    seconds  [     ]     ");
 gotoxy(col,++row);     cprintf("Declination:             ");
 gotoxy(col,++row);     cprintf("    degrees    [   ]     ");
 gotoxy(col,++row);     cprintf("    minutes [      ]     ");

 /* get object's description */
 gotoxy(16,4);
 description[0]=STAR_NAME_LEN;
 strcpy(posptr->name, cgets(description));

 num_str[0]=4+1;
 do {
     gotoxy(col=22,row=5);
     cprintf("    ");
     gotoxy(col,row);
  } while ( (posptr->coord_year = atof(cgets(num_str))) < 1800 ||
	     posptr->coord_year > 2100 );

 num_str[0]=2+1;
 do {
     gotoxy(col=20,row=7);
     cprintf("  ");
     gotoxy(col,row);
  } while ( (posptr->ra_hr = atof(cgets(num_str))) < 0 ||
	     posptr->ra_hr > 23 );


 num_str[0]=5+1;
 do {
     gotoxy(col=17,row=8);
     cprintf("     ");
     gotoxy(col,row);
  } while ( (posptr->ra_min = atof(cgets(num_str))) < 0 ||
	     posptr->ra_min >= 60 );

 do {
     gotoxy(col,row=9);
     cprintf("     ");
     gotoxy(col,row);
  } while ( (posptr->ra_sec = atof(cgets(num_str))) < 0 ||
	     posptr->ra_sec >= 60 );

 num_str[0]=3+1;
 do {
     gotoxy(col=19,row=11);
     cprintf("   ");
     gotoxy(col,row);
  } while ( (posptr->dec_deg=atof(cgets(num_str))) < -90 ||
	     posptr->dec_deg > 90 );

 num_str[0]=6+1;
 do {
     gotoxy(col=16,row=12);
     cprintf("     ");
     gotoxy(col,row);
  } while ( (posptr->dec_min=atof(cgets(num_str))) <= -60 ||
	     posptr->dec_min >= 60 );

 _setcursortype(_NOCURSOR);

 /* correct for minus declination */

 minus_flag = 0;                      /* set flag */
 if (posptr->dec_deg < 0 || posptr->dec_min < 0)
    minus_flag = 1;

 if (posptr->dec_deg < 0)             /* make degrees positive */
    posptr->dec_deg = -posptr->dec_deg;
 if (posptr->dec_min < 0)             /* make minutes positive */
    posptr->dec_min = -posptr->dec_min;

 if (minus_flag) {                         /* if minus declination, set */
    posptr->dec_deg = -posptr->dec_deg;    /* degrees to minus and set  */
    posptr->dec_min = -posptr->dec_min;    /* minutes to minus.         */
  };

 /* precess coordinates which fills structure with RA and dec in degrees */
 process_precess(posptr);

}     /* get_coordinates */






/* This function processes the necessary values from the object's position
   structure and sends them to the calculate precession module. */

void process_precess(struct pos_struct *ptr)
{
 double RA,                            /* right ascension in degrees */
	DEC;                           /* declination in degrees */


 /* calculate RA in degrees */
 RA = (ptr->ra_hr + ptr->ra_min/60 + ptr->ra_sec/3600) * 15;

 /* calculate DEC in degrees */
 DEC = ptr->dec_deg + ptr->dec_min/60;

 calc_precess(BASE_YEAR, ptr->coord_year, &RA, &DEC);

 ptr->ra_2000_deg = RA;
 ptr->dec_2000 = DEC;

}     /* process_precess */






/* This function calculates precession. */

void calc_precess(double end_year, double beg_year, double *RA,
		  double *DEC)
{
 double U,O,P;                    /* intermediate calculated results */


 U = ((end_year + beg_year)/2 - 1900)/100;
 O = 3.07234 + .00186 * U;
 P = 20.0468 - .0085 * U;

 /* precess coordinates */
 *RA += ( O + (P/15) * sin(*RA/RADIAN) * tan(*DEC/RADIAN) ) *
	(end_year - beg_year)/240;

 *DEC += ( P * cos(*RA/RADIAN) * (end_year - beg_year) ) / 3600;

}     /* calc_precess */







/* This function converts local time to universal time. */

void LT_UT(struct time_date *UTptr, struct pos_struct *posptr)
{
 int leap_flag;

 UTptr->sec  = posptr->tm_sec;
 UTptr->min  = posptr->tm_min;
 UTptr->hr   = posptr->tm_hr + posptr->timezone;
 UTptr->day  = posptr->dt_day;
 UTptr->mon  = posptr->dt_mon;
 UTptr->year = posptr->dt_year;

 if (UTptr->hr >= 24) {
    UTptr->day += 1;
    UTptr->hr -= 24;

    leap_flag = (UTptr->mon==2 && (int)UTptr->year%4==0 &&
		 (int)UTptr->year%1000!=0);

    if ( UTptr->day > DAYS[UTptr->mon -1]+leap_flag ) {
       UTptr->mon += 1;
       UTptr->day = 1;

       if (UTptr->mon > MONTHS) {
	  UTptr->year += 1;
	  UTptr->mon = 1;
	}
     }
  }
}     /* LT_UT */







/* This function calculates the Julian date. */

void Julian(struct time_date *UTptr, struct pos_struct *posptr)
{
 long Y,M,A,B,C,D;                     /* intermediate calculated results */
 double cal_date,     		       /* calendar date (YYYYMMDD) */
	GREG_DATE=15821015;            /* start of Gregorian calendar,ie, */
				       /* 1582 AD, Oct 15 */
 Y = UTptr->year;
 M = UTptr->mon;
 if (M==1 || M==2) {
    Y -= 1;
    M += 12;
  }
 A = (long) (Y/100);
 B = 0;

 cal_date = UTptr->year*10000 + UTptr->mon*100 +UTptr->day;
 if (cal_date > GREG_DATE)
    B = 2-A+ (long) (A/4);

 C = (long) (365.25*Y);
 D = (long) (30.6001 * (M+1));

 posptr->Julian_0hrUT = B + C + D + UTptr->day + 1720994.5;
 posptr->Julian = posptr->Julian_0hrUT +  UTptr->hr/24 + UTptr->min/(24*60) +
		  UTptr->sec/(24*60*60);

}     /* Julian */






/* This function calculates the sidereal time. */

void sidereal_time(struct time_date *UTptr, struct pos_struct *posptr)
{
 double SID_RATE=1.002737908,          /* sidereal time rate */
	T,                             /* intermediate calculated result */
	sid_time_0hrUT,                /* sidereal time at 0 hrs UT */
	UT_hr,                         /* UT hr, min, sec in decimal hours */
	sid_time_day,                  /* sidereal time since 0 hrs UT */
	sid_time;                      /* total sidereal time */


 T = (posptr->Julian_0hrUT - 2415020)/36525;
 sid_time_0hrUT = 6.6460656 + 2400.051262*T +.00002581*T*T;

 UT_hr = UTptr->hr + UTptr->min/60 + UTptr->sec/3600;
 sid_time_day = UT_hr * SID_RATE;
 sid_time = sid_time_0hrUT + sid_time_day;

 while (sid_time >= 24)
    sid_time -= 24;

 posptr->sid_time = sid_time;          /* assign sidereal time */

}     /* sidereal_time */






/* This function gets the scope's coordinates: elevation and horizon. */

void get_scope(struct pos_struct *posptr)
{
 char num_str[7+3];               /* 7 chars + 3 more required by 'cgets' */
 int left   = 20,
     right  = left+40,
     top    = 14,
     bottom = top+7,
     col,
     row;


 window(left, top, right, bottom);
 clrscr();
 _setcursortype(_NORMALCURSOR);
 draw_border(left, top, right, bottom);

 gotoxy(col=3, row=2); cprintf("         Scope Coordinates:       ");
 gotoxy(col, row+=2);  cprintf("      Elevation           [      ]");
 gotoxy(col, row+=2);  cprintf("  Horizon (measured CW)  [       ]");

 /* get elevation */
 num_str[0]=6+1;
 do {
     gotoxy(col=30,row=4);
     cprintf("      ");
     gotoxy(col,row);
  } while ((posptr->elev = atof(cgets(num_str))) < 0 || posptr->elev > 90);

 /* get horizon */
 num_str[0]=7+1;
 do {
     gotoxy(col=29,row=6);
     cprintf("       ");
     gotoxy(col,row);
  } while ((posptr->horiz = atof(cgets(num_str))) < 0 ||
	    posptr->horiz > 360);

 _setcursortype(_NOCURSOR);

  /* convert horizon to CCW */
 posptr->horiz=360-posptr->horiz;

}     /* get_scope */






/* This function displays the coordinates on screen. */

void display_coord(int coord_type, struct pos_struct *posptr)
{
 int col,
     row;


 switch (coord_type) {
    case INIT1   : row=13;
		   break;
    case INIT2   : row=17;
		   break;
    case CURRENT : row=9;
  }     /* switch */

 gotoxy(11,row);
 cprintf("%6.3fd", posptr->elev);
 gotoxy(30,row);
 cprintf("%7.3fd", 360 - posptr->horiz);
 gotoxy(49,row);
 cprintf("%2.0fh %2.0fm %2.0fs %2.0ftz %2.0fm %2.0fd %4.0fy", posptr->tm_hr,
	posptr->tm_min, posptr->tm_sec, posptr->timezone, posptr->dt_mon,
	posptr->dt_day, posptr->dt_year);

 gotoxy(11,++row);
 cprintf("%4.0f", posptr->coord_year);
 gotoxy(21,row);
 cprintf("%2.0fh %5.2fm %2.0fs", posptr->ra_hr, posptr->ra_min,
	 posptr->ra_sec);
 gotoxy(43,row);
 cprintf("%3.0fd %6.2fm", posptr->dec_deg, posptr->dec_min);
 gotoxy(67,row);
 cprintf("%-10s", posptr->name);

}     /* display_coord */






/* This function re-initializes the five arrays to 0. */

void reinit_arrays(void)
{
 int cnt1,                   /* counter #1 */
     cnt2;                   /* counter #1 */


 for (cnt1=0; cnt1<4; cnt1++)
    for (cnt2=0; cnt2<4; cnt2++) {
       Q[cnt1][cnt2]=0;
       V[cnt1][cnt2]=0;
       R[cnt1][cnt2]=0;
       X[cnt1][cnt2]=0;
       Y[cnt1][cnt2]=0;
      };

}     /* reinit_arrays */






/* This function initializes the arrays. The function follows the Sky and
   Telescope program from pg. 194-196, Feb. 1989. */

void init_arrays(int init, struct pos_struct *posptr, double Z1, double Z2,
		 double Z3)
{
 int I, J, M, N, L;          /* counters */
 double D,                   /* declination/RADIAN */
	B,                   /* (RA - sidereal time in degrees) / RADIAN */
	F,                   /* horiz/RADIAN */
	H,                   /* (elev + Z3)/RADIAN */
	A, E, W;             /* intermediate calculated values */



 D = posptr->dec_2000 / RADIAN;
 B = (posptr->ra_2000_deg - 15*posptr->sid_time) / RADIAN;

 X[1][init] = cos(D)*cos(B);
 X[2][init] = cos(D)*sin(B);
 X[3][init] = sin(D);

 F = (posptr->horiz) / RADIAN;
 H = (posptr->elev + Z3) / RADIAN;

 subr_750(F, H, Z1, Z2);

 Y[1][init] = Y[1][0];
 Y[2][init] = Y[2][0];
 Y[3][init] = Y[3][0];

 if (init==INIT2) {
    X[1][3] = X[2][1] * X[3][2] - X[3][1] * X[2][2];
    X[2][3] = X[3][1] * X[1][2] - X[1][1] * X[3][2];
    X[3][3] = X[1][1] * X[2][2] - X[2][1] * X[1][2];

    A = sqrt( pow(X[1][3], 2) + pow(X[2][3], 2) +pow(X[3][3], 2) );
    if (A==0)
       A=NEXT_TO_NOTHING;

    for (I=1; I<=3; I++)
	X[I][3] /= A;

    Y[1][3] = Y[2][1] * Y[3][2] - Y[3][1] * Y[2][2];
    Y[2][3] = Y[3][1] * Y[1][2] - Y[1][1] * Y[3][2];
    Y[3][3] = Y[1][1] * Y[2][2] - Y[2][1] * Y[1][2];

    A = sqrt( pow(Y[1][3], 2) + pow(Y[2][3], 2) +pow(Y[3][3], 2) );
    if (A==0)
       A=NEXT_TO_NOTHING;

    for (I=1; I<=3; I++)
	Y[I][3] /= A;

    /* transform matrix */

    for (I=1; I<=3; I++)
       for (J=1; J<=3; J++)
	  V[I][J] = X[I][J];

    determinant_subr(&W);
    E = W;
    if (E==0)
       E=NEXT_TO_NOTHING;

    for (M=1; M<=3; M++) {
       for (I=1; I<=3; I++)
	  for (J=1; J<=3; J++)
	     V[I][J] = X[I][J];
       for (N=1; N<=3; N++) {
	  V[1][M] = 0;
	  V[2][M] = 0;
	  V[3][M] = 0;
	  V[N][M] = 1;
	  determinant_subr(&W);
	  Q[M][N] = W/E;
	 }
      }

    for (I=1; I<=3; I++)
       for (J=1; J<=3; J++)
	  R[I][J] = 0;

    for (I=1; I<=3; I++)
       for (J=1; J<=3; J++)
	  for (L=1; L<=3; L++)
	     R[I][J] += ( Y[I][L] * Q[L][J] );

    for (M=1; M<=3; M++) {
       for (I=1; I<=3; I++)
	  for (J=1; J<=3; J++)
	     V[I][J] = R[I][J];
       determinant_subr(&W);
       E = W;
       if (E==0)
	  E=NEXT_TO_NOTHING;
       for (N=1; N<=3; N++) {
          V[1][M] = 0;
	  V[2][M] = 0;
	  V[3][M] = 0;
	  V[N][M] = 1;
	  determinant_subr(&W);
	  Q[M][N] = W/E;
	 }
      }
   }     /* if */

}     /* init_arrays */






/* This function is the determinant subroutine function from line #650 in
   Sky and Telescope's program. */

void determinant_subr(double *Wptr)
{

 *Wptr = V[1][1] * V[2][2] * V[3][3] + V[1][2] * V[2][3] * V[3][1] +
	 V[1][3] * V[3][2] * V[2][1] -
	 V[1][3] * V[2][2] * V[3][1] - V[1][1] * V[3][2] * V[2][3] -
	 V[1][2] * V[2][1] * V[3][3];

}     /* determinant_subr */






/* This function is the subroutine from line #750 in Sky and Telescope's
   program. */

void subr_750(double F, double H, double Z1, double Z2)
{

 Y[1][0]=cos(F)*cos(H)-sin(F)*(Z2/RADIAN)+sin(F)*cos(H)*(Z1/RADIAN);
 Y[2][0]=sin(F)*cos(H)+cos(F)*(Z2/RADIAN)-cos(F)*sin(H)*(Z1/RADIAN);
 Y[3][0]=sin(H);

}     /* subr_750 */






/* This function converts the current equatorial coordinates to scope
   coordinates. */

void get_equat_to_scope(struct pos_struct *posptr, double Z1, double Z2,
			double Z3, int *run_real_time_flag_ptr)
{
 char num_str[MAX_DIGITS+3];
 int left   = MAIN_WINDOW_LEFT+3,
     right  = MAIN_WINDOW_RIGHT-3,
     top    = 6,
     bottom = MAIN_WINDOW_BOTTOM+1,
     col    = 17,
     row    = 2,
     response;                    /* user response */
 struct time_date UT;             /* Universal Time */


 /* make window */

 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 /* write title */

 gotoxy(col,row);
 cprintf("EQUAT TO SCOPE CONVERSION:");

 /* get coordinates */

 if (posptr->coord_year!=0)                 /* offer option of using already
					  stored coordinates */
    use_previous_equat(&response);
 else
    response=NO;

 if (response==NO) {
    display_bright_stars();            /* don't highlight selection yet */
    use_bright_stars(&response);
    if (response==YES)
       get_bright_star_coord(posptr);
    else
       get_coordinates(posptr);
   }     /* if */

 /* get time */

 select_time_option(&response);
 get_timezone(posptr);                 /* get time zone (including daylight
					  savings) */
 if (response==YES) {
    get_system_time_date(posptr);      /* use computer's time & date */
    *run_real_time_flag_ptr=YES;       /* set run in real time flag = YES */
   }
 else {
    input_time(posptr);                /* enter new time & date */
    *run_real_time_flag_ptr=NO;        /* set run in real time flag = NO */
   }

 /* calc times */

 LT_UT(&UT, posptr);
 Julian(&UT, posptr);
 sidereal_time(&UT, posptr);

 /* calc scope coordinates */

 calc_scope(posptr, Z1, Z2, Z3);

 /* restore screen */

 window(left, top, right, bottom);
 clrscr();

}     /* get_equat_to_scope */






/* This function converts the current scope coordinates to equatorial
   coordinates. */

void get_scope_to_equat(struct pos_struct *posptr, double Z1, double Z2,
			double Z3, int *run_real_time_flag_ptr)
{
 char num_str[MAX_DIGITS+3];
 int left   = MAIN_WINDOW_LEFT+3,
     right  = MAIN_WINDOW_RIGHT-3,
     top    = 6,
     bottom = MAIN_WINDOW_BOTTOM+1,
     col    = 17,
     row    = 2,
     response;                    /* user response */
 struct time_date UT;             /* Universal Time */


 /* make window */

 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 /* write title */

 gotoxy(col,row);
 cprintf("SCOPE TO EQUAT CONVERSION:");

 /* get scope coordinates */

 if (posptr->coord_year!=0)            /* offer option of using already
					  stored coordinates */
    use_previous_scope(&response);
 else
    response=NO;

 if (response==NO)
    get_scope(posptr);

 /* get time */

 select_time_option(&response);
 get_timezone(posptr);                 /* get time zone (including daylight
					  savings) */
 if (response==YES) {
    get_system_time_date(posptr);      /* use computer's time & date */
    *run_real_time_flag_ptr=YES;       /* set run in real time flag = YES */
   }
 else {
    input_time(posptr);                /* enter new time & date */
    *run_real_time_flag_ptr=NO;        /* set run in real time flag = NO */
   }

 /* calc times */

 LT_UT(&UT, posptr);
 Julian(&UT, posptr);
 sidereal_time(&UT, posptr);

 /* calc equat coordinates */

 calc_equat(posptr, Z1, Z2, Z3);

 /* set object's name to 'unknown' */
 strcpy(posptr->name, "unknown");

 /* restore screen */

 window(left, top, right, bottom);
 clrscr();

}     /* get_scope_to_equat */






/* This function converts equatorial to scope coordinates. */

void calc_scope(struct pos_struct *posptr, double Z1, double Z2, double Z3)
{
 int I, J;                   /* counters */
 double D,                   /* declination/RADIAN */
	B,                   /* (RA - sidereal time in degrees) / RADIAN */
	F,                   /* horizon (meas. CCW) / RADIAN */
	H;                   /* elevation/RADIAN */



 D = posptr->dec_2000 / RADIAN;
 B = (posptr->ra_2000_deg - 15*posptr->sid_time) / RADIAN;

 X[1][1] = cos(D)*cos(B);
 X[2][1] = cos(D)*sin(B);
 X[3][1] = sin(D);

 Y[1][1] = 0;
 Y[2][1] = 0;
 Y[3][1] = 0;

 for (I=1; I<=3; I++)
    for (J=1; J<=3; J++)
       Y[I][1] += ( R[I][J] * X[J][1] );

 angle_subr(&F, &H);

 F/=RADIAN;
 H/=RADIAN;

 subr_785(F, H, Z1, Z2);
 angle_subr(&F, &H);

 H-=Z3;

 /* put values in coordinate structure */

 posptr->horiz = F;
 posptr->elev  = H;

}     /* calc_scope */






/* This function is the angle subroutine from the Sky and Telescope program
   line #685. */

void angle_subr(double *Fptr, double *Hptr)
{
 double C;                        /* intermediate calculated result */


 C = sqrt( pow(Y[1][1], 2) + pow(Y[2][1], 2) );

 if (C==0) {
    if (Y[3][1]>0)
       *Hptr=90;
    else if (Y[3][1]<0)
       *Hptr=-90;
   }
 else
    *Hptr=atan( Y[3][1]/C ) * RADIAN;

 if (C==0)
    *Fptr=1000;
 if (C!=0 && Y[1][1]==0)
    if(Y[2][1]>0)
       *Fptr=90;
    else if (Y[2][1]<0)
       *Fptr=270;
 if (Y[1][1]>0)
    *Fptr=atan( Y[2][1] / Y[1][1] ) * RADIAN;
 else if (Y[1][1]<0)
    *Fptr=atan( Y[2][1] / Y[1][1] ) * RADIAN + 180;

 while (*Fptr>360)
    *Fptr-=360;
 while (*Fptr<0)
    *Fptr+=360;

}     /* angle_subr */






/* This function is the subroutine from the Sky and Telescope program line
   #785. */

void subr_785(double F, double H, double Z1, double Z2)
{

 Y[1][1]=cos(F)*cos(H)+sin(F)*(Z2/RADIAN)-sin(F)*cos(H)*(Z1/RADIAN);
 Y[2][1]=sin(F)*cos(H)-cos(F)*(Z2/RADIAN)+cos(F)*sin(H)*(Z1/RADIAN);
 Y[3][1]=sin(H);

}     /* subr_785 */






/* This function calculates the equatorial coordinates given scope
   coordinates. */

void calc_equat(struct pos_struct *posptr, double Z1, double Z2, double Z3)
{
 int I, J;                   /* counters */
 double F,                   /* horizon (meas. CCW) / RADIAN   and
				RA in deg */
	H;                   /* elevation/RADIAN   and
				dec in deg */


 F = posptr->horiz  / RADIAN;
 H = (posptr->elev+Z3) / RADIAN;

 subr_750(F, H, Z1, Z2);

 X[1][1] = Y[1][0];
 X[2][1] = Y[2][0];
 X[3][1] = Y[3][0];

 Y[1][1] = 0;
 Y[2][1] = 0;
 Y[3][1] = 0;

 for (I=1; I<=3; I++)
    for (J=1; J<=3; J++)
       Y[I][1] += ( Q[I][J] * X[J][1] );

 angle_subr(&F, &H);

 F += 15 * posptr->sid_time;

 while (F>360)
    F-=360;
 while (F<0)
    F+=360;

 posptr->ra_2000_deg = F;
 posptr->dec_2000 = H;

 posptr->coord_year = BASE_YEAR;

 /* break apart RA and dec for display purposes */

 decode_RA_dec(posptr);

}     /* calc_equat */






/* This function decodes RA and declination into their components. */

void decode_RA_dec(struct pos_struct *posptr)
{

 posptr->ra_hr  = (int) posptr->ra_2000_deg / 15;
 posptr->ra_min = (int) ((posptr->ra_2000_deg/15 - posptr->ra_hr) * 60);
 posptr->ra_sec = (posptr->ra_2000_deg/15 - posptr->ra_hr -
		   posptr->ra_min/60) * 3600;

 posptr->dec_deg = (int) posptr->dec_2000;
 posptr->dec_min = (posptr->dec_2000 - posptr->dec_deg) * 60;

}     /* decode_RA_dec */






/* This function asks if the previous equatorial coordinates should be
   used. */

void use_previous_equat(int *responseptr)
{
 int left   = 20,
     right  = left+40,
     top    = 9,
     bottom = top+6,
     col,
     row;


 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 gotoxy(col=3,row=3); cprintf("Use existing equatorial coordinates ?");

 *responseptr=YES;                /* set menu option to YES to start with */
 write_yes_no_choice(col+=12, row=5, responseptr);
 get_yes_no_response(col, row, responseptr);

}     /* use_previous_equat */






/* This function asks if the previously stored telescope coordinates should
   be used instead of inputing new coordinates. */

void use_previous_scope(int *responseptr)
{
 int left   = 20,
     right  = left+40,
     top    = 9,
     bottom = top+6,
     col,
     row;


 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);

 gotoxy(col=3,row=3); cprintf("Use existing telescope coordinates ?");

 *responseptr=YES;                /* set menu option to YES to start with */
 write_yes_no_choice(col+=12, row=5, responseptr);
 get_yes_no_response(col, row, responseptr);

}     /* use_previous_scope */






/* This function continuously updates the scope coordinates in real time. */

void run_equat_to_scope_real_time(struct pos_struct *posptr, double Z1,
				  double Z2, double Z3)
{
 char screen_buffer[25*80*2];      /* buffer to hold screen */
 int left   = 18,
     right  = left+44,
     top    = 4,
     bottom = top+2;
 struct time_date UT;


 /* make return to main menu message window */
 gettext(left, top, right, bottom, screen_buffer);
 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);
 gotoxy(4,2);
 blink_message();
 cprintf("Press any key to return to main menu...");
 base_text_attr();
 window(MAIN_WINDOW_LEFT, MAIN_WINDOW_TOP, MAIN_WINDOW_RIGHT,
	MAIN_WINDOW_BOTTOM);

 while (kbhit()==0) {
    get_system_time_date(posptr);      /* get current time */
    LT_UT(&UT, posptr);                /* calc times */
    Julian(&UT, posptr);
    sidereal_time(&UT, posptr);
    calc_scope(posptr, Z1, Z2, Z3);    /* calc equat coordinates */
    display_coord(CURRENT, posptr);    /* display coordinates */
   }     /* while */

 while(kbhit()!=0)                /* get rid of any remaining keystrokes */
    getch();

 window(left, top, right, bottom);
 puttext(left, top, right, bottom, screen_buffer);
 window(MAIN_WINDOW_LEFT, MAIN_WINDOW_TOP, MAIN_WINDOW_RIGHT,
	MAIN_WINDOW_BOTTOM);

}     /* run_equat_to_scope_real_time */






/* This function continuously updates the equatorial coordinates in real
   time. */


void run_scope_to_equat_real_time(struct pos_struct *posptr, double Z1,
				  double Z2, double Z3)
{
 char screen_buffer[25*80*2];      /* buffer to hold screen */
 int left   = 18,
     right  = left+44,
     top    = 4,
     bottom = top+2;
 struct time_date UT;


 /* make return to main menu message window */
 gettext(left, top, right, bottom, screen_buffer);
 window(left, top, right, bottom);
 clrscr();
 draw_border(left, top, right, bottom);
 gotoxy(4,2);
 blink_message();
 cprintf("Press any key to return to main menu...");
 base_text_attr();
 window(MAIN_WINDOW_LEFT, MAIN_WINDOW_TOP, MAIN_WINDOW_RIGHT,
	MAIN_WINDOW_BOTTOM);

 while (kbhit()==0) {
    get_system_time_date(posptr);      /* get current time */
    LT_UT(&UT, posptr);                /* calc times */
    Julian(&UT, posptr);
    sidereal_time(&UT, posptr);
    calc_equat(posptr, Z1, Z2, Z3);    /* calc equat coordinates */
    display_coord(CURRENT, posptr);    /* display coordinates */
   }     /* while */

 while(kbhit()!=0)                /* get rid of any remaining keystrokes */
    getch();

 window(left, top, right, bottom);
 puttext(left, top, right, bottom, screen_buffer);
 window(MAIN_WINDOW_LEFT, MAIN_WINDOW_TOP, MAIN_WINDOW_RIGHT,
	MAIN_WINDOW_BOTTOM);

}     /* run_scope_to_equat_real_time */






/* This function blinks the return to main menu message. */


void blink_message(void)
{
 textattr(BLUE*16 + WHITE + BLINK);
}     /* blink_message */






/* This function highlights the current potential menu selection. */


void highlight_selection(void)
{
 textattr(WHITE*16 + BLUE - BLINK);
}     /* highlight_selection */






/* This function returns the text to the base attribute settings. */


void base_text_attr(void)
{
 textattr(BLUE*16 + WHITE);
}     /* base_text_attr */






/*****     END OF SOURCE CODE     *****/
