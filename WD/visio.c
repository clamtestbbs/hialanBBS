/*-------------------------------------------------------*/
/* visio.c           ( AT-BBS/WD_hialan BBS )            */
/*-------------------------------------------------------*/
/* target : VIrtual Screen Input Output routines         */
/*	    term.c + screen.c + io.c			 */
/* �X  �� : hialan					 */
/*-------------------------------------------------------*/
#include "bbs.h"

/* ----------------------------------------------------- */
/* output routines  (io.c)                               */
/* ----------------------------------------------------- */
#ifdef  LINUX
#define OBUFSIZE  (2048)
#define IBUFSIZE  (128)
#else
#define OBUFSIZE  (4096)
#define IBUFSIZE  (512)
#endif

static char outbuf[OBUFSIZE];
static int obufsize = 0;

void oflush()
{
  if (obufsize)
  {
     write(1, outbuf, obufsize);
     obufsize = 0;
  }
}

static void output(char *s, int len)
{
  /* Invalid if len >= OBUFSIZE */
  register int size;
  register char *data;

  size = obufsize;
  data = outbuf;

  if (size + len > OBUFSIZE)
  {
    write(0, data, size);
    size = len;
  }
  else
  {
    data += size;
    size += len;
  }
  memcpy(data, s, len);
  obufsize = size;
}

void ochar(char c)
{
  if (obufsize > OBUFSIZE - 1)
  {
    write(1, outbuf, obufsize);
    obufsize = 0;
  }
  outbuf[obufsize++] = c;
}

/*-------------------------------------------------------*/
/* term.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : termcap I/O control routines                 */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#include <sys/ioctl.h>

#ifdef HP_UX
#define O_HUPCL 01
#define O_XTABS 02
#endif

#ifdef LINUX
#include <termios.h>

#define stty(fd, data) tcsetattr( fd, TCSETS, data )
#define gtty(fd, data) tcgetattr( fd, data )

struct termios tty_state, tty_new;
#else
struct sgttyb tty_state, tty_new;
#endif

#ifndef TANDEM
#define TANDEM  0x00000001
#endif

#ifndef CBREAK
#define CBREAK  0x00000002
#endif

/* ----------------------------------------------------- */
/* basic tty control                                     */
/* ----------------------------------------------------- */
void init_tty()
{
  if (gtty(1, &tty_state) < 0)
  {
    fprintf(stderr, "gtty failed\n");
    exit(-1);
  }
  memcpy(&tty_new, &tty_state, sizeof(tty_new));

#ifdef  LINUX

  tty_new.c_lflag &= ~(ICANON | ECHO | ISIG);
  tcsetattr(1, TCSANOW, &tty_new);
  restore_tty();

#else

  tty_new.sg_flags |= RAW;

#ifdef  HP_UX
  tty_new.sg_flags &= ~(O_HUPCL | O_XTABS | LCASE | ECHO | CRMOD);
#else
  tty_new.sg_flags &= ~(TANDEM | CBREAK | LCASE | ECHO | CRMOD);
#endif

  stty(1, &tty_new);
#endif
}


#ifdef LINUX
reset_tty()
{
   system("stty -raw echo");
}
restore_tty()
{
   system("stty raw -echo");
}
#else
void reset_tty()
{
  stty(1, &tty_state);
}
void restore_tty()
{
  stty(1, &tty_new);
}

#endif



/* ----------------------------------------------------- */
/* init tty control code                                 */
/* ----------------------------------------------------- */
#define TERMCOMSIZE (40)

int dumb_term = YEA;

int t_lines = 24;
int b_lines = 23;
int p_lines = 20;
int t_columns = 80;

static int automargins;

char *outp;
int *outlp;

void do_move(int destcol, int destline)
{
  char buf[16], *p;

  sprintf(buf, "\33[%d;%dH", destline + 1, destcol + 1);
  for(p = buf; *p; p++)
    ochar(*p);
}

void save_cursor()
{
  ochar('\33');
  ochar('7');
}

void restore_cursor()
{
  ochar('\33');
  ochar('8');
}

/*-------------------------------------------------------*/
/* screen.c	( NTHU CS MapleBBS Ver 2.36 )		 */
/*-------------------------------------------------------*/
/* target : ANSI/Chinese screen display routines 	 */
/* create : 95/03/29				 	 */
/* update : 95/12/15				 	 */
/*-------------------------------------------------------*/
#include <varargs.h>

#define o_ansi(x)       output(x, sizeof(x)-1)
#define o_clear()       o_ansi("\033[;H\033[2J")
#define o_cleol()       o_ansi("\033[K")
#define o_standup()     o_ansi("\033[7m")
#define o_standdown()   o_ansi("\033[m")
#define o_scrollrev()   o_ansi("\033[L")

uschar scr_lns, scr_cols, cur_ln = 0, cur_col = 0;
uschar docls, standing = NA;
char roll = 0;
int scrollcnt, tc_col, tc_line;

screenline *big_picture = NULL;

void initscr()
{
  extern void *calloc();

  dumb_term = NA;
  scr_lns = t_lines;
  scr_cols = t_columns = ANSILINELEN;
  big_picture = (screenline *) calloc(scr_lns, sizeof(screenline));
  docls = YEA;
}

void move(int y, int x)
{
  cur_col = x;
  cur_ln = y;
}

void getyx(int *y, int *x)
{
  *y = cur_ln;
  *x = cur_col;
}

static void rel_move(was_col, was_ln, new_col, new_ln)
{

  if (new_ln >= t_lines || new_col >= t_columns)
    return;

  tc_col = new_col;
  tc_line = new_ln;
  if (new_col == 0)
  {
    if (new_ln == was_ln)
    {
      if (was_col)
	ochar('\r');
      return;
    }
    else if (new_ln == was_ln + 1)
    {
      ochar('\n');
      if (was_col)
	ochar('\r');
      return;
    }
  }

  if (new_ln == was_ln)
  {
    if (was_col == new_col)
      return;

    if (new_col == was_col - 1)
    {
      ochar(Ctrl('H'));
      return;
    }
  }
  do_move(new_col, new_ln);
}

static void standoutput(char *buf, int ds, int de, int sso, int eso)
{
  if (eso <= ds || sso >= de)
    output(buf + ds, de - ds);
  else
  {
    int st_start, st_end;

    st_start = BMAX(sso, ds);
    st_end = BMIN(eso, de);
    if (sso > ds)
      output(buf + ds, sso - ds);
    o_standup();
    output(buf + st_start, st_end - st_start);
    o_standdown();
    if (de > eso)
      output(buf + eso, de - eso);
  }
}

void redoscr()
{
  register screenline *bp;
  register int i, j, len;

  if (dumb_term)
    return;

  o_clear();
  for (tc_col = tc_line = i = 0, j = roll; i < scr_lns; i++, j++)
  {
    if (j >= scr_lns)
      j = 0;
    bp = &big_picture[j];
    if (len = bp->len)
    {
      rel_move(tc_col, tc_line, 0, i);

      if (bp->mode & STANDOUT)
	standoutput(bp->data, 0, len, bp->sso, bp->eso);
      else
	output(bp->data, len);
      tc_col += len;
      if (tc_col >= t_columns)
      {
	if (automargins)
	  tc_col = t_columns - 1;
	else
	{
	  tc_col -= t_columns;
	  tc_line++;
	  if (tc_line >= t_lines)
	    tc_line = b_lines;
	}
      }
      bp->mode &= ~(MODIFIED);
      bp->oldlen = len;
    }
  }
  rel_move(tc_col, tc_line, cur_col, cur_ln);
  docls = scrollcnt = 0;
  oflush();
}


void refresh()
{
  register screenline *bp = big_picture;
  register int i, j, len;

  if (dumb_term)
    return;
  if (num_in_buf())
    return;

  if ((docls) || (abs(scrollcnt) >= (scr_lns - 3)))
  {
    redoscr();
    return;
  }

  if (scrollcnt < 0)
  {
    rel_move(tc_col, tc_line, 0, 0);
    do
    {
      o_scrollrev();
    } while (++scrollcnt);
  }
  else if (scrollcnt > 0)
  {
    rel_move(tc_col, tc_line, 0, b_lines);
    do
    {
      ochar('\n');
    } while (--scrollcnt);
  }

  for (i = 0, j = roll; i < scr_lns; i++, j++)
  {
    if (j >= scr_lns)
      j = 0;
    bp = &big_picture[j];
    len = bp->len;
    if (bp->mode & MODIFIED && bp->smod < len)
    {
      bp->mode &= ~(MODIFIED);
      if (bp->emod >= len)
	bp->emod = len - 1;
      rel_move(tc_col, tc_line, bp->smod, i);

      if (bp->mode & STANDOUT)
	standoutput(bp->data, bp->smod, bp->emod + 1, bp->sso, bp->eso);
      else
	output(&bp->data[bp->smod], bp->emod - bp->smod + 1);
      tc_col = bp->emod + 1;
      if (tc_col >= t_columns)
      {
	if (automargins)
	{
	  tc_col -= t_columns;
	  if (++tc_line >= t_lines)
	    tc_line = b_lines;
	}
	else
	  tc_col = t_columns - 1;
      }
    }

    if (bp->oldlen > len)
    {
      rel_move(tc_col, tc_line, len, i);
      o_cleol();
    }
    bp->oldlen = len;
  }
  rel_move(tc_col, tc_line, cur_col, cur_ln);
  oflush();
}

void clear()
{
  int i;
  screenline *slp;

  docls = YEA;
  cur_ln = cur_col = roll = i = 0;
  slp = big_picture;
  while (i++ < t_lines)
    memset(slp++, 0, 9);
}

void clrtobot()
{
  if (!dumb_term)
  {
    register screenline *slp;
    register int i, j;

    for (i = cur_ln, j = i + roll; i < scr_lns; i++, j++)
    {
      if (j >= scr_lns)
	j -= scr_lns;
      slp = &big_picture[j];
      slp->mode = slp->len = 0;
      if (slp->oldlen)
	slp->oldlen = 255;
    }
  }
}

void clrtoeol()
{
  if (!dumb_term)
  {
    register screenline *slp;
    register int ln;

    standing = NA;
    if ((ln = cur_ln + roll) >= scr_lns)
      ln -= scr_lns;
    slp = &big_picture[ln];
    if (cur_col <= slp->sso)
      slp->mode &= ~STANDOUT;

    if (cur_col > slp->oldlen)
    {
      for (ln = slp->len; ln <= cur_col; ln++)
	slp->data[ln] = ' ';
    }

    if (cur_col < slp->oldlen)
    {
      for (ln = slp->len; ln >= cur_col; ln--)
	slp->data[ln] = ' ';
    }

    slp->len = cur_col;
  }
}


void clrchyiuan(int x,int y)
{
  if (!dumb_term)
  {
    register screenline *slp;
    register int i, j;

    for (i = x, j = i + roll; i < y; i++, j++)
    {
      if (j >= scr_lns)
        j -= scr_lns;
      slp = &big_picture[j];
      slp->mode = slp->len = 0;
      if (slp->oldlen)
        slp->oldlen = 255;
    }
  }
}


void outch(register uschar c)
{
  register screenline *slp;
  register int i;

#ifndef BIT8
  c &= 0x7f;
#endif

  if (dumb_term)
  {

#ifdef BIT8
    if ((c != '') && !isprint2(c))
#else
    if (!isprint(c))
#endif

    {
      if (c == '\n')
	ochar('\r');
      else
	c = '*';
    }
    ochar(c);
    return;
  }

  if ((i = cur_ln + roll) >= scr_lns)
    i -= scr_lns;
  slp = &big_picture[i];

#ifdef BIT8
  if ((c != '') && !isprint2(c))
#else
  if (!isprint(c))
#endif

  {
    if (c == '\n' || c == '\r')
    {
      if (standing)
      {
	slp->eso = BMAX(slp->eso, cur_col);
	standing = NA;
      }

#if 1
      if ((i = cur_col - slp->len) > 0)
	memset(&slp->data[slp->len], ' ', i + 1);
#else
      if (cur_col > slp->len)
      {
	for (i = slp->len; i <= cur_col; i++)
	  slp->data[i] = ' ';
      }
#endif

      slp->len = cur_col;
      cur_col = 0;
      if (cur_ln < scr_lns)
	cur_ln++;
      return;
    }
    c = '*';			/* substitute a '*' for non-printable */
  }

  if (cur_col >= slp->len)
  {
    for (i = slp->len; i < cur_col; i++)
      slp->data[i] = ' ';
    slp->data[cur_col] = '\0';
    slp->len = cur_col + 1;
  }

  if (slp->data[cur_col] != c)
  {
    slp->data[cur_col] = c;
    if ((slp->mode & MODIFIED) != MODIFIED)
      slp->smod = slp->emod = cur_col;
    slp->mode |= MODIFIED;
    if (cur_col > slp->emod)
      slp->emod = cur_col;
    if (cur_col < slp->smod)
      slp->smod = cur_col;
  }

  if (++cur_col >= scr_cols)
  {
    if (standing && (slp->mode & STANDOUT))
    {
      standing = 0;
      slp->eso = BMAX(slp->eso, cur_col);
    }
    cur_col = 0;
    if (cur_ln < scr_lns)
      cur_ln++;
  }
}


static void parsecolor(char *buf)
{
  char *val;
  char data[24];

  data[0] = '\0';
  val = (char *) strtok(buf, ";");

  while (val)
  {
    if (atoi(val) < 30)
    {
      if (data[0])
	strcat(data, ";");
      strcat(data, val);
    }
    val = (char *) strtok(NULL, ";");
  }
  strcpy(buf, data);
}


#define NORMAL (00)
#define ESCAPE (01)
#define VTKEYS (02)


void outc(register unsigned char ch)
{
  if (showansi)
    outch(ch);
  else
  {
    static char buf[24];
    static int p = 0;
    static int mode = NORMAL;
    int i;

    switch (mode)
    {
    case NORMAL:
      if (ch == '\033')
	mode = ESCAPE;
      else
	outch(ch);
      return;

    case ESCAPE:
      if (ch == '[')
	mode = VTKEYS;
      else
      {
	mode = NORMAL;
	outch('');
	if (ch != ']')
  	  outch(ch);
      }
      return;

    case VTKEYS:
      if (ch == 'm')
      {
	buf[p++] = '\0';
	parsecolor(buf);
      }
      else if ((p < 24) && (not_alpha(ch)))
      {
	buf[p++] = ch;
	return;
      }

      if (buf[0])
      {
	outch('');
	outch('[');

	for (i = 0; p = buf[i]; i++)
	  outch(p);
	outch(ch);
      }
      p = 0;
      mode = NORMAL;
    }
  }
}


void outs(str)
  register char *str;
{
  while (*str)
    outc(*str++);
}


void
outmsg(msg)
  register char *msg;
{
  move(b_lines, 0);
  clrtoeol();
  while (*msg)
    outc(*msg++);
}

void outz(register char *msg)
{
  outmsg(msg);
  refresh();
  sleep(1);
}


void
prints(va_alist)
va_dcl
{
  va_list args;
  char buff[1024], *fmt;

  va_start(args);
  fmt = va_arg(args, char *);
  vsprintf(buff, fmt, args);
  va_end(args);
  outs(buff);
}


void scroll()
{
  if (dumb_term)
    outc('\n');
  else
  {
    scrollcnt++;
    if (++roll >= scr_lns)
      roll = 0;
    move(b_lines, 0);
    clrtoeol();
  }
}


void rscroll()
{
  if (dumb_term)
    outs("\n\n");
  else
  {
    scrollcnt--;
    if (--roll < 0)
      roll = b_lines;
    move(0, 0);
    clrtoeol();
  }
}


/* 		SO/do_talk.c		    */

static change_scroll_range(int top, int bottom)
{ 
  char buf[TERMCOMSIZE];
  
  sprintf(buf, "\033[%d;%dr", top+1, bottom+1);
  output(buf, strlen(buf));
}

static scroll_forward()
{
  output("\033D", 2);
}

/*
static scroll_reverse()
{
  output("\033M", 2);
}
*/

void region_scroll_up(int top, int bottom)
{
   int i;

   if (top > bottom) 
   {
      i = top; 
      top = bottom;
      bottom = i;
   }

   if (top < 0 || bottom >= scr_lns)
     return;

   for (i = top; i < bottom; i++)
      big_picture[i] = big_picture[i + 1];
   memset(big_picture + i, 0, sizeof(*big_picture));
   memset(big_picture[i].data, ' ', scr_cols);
   save_cursor();
   change_scroll_range(top, bottom);
   do_move(0, bottom);
   scroll_forward();
   change_scroll_range(0, scr_lns - 1);
   restore_cursor();
   refresh();
}

/*
region_scroll_down(int top, int bottom)
{
}
*/

/*    		end do_talk.c		*/

void standout()
{
  if (!standing && !dumb_term /*&& strtstandoutlen*/)
  {
    register screenline *slp;

    slp = &big_picture[((cur_ln + roll) % scr_lns)];
    standing = YEA;
    slp->sso = slp->eso = cur_col;
    slp->mode |= STANDOUT;
  }
}


void standend()
{
  if (standing && !dumb_term /*&& strtstandoutlen*/)
  {
    register screenline *slp;

    slp = &big_picture[((cur_ln + roll) % scr_lns)];
    standing = NA;
    slp->eso = BMAX(slp->eso, cur_col);
  }
}

#define VS_STACK_SIZE 5                                        
int vs_stack_ptr = -1;          /* CityLion */                 
int old_roll[VS_STACK_SIZE];                                   
int my_newfd[VS_STACK_SIZE],x[VS_STACK_SIZE],y[VS_STACK_SIZE]; 
int i_newfd = 0;
int mode0[VS_STACK_SIZE],stat0[VS_STACK_SIZE];                 
                                                               
void vs_save(screenline *screen)                                                
{                                                              
  vs_stack_ptr++;                                              
  old_roll[vs_stack_ptr] = roll;
  
  if(currutmp)
  {
    mode0[vs_stack_ptr] = currutmp->mode;                        
    stat0[vs_stack_ptr] = currstat;                              
  }
  getyx(&y[vs_stack_ptr],&x[vs_stack_ptr]);                    
  memcpy(screen, big_picture, t_lines * sizeof(screenline));   
  my_newfd[vs_stack_ptr] = i_newfd;                            
  i_newfd = 0;                                              
}                                                           
                                                            
void vs_restore(screenline *screen)
{                                                           
  roll = old_roll[vs_stack_ptr];                            
  i_newfd = my_newfd[vs_stack_ptr];
  if(currutmp)
  {
    currstat = stat0[vs_stack_ptr];
    currutmp->mode = mode0[vs_stack_ptr];
  }
  memcpy(big_picture, screen, t_lines * sizeof(screenline));
  move(y[vs_stack_ptr],x[vs_stack_ptr]);                    
  vs_stack_ptr--;                                           
  free(screen);                                             
  redoscr();                                                
  refresh();                                                
}                                                           

void capture_screen()
{
   char fname[PATHLEN];
   FILE* fp;
   extern screenline *big_picture;
   extern uschar scr_lns;
   int i;

   sethomefile(fname, cuser.userid, "buf.0");
   if (fp = fopen(fname, "w")) 
   {
      for (i = 0; i < scr_lns; i++)
         fprintf(fp, "%.*s\n", big_picture[i].len, big_picture[i].data);
      fclose(fp);
   }
}

/*-------------------------------------------------------*/
/* io.c         ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : basic console/screen/keyboard I/O routines   */
/* create : 95/02/28                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/
#ifdef AIX
#include <sys/select.h>
#endif

void passwd_outs(char *text)
{
  register int column = 0;
  register char ch;
  while ((ch = *text++) && (++column < 80))
  {
    outch('*');
  }
}

/* ----------------------------------------------------- */
/* �w����ܰʺA�ݪO                                      */
/* ----------------------------------------------------- */
#define STAY_TIMEOUT    (30*60)

static void hit_alarm_clock()
{
  static int stay_time = 0;
  static int idle_time = 0;

  time_t now = time(0);
  char buf[100]="\0";

  if(currutmp->pid != currpid)
    setup_utmp(XMODE);   /* ���s�t�m shm */

  if((idle_time = now - currutmp->lastact) > IDLE_TIMEOUT && !HAS_PERM(PERM_RESEARCH))
  {
    pressanykey("�W�L���m�ɶ��I��X�h�o�K�K");
    abort_bbs();
  }

  if (HAS_HABIT(HABIT_MOVIE) && (currstat && (currstat < CLASS || currstat == MAILALL)))
    movie(0);

  alarm(MOVIE_INT);
  stay_time += MOVIE_INT;

  if(idle_time > IDLE_TIMEOUT - 60 && !HAS_PERM(PERM_RESEARCH)) 
    sprintf(buf, "[1;5;37;41mĵ�i�G�z�w���m�L�[�A�Y�L�^���A�t�ΧY�N�����I�I[m");
  else if(stay_time > 10 * 60 && chkmail(0)) 
  {
    sprintf(buf, "\033[1;33;41m[%s] �H�c���٦��S�ݹL���H��\033[m",
      Etime(&now));
    stay_time = 0 ;
  }
  else if(stay_time > STAY_TIMEOUT && HAS_HABIT(HABIT_ALARM))
  {
    /* �b�o�̴��� user �𮧤@�U */
    char *msg[10] = {
    "�����y, �|�|��, �ܤf��....�ݤf��...�A�~��...!",
    "�@��O�ͱ� �@��O�R�� ���k���G�Ƭ����ۦۤv...",
    "�O�_���H�A�ѱz���ߪ��t�I? �j�a��talk talk�a.. ",
    "���T��,�k�T��,��l��᧾�ѧ�� �j�a�ӧ@�B�ʭ�~",
    "����..����..��������! ���w���z��...",
    "�ΡE�\\�E��E��",
    "�ѩ����F�S��....^.^",
    "���Ѧ��S���Ҹհ�...���ѭ��n��...!",
    "�ݡ�y�P�I",
    "�E�Ѧb�ߤ���۬ӡEŪ�ѥh�E"};
    int i = rand() % 10;

    sprintf(buf, "[1;33;41m[%s] %s[m", Etime(&now), msg[i]);
    stay_time = 0 ;
  }

  if(buf[0]) 
  {
    outmsg(buf);
    refresh();
    bell();
  }
}

void init_alarm()
{
  alarm(0);
  signal(SIGALRM, hit_alarm_clock);
  alarm(MOVIE_INT);
}

/* ----------------------------------------------------- */
/* input routines                                        */
/* ----------------------------------------------------- */
static struct timeval i_to, *i_top = NULL;

#define INPUT_ACTIVE    0
#define INPUT_IDLE      1
static int i_mode = INPUT_ACTIVE;

static char inbuf[IBUFSIZE];
static int ibufsize = 0;
static int icurrchar = 0;

void add_io(int fd, int timeout)
{
  i_newfd = fd;
  if (timeout)
  {
    i_to.tv_sec = timeout;
    i_to.tv_usec = 0;
    i_top = &i_to;
  }
  else
    i_top = NULL;
}

int num_in_buf()
{
  return icurrchar - ibufsize;
}

static int dogetch()
{
  int ch;

  if(currutmp) time(&currutmp->lastact);

  for (;;)
  {
    if (ibufsize == icurrchar)
    {
      fd_set readfds;
      struct timeval to;

      to.tv_sec = to.tv_usec = 0;
      FD_ZERO(&readfds);
      FD_SET(0, &readfds);
      if (i_newfd)
        FD_SET(i_newfd, &readfds);
      if ((ch = select(FD_SETSIZE, &readfds, NULL, NULL, &to)) <= 0)
      {
        if (dumb_term)
          oflush();
        else
          refresh();

        FD_ZERO(&readfds);
        FD_SET(0, &readfds);
        if (i_newfd)
          FD_SET(i_newfd, &readfds);

        while ((ch = select(FD_SETSIZE, &readfds, NULL, NULL, i_top)) < 0)
        {
          if (errno == EINTR)
            continue;
          else
          {
            perror("select");
            return -1;
          }
        }
        if (ch == 0)
          return I_TIMEOUT;
      }
      if (i_newfd && FD_ISSET(i_newfd, &readfds))
        return I_OTHERDATA;

      while ((ibufsize = read(0, inbuf, IBUFSIZE)) <= 0)
      {
        if (ibufsize == 0)
          longjmp(byebye, -1);
        if (ibufsize < 0 && errno != EINTR)
          longjmp(byebye, -1);
      }
      icurrchar = 0;
    }

    i_mode = INPUT_ACTIVE;
    
    ch = inbuf[icurrchar++];
    return (ch);
  }
}

extern char oldmsg_count;            /* pointer */    
extern char watermode;

int igetch()
{
    register int ch;
    while(ch = dogetch())
    {
     switch (ch)
      {
       case Ctrl('L'):
         redoscr();
         continue;
       case Ctrl('I'):
         if(currutmp != NULL && currutmp->mode == MMENU)
         {
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
           vs_save(screen);
           t_idle();
           vs_restore(screen);
           continue;
         }
         else return(ch);

       case Ctrl('Q'):
         if(currutmp != NULL && currutmp->mode)
         {
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
           vs_save(screen);
           DL_func("SO/dreye.so:main_dreye");
           vs_restore(screen);
           continue;
         }

       case Ctrl('W'):  // wildcat : �ֳt���� :p
         if(currutmp->mode && currutmp->mode != READING)
         {
           if(answer("�T�w�n����?? (y/N)") != 'y')
             return(ch);
           update_data();
           u_exit("ABORT");
           pressanykey("���¥��{, �O�o�`�ӳ� !");
           exit(0);
         }
         else return (ch);

#if 0
       case Ctrl('Z'):   /* wildcat:help everywhere */
       {
         static short re_entry = 0; /* CityLion: �����J��... */
         if(currutmp && !re_entry && currutmp->mode != IDLE)
         {
           int mode0 = currutmp->mode;
           int stat0 = currstat;
           int more0 = inmore;
           int i;
           int old_roll = roll;
           int my_newfd = i_newfd;
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));

           re_entry = 1;
           vs_save(screen);
           i = show_help(currutmp->mode);

           currutmp->mode = mode0;
           currstat = stat0;
           inmore = more0;
           roll = old_roll;
           i_newfd = my_newfd;
           vs_restore(screen);
           re_entry = 0;
           continue;
         }
         else return (ch);
       }
#endif
       case Ctrl('Z'):
       {
         static char re_entry = 0;
         screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));

         if(re_entry)
         {
           pressanykey("�z�w�g�ϥΧֳt���F!");
           return ch;
         }         
         
         if(currutmp && currutmp->mode != IDLE)
         {
           int mode0 = currutmp->mode;
           int stat0 = currstat;
           int more0 = inmore;
           int old_roll = roll;
           int my_newfd = i_newfd;
           char *choose[12]={"mM)�H��C��", 
           		    "fF)�ڪ��̷R", 
           		    "uU)�u�W�W��",
           		    "bB)�Ҧ��ݪO",
           		    "wW)�^�~�r��",
           		    "pP)�C�ֳ]�I",
           		    "vV)�벼����",
           		    "kK)�d�ߺ���",
           		    "nN)��d���O",
           		    "sS)�I�q����",
           		    "gG)�U����",
           		    msg_choose_cancel};
	   char ans='m';
	   
           if(currutmp->mode == EDITING || !cuser.userlevel)
             return ch;
         
           re_entry = 1;
           vs_save(screen);         
 	   
 	   while(ans != 'q')
 	   {
  	     clear();
  	     setutmpmode(QMENU);
             switch(ans = win_select("�ֳt���", "", choose, 12, ans))
             {
               case 'm':
                 m_read();
                 break;
               case 'f':
                 Favor();
                 break;
               case 'u':
                 if(currutmp->mode != LUSERS)
                   t_users();
                 else
                   pressanykey("�z�w�g�b�ϥΪ̦W��F!!");
                 break;
               case 'b':
                 New();
                 break;
               case 'w':
                 DL_func("SO/dreye.so:main_dreye");
                 break;
               case 'p':
                 DL_func("SO/soman.so:soman");
                 break;
               case 'v':
                 DL_func("SO/vote.so:all_vote");
                 break;
               case 'k':
                 t_query();
                 break; 
               case 's':
                 DL_func("SO/song.so:ordersong");
                 break;
               case 'n':
                 note();
                 break;
               case 'g':
                 Goodbye();
                 break;
             }
           }

           vs_restore(screen);
           re_entry = 0;
           currutmp->mode = mode0;
           currstat = stat0;
           inmore = more0;
           roll = old_roll;
           i_newfd = my_newfd;
           continue;
         }
         return ch;
       }
       break;

       case Ctrl('U'):
         resetutmpent();
         if(currutmp != NULL && currutmp->mode != EDITING &&
            currutmp->mode != LUSERS && currutmp->mode)
         {
           int mode0 = currutmp->mode;
           int stat0 = currstat;
           screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));

           vs_save(screen);
           t_users();
           vs_restore(screen);

           currutmp->mode = mode0;
           currstat = stat0;

           continue;
         }
         else return (ch);

        case Ctrl('R'):
        {
          if(currutmp == NULL) return (ch);
          else if(watermode > 0)
          {
            watermode = (watermode + oldmsg_count)% oldmsg_count + 1;
            t_display_new(0);
            continue;
          }
          else if (!currutmp->mode && (currutmp->chatid[0] == 2 ||
               currutmp->chatid[0] == 3) && oldmsg_count && !watermode)
          {
            watermode=1;
            t_display_new(1);
            continue;
          }
          else if (currutmp->msgs[0].last_pid && currutmp->mode)
          {
            screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));
            vs_save(screen);
            watermode=1;
            t_display_new(1);
            my_write(currutmp->msgs[0].last_pid, "���y��^�h�G");
            vs_restore(screen);
            continue;
          }
          else return (ch);
        }

        case '\n':   /* Ptt�� \n���� */
           continue;
        case Ctrl('T'):
          if(watermode > 0 )
          {
            watermode = (watermode + oldmsg_count - 2 )% oldmsg_count + 1;
            t_display_new(0);
            continue;
          }

        default:
          return (ch);
       }
    }
}

int offset_count(char *prompt)   // Robert Liu 20010813
{
  int i=0, j=0, off=0;
  for(i=0 ; i<strlen(prompt) ; i++)
  {
    if(prompt[i]==27) off=1;
    if(off==0) j++;
    if(prompt[i]=='m' && off==1) off=0;
  }
  return (strlen(prompt)-j);
}

int check_ctrlword(char *buf,int len)
{
  int i;
  
  for(i = 0;i < len - 1;i++)
  {
    if(buf[i]=='\033')
      return 1;
    if(buf[i]=='\0')
      return 0;
  }
  return 0;
}

int 
getdata(int line, int col, char *prompt, char *buf, int len, int echo, char *ans)
{
  /*int ctrlword = 0;*/  /*�w���o�ͱ��p,�ҥH�d��...*/
		         /*�p�G���H�i�H�}��,�A��L���}*/
		         /*�s�P���v�������a��]���}  by hialan*/
  register int ch;
  int clen;
  int x, y;
  int off_set = 0; /*add color*/
  extern unsigned char scr_cols;
#define MAXLASTCMD 6
  static char lastcmd[MAXLASTCMD][80];

  if (prompt)
  {
    move(line, col);
    clrtoeol();
    outs(prompt);
    off_set=offset_count(prompt); /*add color*/
  }
  else
    clrtoeol();

  if (dumb_term || !echo || /* echo == PASS || */echo == 9)
  {                     /* shakalaca.990422: ���F��J passwd �ɦ��ϥ� */
    len--;              /* �U���o�q�{���X�O�S���ϥ� (!echo) */
    clen = 0;
    while ((ch = igetch()) != '\r')
    {
      if (ch == '\n')
        break;
      if (ch == '\177' || ch == Ctrl('H'))
      {
        if (!clen)
        {
          bell();
          continue;
        }
        clen--;
        if (echo)
        {
          ochar(Ctrl('H'));
          ochar(' ');
          ochar(Ctrl('H'));
        }
        continue;
      }

#ifdef BIT8
      if (!isprint2(ch))
#else
      if (!isprint(ch))
#endif

      {
        if (echo)
          bell();
        continue;
      }
      if (clen >= len)
      {
        if (echo)
          bell();
        continue;
      }
      buf[clen++] = ch;
      if (echo && echo != 9)
        ochar(/* echo == PASS ? '-' : */ch); /* shakalaca.990422: ���F passwd */
    }
    buf[clen] = '\0';
    outc('\n');
    oflush();
  }
  else
  {
   int cmdpos = MAXLASTCMD -1;
   int currchar = 0;
   int keydown;
   int dirty;

    getyx(&y, &x);
    x=x-off_set;  /*add color by hialan*/
    standout();
    for (clen = len--; clen; clen--)
      outc(' ');
    standend();

    if (ans && check_ctrlword(ans,strlen(ans)) != 1) {
       int i;

       strncpy(buf, ans, len);
       buf[len] = 0;
       for (i = strlen(buf) + 1; i < len; i++)
          buf[i] = 0;
       
       move(y, x);
       edit_outs(buf);
       clen = currchar = strlen(buf);

    }
    else
       memset(buf, 0, len);

    dirty = 0;
    while (move(y, x + currchar), (ch = igetkey()) != '\r')
    {
       keydown = 0;
       switch (ch) 
       {
       case Ctrl('Y'): 
       {
          int i;

          if (clen && dirty) 
          {
             for (i = MAXLASTCMD - 1; i; i--)
                strcpy(lastcmd[i], lastcmd[i - 1]);
             strncpy(lastcmd[0], buf, len);
          }

          move(y, x);
          for (clen = len--; clen; clen--)
            outc(' ');
          memset(buf, '\0', strlen(buf));
          clen = currchar = strlen(buf);
          continue;
       }

       case KEY_DOWN:
       case Ctrl('N'):
          keydown = 1;
       case Ctrl('P'):
       case KEY_UP: 
       {
          int i;

          if (clen && dirty) 
          {
             for (i = MAXLASTCMD - 1; i; i--)
                strcpy(lastcmd[i], lastcmd[i - 1]);
             strncpy(lastcmd[0], buf, len);
          }

          i = cmdpos;
          do 
          {
             if (keydown)
                --cmdpos;
             else
                ++cmdpos;
             if (cmdpos < 0)
                cmdpos = MAXLASTCMD - 1;
             else if (cmdpos == MAXLASTCMD)
                cmdpos = 0;
          } while (cmdpos != i && (!*lastcmd[cmdpos] || !strncmp(buf, lastcmd[cmdpos], len)));
          if (cmdpos == i)
             continue;

          strncpy(buf, lastcmd[cmdpos], len);
          buf[len] = 0;

          move(y, x);                   /* clrtoeof */
          for (i = 0; i <= clen; i++)
             outc(' ');
          move(y, x);

          if (echo == PASS)
            passwd_outs(buf);
          else
            edit_outs(buf);
          clen = currchar = strlen(buf);
          dirty = 0;
          continue;
       }
       case KEY_ESC:
         if(currutmp)
         {
           if (KEY_ESC_arg == 'c')
              capture_screen();
              
           if (KEY_ESC_arg == 'n')
              edit_note();
         }

         if (ch == 'U' && currstat != IDLE  &&
           !(currutmp->mode == 0 &&
           (currutmp->chatid[0] == 2 || currutmp->chatid[0] == 3)))
            t_users();
         continue;

/* yagami.000504 : ��Хi��̫e�γ̫� */ 
/* wildcat : ���� ctrl-a , ctrl-e �]�O�@�˪��� :p */
       case KEY_HOME:
         currchar = 0;
         break;
       case KEY_END:
         currchar = strlen(buf);
         break;

       case KEY_LEFT:
          if (currchar)
             --currchar;
          continue;
       case KEY_RIGHT:
          if (buf[currchar])
             ++currchar;
          continue;
       }

      if (ch == '\n' || ch == '\r')
         break;

      if (ch == Ctrl('I') && currstat != IDLE &&
          !(currutmp->mode == 0 &&
            (currutmp->chatid[0] == 2 || currutmp->chatid[0] == 3))) 
      {
         t_idle();
         continue;
      }
      if (ch == '\177' || ch == Ctrl('H'))
      {
        if (currchar) 
        {
           int i;

           currchar--;
           clen--;
           for (i = currchar; i <= clen; i++)
              buf[i] = buf[i + 1];
           move(y, x + clen);
           outc(' ');
           move(y, x);
           if (echo == PASS)
             passwd_outs(buf);
           else
             edit_outs(buf);
           dirty = 1;
        }
        continue;
      }
      if (ch == Ctrl('D')) 
      {
        if (buf[currchar]) 
        {
           int i;

           clen--;
           for (i = currchar; i <= clen; i++)
              buf[i] = buf[i + 1];
           move(y, x + clen);
           outc(' ');
           move(y, x);
           if (echo == PASS)
             passwd_outs(buf);
           else
             edit_outs(buf);
           dirty = 1;
        }
        continue;
      }
      if (ch == Ctrl('K')) 
      {
         int i;

         buf[currchar] = 0;
         move(y, x + currchar);
         for (i = currchar; i < clen; i++)
            outc(' ');
         clen = currchar;
         dirty = 1;
         continue;
      }
      if (ch == Ctrl('A')) 
      {
         currchar = 0;
         continue;
      }
      if (ch == Ctrl('E')) 
      {
         currchar = clen;
         continue;
      }


      if (!(isprint2(ch)))
      {
        continue;
      }
      if (clen >= len || x + clen >= scr_cols)
      {
        continue;
      }
/*
woju
*/
      if (buf[currchar]) {               /* insert */
         int i;

         for (i = currchar; buf[i] && i < len && i < 80; i++)
            ;
         buf[i + 1] = 0;
         for (; i > currchar; i--)
            buf[i] = buf[i - 1];
      }
      else                              /* append */
         buf[currchar + 1] = '\0';

      buf[currchar] = ch;
      move(y, x + currchar);
      if (echo == PASS)
        passwd_outs(buf + currchar);
      else
      /* shakalaca.990422: �쥻�u���U������, �o�O���F��J passwd ���ϥ� */
        edit_outs(buf + currchar);
      currchar++;
      clen++;
      dirty = 1;
    }
    buf[clen] = '\0';

//    ctrlword = check_ctrlword(buf,clen);
    
    if (clen > 1 && echo != PASS /*&& ctrlword != 1*/) {
    /* shaklaaca.990514: ^^^^^^^ ������J�� password �d�U���� */
    /* ������J������X�d�U���� ~~ by hialan */
       for (cmdpos = MAXLASTCMD - 1; cmdpos; cmdpos--)
          strcpy(lastcmd[cmdpos], lastcmd[cmdpos - 1]);
       strncpy(lastcmd[0], buf, len);
    }
    if (echo) {
      move(y, x + clen);
      outc('\n');
    }
    refresh();
    

  }
  if ((echo == LCECHO) && ((ch = buf[0]) >= 'A') && (ch <= 'Z'))
    buf[0] = ch | 32;

  return clen;
}


char
getans(prompt)
  char *prompt;
{
  char ans[5];

  getdata(b_lines,0,prompt,ans,4,LCECHO,0);

  return ans[0];
}

int word_bigsmall(int ch)
{
  if(ch >= 'a' && ch <= 'z')
    ch = (ch - 'a') + 'A';
                                                                                
  return ch;
}
                                                                                
/*  getans2 �ǭȸ���  by hialan 2002/4/14
                                                                                
line    --> �ĴX�C
col     --> �ĴX�Ӧr
prompt  --> ����
s       --> �ﶵ�榡
  �Ҧp:
    char *test[2] = {"yY.�O","nN.�_"};
  �Ĥ@�Ӥp�gy�O�ֳt��
  �ĤG�ӥH��Ψ����
many    --> ���X�ӿﶵ
def     --> �w�]��
  �P�W��:
    getans2(b_line,0,"", test, 2, 'y');
                                                                                
*/
                                                                                
char getans2(int line, int col, char *prompt, char **s, int many, char def)
{
  int point = 0;        /*����*/
  int len = strlen(prompt);
  int i;
  int off_set = 0;
  int ch;
  char bar_color[50];
  char buf[128], buf2[128];
  char *p;      /*�T��������*/

  /*�w�]��*/
  off_set = offset_count(prompt);
  if(!s) s = msg_choose;
  
  if(many == 1) return *(s[0]);	/* �u���@�ӿﶵ�擄��? */
  
  for(i = 0;i < many;i++)
  {
    p = s[i];
    if(def == *p)
    {
      point = i;
      break;
    }
  }
  
  /* ��� getdata */                                                                              
  if(!HAVE_HABIT(HABIT_LIGHTBAR))
  {
    char tmp[128],tmp2[2];
                                                                                
    buf2[0] = '\0';
    tmp2[0] = word_bigsmall(*(s[point]));
    tmp2[1] = '\0';
                                                                                
    strcpy(tmp,prompt);
                                                                                
    for(i = 0; i < many; i++)
    {
      p = s[i];
      strcat(tmp, p+1);
      strcat(tmp, " ");
    }
                                                                                
    sprintf(tmp,"%s[%c] ", tmp, tmp2[0]);
    getdata(line, col, tmp, buf2, 4, LCECHO, 0);
                                                                                
    if(*buf2 == '\0')
      return *tmp2;
    else
      return *buf2;
  }
                                                                                
  get_lightbar_color(bar_color);
  /*�M�ſù�*/
  if (prompt)
  {
    move(line, col);
    clrtoeol();
    outs(prompt);
  }
  else
    clrtoeol();
                                                                                
  do
  {
    for(i = 0;i < many;i++)
    {
      p = s[i];
      if(i == point)
        sprintf(buf,"\033[1m[\033[m%s%s\033[0;1m]\033[m",bar_color,p+1);
      else
        sprintf(buf," %s ",p+1);
                                                                                
      if(i == 0)
        sprintf(buf2,"%s",buf);
      else
        strcat(buf2,buf);
    }
    move(line, (col+len) - off_set);
    outs(buf2);
    move(b_lines,0);
                                                                                
    ch = igetkey();
    switch(ch)
    {
      case KEY_TAB:
      case KEY_DOWN:
      case KEY_RIGHT:
        point++;
        if(point >= many)
          point = 0;
        break;
      case KEY_UP:
      case KEY_LEFT:
        point--;
        if(point < 0)
          point = many - 1;
        break;
                                                                                
      default:
        ch = word_bigsmall(ch);
        for(i = 0;i < many;i++)
        {
          p = s[i];
          if(ch == word_bigsmall(*p))
            point = i;
        }
        break;
    }
  }while(ch != '\r');
                                                                                
  return *(s[point]);
                                                                                
  /*  �H�W����
      p = s[point];
      return p[0];*/
}



/*
woju
*/
#define TRAP_ESC

#ifdef  TRAP_ESC
int KEY_ESC_arg;

int igetkey()
{
  int mode;
  int ch, last;

  mode = last = 0;
  while (1)
  {
    ch = igetch();
    if (mode == 0)
    {
      if (ch == KEY_ESC)
        mode = 1;
      else
        return ch;              /* Normal Key */
    }
    else if (mode == 1)
    {                           /* Escape sequence */
      if (ch == '[' || ch == 'O')
        mode = 2;
      else if (ch == '1' || ch == '4')
        mode = 3;
      else
      {
        KEY_ESC_arg = ch;
        return KEY_ESC;
      }
    }
    else if (mode == 2)
    {                           /* Cursor key */
      if (ch >= 'A' && ch <= 'D')
        return KEY_UP + (ch - 'A');
      else if (ch >= '1' && ch <= '6')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 3)
    {                           /* Ins Del Home End PgUp PgDn */
      if (ch == '~')
        return KEY_HOME + (last - '1');
      else
        return ch;
    }
    last = ch;
  }
}

#else                           /* TRAP_ESC */

int igetkey(void)
{
  int mode;
  int ch, last;

  mode = last = 0;
  while (1)
  {
    ch = igetch();
    if (ch == KEY_ESC)
      mode = 1;
    else if (mode == 0)         /* Normal Key */
      return ch;
    else if (mode == 1)
    {                           /* Escape sequence */
      if (ch == '[' || ch == 'O')
        mode = 2;
      else if (ch == '1' || ch == '4')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 2)
    {                           /* Cursor key */
      if (ch >= 'A' && ch <= 'D')
        return KEY_UP + (ch - 'A');
      else if (ch >= '1' && ch <= '6')
        mode = 3;
      else
        return ch;
    }
    else if (mode == 3)
    {                           /* Ins Del Home End PgUp PgDn */
      if (ch == '~')
        return KEY_HOME + (last - '1');
      else
        return ch;
    }
    last = ch;
  }
}
#endif                          /* TRAP_ESC */


