/*-------------------------------------------------------*/
/* stuff.c      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : utility routines                             */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#include "bbs.h"
#include <stdarg.h>


void setbdir(char *buf, char *boardname)
{
  sprintf(buf, "boards/%s/%s", boardname,
    currmode & MODE_DIGEST ? fn_mandex : ".DIR");
}

int invalid_fname(char *str)
{
  char ch;

  if (strspn(str, ".") == strlen(str))
     return 1;

  while (ch = *str++)
  {
    if (not_alnum(ch) && !strchr("@[]-._", ch))
      return 1;
  }
  return 0;
}


int
invalid_pname(str)
  char *str;
{
  char *p1, *p2, *p3;

  p1 = str;
  while (*p1) 
  {
    if (!(p2 = strchr(p1, '/')))
      p2 = str + strlen(str);
    
    if (p1 + 1 > p2 || p1 + strspn(p1, ".") == p2)
      return 1;
    
    for (p3 = p1; p3 < p2; p3++)
      if (not_alnum(*p3) && !strchr("@[]-._", *p3))
        return 1;
    
    p1 = p2 + (*p2 ? 1 : 0);
  }
  return 0;
}



int
valid_ident(ident)
  char *ident;
{
  static char *invalid[] = {"unknown@", "root@", "gopher@", "bbs@",
  "@bbs", "guest@", "@ppp", "@slip", NULL};
  char buf[128];
  int i;

  str_lower(buf, ident);
  for (i = 0; invalid[i]; i++)
    if (strstr(buf, invalid[i]))
      return 0;
  return 1;
}


/*
woju
*/
int userid_is_BM(char *userid, char *list)     /* �O�D�GBM list */
{
  register int ch, len;

  ch = list[0];
  if ((ch > ' ') && (ch < 128))
  {
    len = strlen(userid);
    do
    {
      if (!ci_strncmp(list, userid, len))
      {
        ch = list[len];
        if ((ch == 0) || (ch == '/') || (ch == ']'))
          return 1;
      }
      while (ch = *list++)
      {
        if (ch == '/')
          break;
      }
    } while (ch);
  }
  return 0;
}

/* ----------------------------------------------------- */
/* �ɮ��ˬd��ơG�ɮסB�ؿ��B�ݩ�                        */
/* ----------------------------------------------------- */
off_t
dashs(fname)
  char *fname;
{
  struct stat st;

  if (!stat(fname, &st))
        return (st.st_size);
  else
        return -1;
}


long
dasht(fname)
  char *fname;
{
  struct stat st;

  if (!stat(fname, &st))
        return (st.st_mtime);
  else
        return -1;
}


int
dashl(fname)
  char *fname;
{
  struct stat st;

  return (lstat(fname, &st) == 0 && S_ISLNK(st.st_mode));
}


dashf(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISREG(st.st_mode));
}


int
dashd(fname)
  char *fname;
{
  struct stat st;

  return (stat(fname, &st) == 0 && S_ISDIR(st.st_mode));
}


int
belong(filelist, key)
  char *filelist;
  char *key;
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[80], *ptr;

    while (fgets(buf, 80, fp))
    {
      if ((ptr = strtok(buf, str_space)) && !strcasecmp(ptr, key))
      {
        rc = 1;
        break;
      }
    }
    fclose(fp);
  }
  return rc;
}


int
belong_spam(filelist, key)
  char *filelist;
  char *key;
{
  FILE *fp;
  int rc = 0;

  if (fp = fopen(filelist, "r"))
  {
    char buf[STRLEN], *ptr;

    while (fgets(buf, STRLEN, fp))
    {
      if(buf[0] == '#') continue;
      if ((ptr = strtok(buf, " \t\n\r")) && strstr(key, ptr))
      {
        rc = 1;
        break;
      }
    }
    fclose(fp);
  }
  return rc;
}


char *
Cdatelite(clock)
  time_t *clock;
{
  static char foo[18];
  struct tm *mytm = localtime(clock);

  strftime(foo, 18, "%D %T", mytm);
  return (foo);
}


char *
Cdate(clock)
  time_t *clock;
{
  static char foo[18];
  struct tm *mytm = localtime(clock);

  strftime(foo, 18, "%D %T %a", mytm);
  return (foo);
}


void pressanykey_old(char *fmt, ...)
{
  va_list ap;
  char msg[128];
  int ch;

  msg[0]=0;

  va_start(ap, fmt);
  if(fmt) 
    vsprintf(msg, fmt, ap);
  va_end(ap);

  if (msg[0])
  {
    move(b_lines, 0); clrtoeol();
    prints(COLOR1"[1m�� [37m%-54s  "COLOR2"[�ť�]�� ESC_c�Ȧs [m", msg);

  }
  else
  {
    outmsg(COLOR1"[1m                        �� �Ы� [37m(Space/Return)"
    COLOR1" �~�� ��                         [m");
  }
  
  do
  {
    ch = igetkey();
    if (ch == KEY_ESC && KEY_ESC_arg == 'c')
      capture_screen();
  } while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

  move(b_lines, 0);
  clrtoeol();
  refresh();
}

void pressanykey(char *fmt, ...)
{
  va_list ap;
  char msg[128];
  int ch;
  screenline* screen = (screenline *)calloc(t_lines, sizeof(screenline));

  msg[0]=0;
  va_start(ap,fmt);
  if(fmt) 
    vsprintf(msg, fmt, ap);
  va_end(ap);
  
  vs_save(screen);
  if (msg[0])
  {
    int len;
    
    len = strlen(msg) + 12;
    if(len > 80) len = 80;
    if(len < 28) len = 28;  /*title*/
    
    msgbox(0, len, "�����N���~��....", msg, 1);
  }
  else
  {
    outmsg(COLOR1"[1m                        �� �Ы� [37m(Space/Return)"
    COLOR1" �~�� ��                         [m");
  }
  
  do
  {
    ch = igetkey();
    if (ch == KEY_ESC && KEY_ESC_arg == 'c')
      capture_screen();
  } while ((ch != ' ') && (ch != KEY_LEFT) && (ch != '\r') && (ch != '\n'));

  vs_restore(screen);
  refresh();
}


void bell()
{
  char sound[3], *ptr;

  ptr = sound;
  memset(ptr, Ctrl('G'), sizeof(sound));
  if (HAS_HABIT(HABIT_BELL))
    write(1, ptr, sizeof(sound));
}


int search_num(int ch, int max)
{
  int clen = 1;
  int x, y;
  extern unsigned char scr_cols;
  char genbuf[10];

  outmsg("[7m ���ܲĴX���G[0m");
  outc(ch);
  genbuf[0] = ch;
  getyx(&y, &x);
  x--;
  while ((ch = igetch()) != '\r')
  {
    if (ch == 'q' || ch == 'e')
      return -1;
    if (ch == '\n')
      break;
    if (ch == '\177' || ch == Ctrl('H'))
    {
      if (clen == 0)
      {
        bell();
        continue;
      }
      clen--;
      move(y, x + clen);
      outc(' ');
      move(y, x + clen);
      continue;
    }
    if (!isdigit(ch))
    {
      bell();
      continue;
    }
    if (x + clen >= scr_cols || clen >= 6)
    {
      bell();
      continue;
    }
    genbuf[clen++] = ch;
    outc(ch);
  }
  genbuf[clen] = '\0';
  move(b_lines, 0);
  clrtoeol();
  if (genbuf[0] == '\0')
    return -1;
  clen = atoi(genbuf);
  if (clen == 0)
    return 0;
  if (clen > max)
    return max;
  return clen - 1;
}


void stand_title(char *title)
{
  clear();
  prints("%s[1m�i %s �j[m\n", COLOR1, title);
}


/* opus : cursor position */
void cursor_show(row, column)
  int row, column;
{
  int i;

  i = (rand()%(strlen(cuser.cursor)/2))*2;
  move(row, column);
  prints("%c%c", cuser.cursor[i], cuser.cursor[i+1]);
  move(row, column + 1);
}


void cursor_clear(row, column)
  int row, column;
{
  move(row, column);
  outs(STR_UNCUR);
}


int
cursor_key(row, column)
  int row, column;
{
  int ch;

  cursor_show(row, column);
  ch = igetkey();
  cursor_clear(row, column);
  return ch;
}

void
printdash(mesg)
  char *mesg;
{
  int head = 0, tail;

  if (mesg)
    head = (strlen(mesg) + 1) >> 1;

  tail = head;

  while (head++ < 38)
    outch('-');

  if (tail)
  {
    outch(' ');
    outs(mesg);
    outch(' ');
  }

  while (tail++ < 38)
    outch('-');
  outch('\n');
}


// wildcat : ��سf��
int
check_money(unsigned long int money,int mode)
{
  unsigned long int usermoney;

  usermoney = mode ? cuser.goldmoney : cuser.silvermoney;
  if(usermoney<money)
  {
    move(1,0);
    clrtobot();
    move(10,10);
    pressanykey("��p�I�z���W�u�� %d ���A������I",usermoney);
    return 1;
  }
  return 0;
}

#if 0
// wildcat : �d�H���� :p
void
get_bonus()
{
  int money;
  time_t now = time(0);
  char buf[256];
  
  money= random()%2001;
  xuser.silvermoney +=money;
  sprintf(buf,"[1;31m���� [33m%s [31m��o�d�H���� [36m%d ���ȹ� , %s[m",
    cuser.userid,money,Etime(&now));
  f_cat(BBSHOME"/log/y2k_bonus",buf);

  pressanykey("���ߧA��o %d ���ȹ����d�H����",money);
}  
#endif

/* wildcat 981218 */
#define INTEREST_TIME	86400*7	// wildcat:7�ѵo��@���Q��
#define BANK_RATE	1.06	// wildcat:�Ȧ�Q�v 1.06 
#define SONG_TIME	86400	// hialan :�@�ѵo�@���벼����

void
update_data()
{
  int add = (time(0) - update_time)/30, change=0;
  static userec tuser;

  do_getuser(cuser.userid, &tuser);
  
  if((time(0) - tuser.update_songtime) >= SONG_TIME || tuser.songtimes > 5)
  {
    tuser.songtimes = 5;
    tuser.update_songtime = time(0);
    change = 1;
  }

  if((time(0) - tuser.dtime) >= INTEREST_TIME && tuser.silvermoney)
  {
    if(tuser.scoretimes < 100) tuser.scoretimes = 100; /*�峹�����o����*/
    tuser.silvermoney *= BANK_RATE;
    tuser.dtime = time(0);
    change = 1;
  }  
  
  if(add)
  {
    tuser.silvermoney += add*5;
    tuser.totaltime += (time(0)-update_time);
    update_time = time(0);
    change = 1;
  }
  cuser = tuser;
  if(change==1)
    substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
}

#if 0
/*---------------------------------------------------------------------*/
/* int chyiuan_ansi(buf,str,max)���Ϊk:�۵e����                        */
/* buf:chyiuan_ansi�L�᪺string                                        */
/* str:chyiuan_ansi���e��string                                        */
/* count:�Ǧ^move������shift���ƭ�                                     */
/* �Ƶ�:�p�G�O�m��Ҧ�, �W�L����Ȯ�, �|�屼�r��W�L����, ���O�dcolor  */
/*---------------------------------------------------------------------*/

int
chyiuan_ansi(buf,str,max)
  char *buf,*str;
  int max;
{
  int count = 0;
  int count0 = 0;
  int count1 = 0;
  char buf0[256];
  
  count0 = strip_ansi(buf0,str,0);
  if((cuser.uflag & COLOR_FLAG) && count0 <= max)
  {
    count1=strip_ansi(NULL,str,1);
    count=count1-count0;
    strcpy(buf, str);
  }
  else if((cuser.uflag & COLOR_FLAG) && count0 > max)
  {
    count0 = cut_ansistr(buf0,str,max);
    count1 = strip_ansi(NULL,buf0,1);
    count=count1-count0;
    strcpy(buf, buf0);
  }
  else
  {
    count=0;
    strcpy(buf, buf0);
  }
  return count;
}
#endif

int
answer(char *s)
{
  char ch;
  outmsg(s);
  ch = igetch ();
  if (ch == 'Y')
    ch = 'y';
  if (ch == 'N')
    ch = 'n';
  return ch;
}

#if defined(SunOS) || defined(SOLARIS)

#include <syslog.h>

void
setenv(name, value, overwrite)
  const char *name;
  const char *value;
  int overwrite;
{
  if (overwrite || (getenv(name) == 0))
  {
    char *p;

    if ((p = malloc(strlen(name) + strlen(value) + 2)) == 0)
    {
      syslog(LOG_ERR, "out of memory\n");
      exit(1);
    }
    sprintf(p, "%s=%s", name, value);
    putenv(p);
  }
}

atexit(procp)
void (*procp)();
{
   on_exit(procp, 0);
}

#endif

void edit_note()
{
   char fname[PATHLEN];
   int mode0 = currutmp->mode;
   int stat0 = currstat;
   char c0 = *quote_file;

   *quote_file = 0;
   setutmpmode(NOTE);
   sethomefile(fname, cuser.userid, "note");
   vedit(fname, 0);
   currutmp->mode = mode0;
   currstat = stat0;
   *quote_file = c0;
}


char*
my_ctime(const time_t *t)
{
  struct tm *tp;
  static char ans[100];

  tp = localtime(t);
  sprintf(ans, "%d/%02d/%02d %02d:%02d:%02d", tp->tm_year % 100,
     tp->tm_mon + 1,tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
  return ans;
}
#if 0
/* ----------------------------------------------------- */
/* �Ȧs�� TBF (Temporary Buffer File) routines           */
/* ----------------------------------------------------- */

char *tbf_ask()
{
  static char fn_tbf[10] = "buf.0";
  getdata(b_lines, 0, "�п�ܼȦs��(0-9)�G", fn_tbf + 4, 2, DOECHO,"0");
  return fn_tbf;
}
#endif


/*
 * check_personal_note() ���\�θ� chkmail() �@��..
 * �ҥH�i�H�[�b my_query() ��:�o�˧O�Hquery�ɴN�i�H�ݨ즳�S���s�d��
 * �٦��i�H�[�b show_title() ��:�o�˦��s�d���ɴN�|�����s�H��@�˦btitle���ܳ�!
 */

int
check_personal_note(int newflag, char* userid) 
{
 char fpath[PATHLEN];
 FILE *fp;
 int  total = 0;
 notedata myitem;
 char *fn_note_dat      = "pnote.dat";

 if (userid == NULL)
   sethomefile(fpath, cuser.userid, fn_note_dat);
 else
   sethomefile(fpath, userid, fn_note_dat);

 if ((fp = fopen(fpath, "r")) != NULL) 
 {
   while (fread(&myitem, sizeof(myitem), 1, fp) == 1) 
   {
     if (newflag)
       if (myitem.buf[0][0] == 0) total++;
     else
       total++;
   }
   fclose(fp);
   return total;
 }
 return 0;
}

int show_help(int mode)
{
  if(inmore)
//    more(BBSHOME"/etc/help/MORE.help",YEA);
    return 0;
  else if(mode == LUSERS)
//    more(BBSHOME"/etc/help/LUSERS.help",YEA);
    return 0;
  else if(mode == READBRD || mode == READNEW)
//    more(BBSHOME"/etc/help/BOARD.help",YEA);
    return 0;
  else if(mode == RMAIL)
//    more(BBSHOME"/etc/help/MAIL.help",YEA);
    return 0;
  else if(mode == READING)
//    more(BBSHOME"/etc/help/READ.help",YEA);
    return 0;
  else if(mode == ANNOUNCE)
//    more(BBSHOME"/etc/help/ANNOUNCE.help",YEA);
    return 0;
  else if(mode == EDITING)
//    more(BBSHOME"/etc/help/EDIT.help",YEA);
    return 0;
  else
    HELP();
  return 0;
}

int
mail2user(muser, title, fname, filemode)
  char *muser, *title, *fname;
  int filemode;
{
  fileheader mhdr;
  char buf[256], buf1[80];

  sethomepath(buf1, muser);
  stampfile(buf1, &mhdr);

  if(filemode != 0)
    strcpy(mhdr.owner, "[��.��.��]");    
  else
    strcpy(mhdr.owner, cuser.userid);

  strcpy(mhdr.title, title);
  mhdr.savemode = 0;
  mhdr.filemode = filemode;
  sethomedir(buf, muser);
  rec_add(buf, &mhdr, sizeof(mhdr));
  f_cp(fname, buf1, O_TRUNC);
  return 0;
}

void
debug(mode)
  char *mode;
{
  time_t now = time(0);
  char buf[128];

  sprintf(buf, "%s %s %s\n", Etime(&now), mode, cuser.userid);      
  f_cat("debug",buf);
}

/*-------------------------------------------------------*/
/* register.c   ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : user register routines                       */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

/* origin: SOB & Ptt              */
/* modify: wildcat/980909         */
/* �T�{user�O�_�q�L���U�B��ƥ��T */
check_register()
{
  char *ptr;
  char genbuf[200],buf[100];

  if(!HAS_PERM(PERM_POST) && (cuser.lastlogin - cuser.firstlogin >= 86400))
    cuser.userlevel |= PERM_POST;  

  stand_title("�иԲӶ�g�ӤH���");

  while (strlen(cuser.username) < 2)
    getdata(2, 0, "�︹�ʺ١G", cuser.username, 24, DOECHO,0);
  strcpy(currutmp->username, cuser.username);

  for (ptr = cuser.username; *ptr; ptr++)
    if (*ptr == 9)              /* TAB convert */
      strcpy(ptr, " ");

  while (strlen(cuser.feeling) < 2)
    getdata(3, 0, "�߱����A�G", cuser.feeling, 5, DOECHO,0);
  cuser.feeling[4] = '\0';
  strcpy(currutmp->feeling, cuser.feeling);

  while (strlen(cuser.realname) < 4)
    getdata(4, 0, "�u��m�W�G", cuser.realname, 20, DOECHO,0);

  while (!cuser.month || !cuser.day || !cuser.year)
  {
    sprintf(genbuf, "%02i/%02i/%02i", cuser.year,cuser.month, cuser.day);
    getdata(6, 0, "�X�ͦ~�� �褸 19", buf, 3, DOECHO,0);
    cuser.year = (buf[0] - '0') * 10 + (buf[1] - '0');
    getdata(7, 0, "�X�ͤ��", buf, 3, DOECHO,0);
    cuser.month = (buf[0] - '0') * 10 + (buf[1] - '0');
    getdata(8, 0, "�X�ͤ��", buf, 3, DOECHO,0);
    cuser.day = (buf[0] - '0') * 10 + (buf[1] - '0');
    if (cuser.month > 12 || cuser.month < 1 ||
      cuser.day > 31 || cuser.day < 1 || cuser.year > 90 || cuser.year < 40)
      continue;
    break;
  }

  while (cuser.sex > 7)
  {
    char buf;
    char *choose_sex[8]={"11.����","22.�j��","33.���}","44.����","55.����","66.����","77.�Ӫ�","88.�q��"};

    buf = getans2(10, 0, "�ʧO ", choose_sex, 8, cuser.sex + '1');    
    if (buf >= '1' && buf <= '8')
      cuser.sex = buf - '1';
  }

  if (belong_spam(BBSHOME"/etc/spam-list",cuser.email))
  {
    strcpy(cuser.email,"NULL");
    pressanykey("��p,�����������A�� E-Mail �H�c��m");
  }
  
  if (!strchr(cuser.email, '@'))
  {
    bell();
    move(t_lines - 4, 0);
    prints("\
�� ���F�z���v�q�A�ж�g�u�ꪺ E-mail address�A �H��T�{�դU�����A\n\
   �榡�� [44muser@domain_name[0m �� [44muser@\\[ip_number\\][0m�C\n\n\
�� �p�G�z�u���S�� E-mail�A�Ъ����� [return] �Y�i�C");

    do
    {
      getdata(12, 0, "�q�l�H�c�G", cuser.email, 50, DOECHO,0);
      if (!cuser.email[0])
        sprintf(cuser.email, "%s%s", cuser.userid, str_mail_address);
      if(belong_spam(BBSHOME"/etc/spam-list",cuser.email))
      {
        strcpy(cuser.email, "NULL");
        pressanykey("��p,�����������A�� E-Mail �H�c��m");
      }
    } while (!strchr(cuser.email, '@'));

#ifdef  REG_MAGICKEY   
    mail_justify(cuser);
#endif

  }

  cuser.userlevel |= PERM_DEFAULT;
  if (!HAS_PERM(PERM_SYSOP) && !(cuser.userlevel & PERM_LOGINOK))
  {
    /* �^�йL�����{�ҫH��A�δ��g E-mail post �L */

    sethomefile(genbuf, cuser.userid, "email");
    if (dashf(genbuf))
      cuser.userlevel |= ( PERM_POST );

#ifdef  STRICT
    else
    {
      cuser.userlevel &= ~PERM_POST;
      more("etc/justify", YEA);
    }
#endif

  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
/* wildcat 981218 */
    clear();
    update_data(); 

#ifdef REG_FORM
    if (HAS_PERM(PERM_POST) && !HAS_PERM(PERM_LOGINOK)
      && answer("�O�_�n��g���U�� (y/N)") == 'y')
    DL_func("SO/register.so:u_register");
#endif

  }
  if (HAS_PERM(PERM_DENYPOST) && !HAS_PERM(PERM_SYSOP))
    cuser.userlevel &= ~PERM_POST;
}

/*-------------------*/
/*  hialan's script  */
/*-------------------*/
int
change_bp(y, title, desc)
  char desc[3][80];
  char *title;
  int y;
{
  int i, ch;
  char barcolor[50];
  char *bgcolor="\033[0;30;46m";
  
  move(y, 0);
  clrtoeol();
  prints("   \033[0m\033[30m\033[47m    %-53s%-16s\033[m", title, "�� q �䵲��");

  y++;
  for(i = 0;i < 3;i++)
  {
    move(i+y,0);
    clrtoeol();
    prints("     %s%-71s\033[m", bgcolor, desc[i]);
  }

  get_lightbar_color(barcolor);
  i = 0;
  while(1)
  {
    move(i+y,0);
    clrtoeol();
    prints("   ��%s%-71s\033[m", barcolor, desc[i]);
    
    ch = igetkey();
    
    move(i+y,0);
    clrtoeol();
    prints("     %s%-71s\033[m", bgcolor, desc[i]);
   
    switch(ch)
    {
      case KEY_UP:
        i--;
        if(i < 0) i = 2;
        break;
        
      case KEY_DOWN:
        i++;
        if(i > 2) i = 0;
        break;
      
      case '\r':
      
        do
        {
          getdata(i+y, 0, "     ", desc[i], 71, DOECHO, desc[i]);
 
          move(i+y,0);
          clrtoeol();
          prints("     %s%-71s\033[m", bgcolor, desc[i]);

          i++;
          if(i > 2) i = 0;
        }while(desc[i][0] == '\0' && i != 0);
      
        break;
      
      case 'Q':
      case 'q':
        return 0;
    }
  }
  pressanykey(NULL);
}

#define FN_GAMELOG "etc/game.log"

int game_log(char *game, ...)
{
  va_list ap;
  char *fmt, desc[128];
  FILE *fp;
  time_t now=time(0);
  
  va_start(ap,game);
    fmt = va_arg(ap, char *);
    vsprintf(desc, fmt, ap);
  va_end(ap);
  
  fp=fopen(FN_GAMELOG, "a+");
  fprintf(fp, "\033[1;37m%s \033[33m%s \033[32m%s \033[36m%s\033[m\n", 
              game, Etime(&now), cuser.userid, desc);
  fclose(fp);
  
  return 0;
}
