/*-------------------------------------------------------*/
/* menu.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : menu/help/movie routines                     */
/* create : 95/03/29                                     */
/* update : 03/07/12                                     */
/* change : hialan.nation@infor.org		         */
/*-------------------------------------------------------*/
#include "bbs.h"

/* ------------------------------------- */
/* help & menu processring               */
/* ------------------------------------- */
int refscreen = NA;
extern char *boardprefix;

void showtitle(char *title, char *mid)
{
  char buf[40];
  int spc, pad;

  spc = strlen(mid);

  if (title[0] == 0)
    title++;
  else if (chkmail(0))
  {
    mid = "\033[41;33;5m   �H�c�̭����s�H��I  \033[m\033[1m"COLOR1;
    spc = 22;      // CyberMax: spc �O�ǰt mid ���j�p.
  }
  else if(check_personal_note(1,NULL))
  {
    mid = "\033[43;37;5m    �����������d����   \033[m\033[1m"COLOR1;
    spc = 22;
  }
  else if (dashf(BBSHOME"/register.new") && HAS_PERM(PERM_ACCOUNTS))
  {
    mid = "\033[45;33;5m  ���s���ϥΪ̵��U�o!  \033[m\033[1m"COLOR1;
    spc = 22;
  }

  spc = 64 - strlen(title) - spc - strlen(currboard);

  if (spc < 0)
     spc = 0;
  pad = 1 - spc & 1;
  memset(buf, ' ', spc >>= 1);
  buf[spc] = '\0';

  clear();
  prints("%s\033[1m\033[33m��\033[37m%s\033[33m��"
  	 "\033[33m%s%s%s%s \033[0;45;36m��\033[1;33m%s�i%s�j\033[m\n",
    COLOR1, title, 
    buf, mid, buf, " " + pad,
    currmode & MODE_SELECT ? "�t�C" :
    currmode & MODE_DIGEST ? "��K" : "�ݪO", currboard);
}

/* ------------------------------------ */
/* �ʵe�B�z                              */
/* ------------------------------------ */
#define FILMROW 11
#define MENU_ROW 3
unsigned char menu_row = 13;
unsigned char menu_column = 4;
char mystatus[256];


/* wildcat 1998.8.7 */

void movie(int i)
{
  extern struct FILMCACHE *film;
  static short history[MAX_HISTORY];
  static char myweek[] = "��@�G�T�|����";
  static char buf[128],pbuf[128];
  char *msgs[] = {"��", "�}", "��", "��","��"};
  time_t now = time(NULL);
  struct tm *ptime = localtime(&now);

  resolve_garbage(); /* get film cache */

  if (currstat == GAME) return;
  if (HAVE_HABIT(HABIT_MOVIE))
  {
    if((!film->busystate) && film->max_film) /* race condition */
    {
      do
      {
        if (i != 1 || i != 0 || i != 999)
          i = 1 + (rand()%film->max_film);
        
        for (now = film->max_history; now >= 0; now--)
        {
          if (i == history[now])
          {
            i = 0;
            break;
          }
        }
      } while (i == 0);
    }

    memcpy(history, &history[1], film->max_history * sizeof(short));
    history[film->max_history] = now = i;

    if (i == 999)       /* Goodbye my friend */
      i = 0;

    setapath(pbuf, "Note");
    sprintf(buf, "%s/%s", pbuf, film->notes[i]);
    if(film->notes[i][0])
      show_file(buf, 13, FILMROW, NO_RELOAD);
  }

  i = ptime->tm_wday << 1;
  update_data();
  sprintf(mystatus, " %d:%02d %c%c  %0d/%0d  "
		    "�ڬO:\033[1m%-13s\033[m��\033[1;37m%6d%c,\033[m|��\033[1;33m%6d%c"
		    "\033[32m[�]%-2.2s]\033[41;37m%-18.18s\033[m",
    ptime->tm_hour, ptime->tm_min, myweek[i], myweek[i + 1],
    ptime->tm_mon + 1, ptime->tm_mday, cuser.userid, 
    (cuser.silvermoney/1000) <= 0 ? cuser.silvermoney : cuser.silvermoney/1000,
    (cuser.silvermoney/1000) <= 0 ? ' ' : 'k',
    (cuser.goldmoney/1000) <= 0 ? cuser.goldmoney : cuser.goldmoney/1000,
    (cuser.goldmoney/1000) <= 0 ? ' ' : 'k',    
    msgs[currutmp->pager],
    currutmp->birth ? "�ͤ�O�o�n�Ыȭ�!!" : film->today_is);
  move(1, 0);
  clrtoeol();
  outs(mystatus);
//  refresh();
}


/* ===== end ===== */

char movie2[11][256] = {"\0","\0","\0","\0","\0","\0","\0","\0","\0","\0","\0"};

int is_menu_stat()	/*�P�_�S�w�ݪO  by hialan 02/04/20*/
{
  if(currstat <= CLASS)
     return 1;
  return 0;
}

int show_movie2()  //�p�ݪO
{
  int i;
  char fn[256];
  FILE *fp;

  if(!is_menu_stat()) return 0;

  sprintf(fn,BBSHOME"/m2/%d",(rand()%10)+1);
  if(fp = fopen(fn,"r"))
  {
    while(fgets(movie2[i],256,fp) != NULL)
      i++;
    fclose(fp);
  }
  
  if(i < 11)
    for(;i<=11;i++) strcpy(movie2[i],"\0");
  
  return 1;
}

static int show_menu(struct one_key *p)
{
  register int n = 0, m = 0;
  register char *s;
  char buf[256];

  movie(0);
#ifdef HAVE_NOTE_2
  show_movie2();  /* Ū�� NOTE2 �����e*/
#endif

  move(MENU_ROW - 1,0);
  prints("%s      �\\  ��          ��    ��                  �� [Ctrl-Z] �ֳt���           \033[m",  COLOR3);

  move(MENU_ROW, 0);
  while ((s = p[n].desc)!=NULL || movie2[m][0]!='\0')  /*�n��ӦP�ɬ�0�~����*/
  {
    if ( s != NULL )  /*�p�G �ݪO���� ���O NULL*/
    {
      if (HAS_PERM(p[n].level))
      {
        prints("%*s  [\033[1;33m%c\033[m]", menu_column, "", s[0]);
        strcpy(buf,s+1);
        if(!is_menu_stat() || movie2[m][0]=='\0')
        {
          outs(buf);
          outs("\n");    
        }
        else
          prints("%-28s%s", buf,movie2[m++]);
      }
      n++;
    }
    else if (is_menu_stat() ) /*�ݪO�����O NULL , �ҥH�u��� movie2*/
      prints("%37s%-s", "", movie2[m++] );
    else
      movie2[m][0] = '\0';
  }
  
  return n - 1;
}


void
domenu(int cmdmode, char *cmdtitle, int cmd, struct one_key *cmdtable)
{

  int lastcmdptr;  	// cmdtable ������, �Ψӫ���{�b��ЩҦb���}�C��m
  int n, pos, total, i; // pos   �b switch �� cmd ��, ���s�p����ܦ�m��
  			// n     �M pos �ۦP�γ~, ���^���
  			// total �`�@���X�ӥi������
  			// i	 �i�����ت�����
  int err;
  int chkmailbox();
  static char cmd0[LOGIN];  //�C�� menu �ϥιL�᪺��m
  char bar_color[50];  

  get_lightbar_color(bar_color); //���o�����C��
  
  if (cmd0[cmdmode]) cmd = cmd0[cmdmode];
  setutmpmode(cmdmode);
  sprintf(tmpbuf,"%s [�u�W %d �H]",BOARDNAME,count_ulist());

  showtitle(cmdtitle, tmpbuf);
  total = show_menu(cmdtable);
  move(1,0);
  outs(mystatus);

  lastcmdptr = pos = 0;

  while (!HAS_PERM(cmdtable[lastcmdptr].level))  //count the first lastcmdptr
  {
    if(++lastcmdptr > total)
      return;
  }
  
  do
  {
    i = -1;

    switch (cmd)   /*�]���Ƕi�ӴN��cmd�F..�ҥH�ΥL��������w�]��*/
    {
    case KEY_ESC:
       if (KEY_ESC_arg == 'c')
          capture_screen();
       else if (KEY_ESC_arg == 'n') 
       {
          edit_note();
          refscreen = YEA;
       }
       i = lastcmdptr;
       break;
    case Ctrl('N'):
       New();
       refscreen = YEA;
       i = lastcmdptr;
       break;
    case Ctrl('A'):
    {
      int stat0 = currstat;
      currstat = RMAIL;
      if (man() == RC_FULL)
        refscreen = YEA;
      i = lastcmdptr;
      currstat = stat0;
      break;
    }
    case KEY_DOWN:
      i = lastcmdptr;

    case KEY_HOME:
    case KEY_PGUP:
      do
      {
        if (++i > total)
          i = 0;
      } while (!HAS_PERM(cmdtable[i].level));
      break;

    case KEY_END:
    case KEY_PGDN:
      i = total;
      break;

    case KEY_UP:
      i = lastcmdptr;
      do
      {
        if (--i < 0)
          i = total;
      } while (!HAS_PERM(cmdtable[i].level));
      break;

    case KEY_LEFT:
    case 'e':
    case 'E':
      if (cmdmode == MMENU)
        cmd = 'G';
      else if ((cmdmode == MAIL) && chkmailbox())
        cmd = 'R';
      else 
        return;
    default:
       if ((cmd == Ctrl('G') || cmd == Ctrl('S')) && (currstat == MMENU || currstat == TMENU || currstat == XMENU))  
       {
          if (cmd == Ctrl('S'))
             ReadSelect();
          else if (cmd == Ctrl('G'))
            Read();
          refscreen = YEA;
          i = lastcmdptr;
          break;
       }
      if (cmd == '\n' || cmd == '\r' || cmd == KEY_RIGHT)
      {
        boardprefix = cmdtable[lastcmdptr].desc;

        //�ഫ so ���禡
        if(cmdtable[lastcmdptr].mode && DL_get(cmdtable[lastcmdptr].fptr))
        {
          void *p = (void *)DL_get(cmdtable[lastcmdptr].fptr);
          if(p) 
            cmdtable[lastcmdptr].fptr = p;
          else 
            break;
        }

        currstat = XMODE;
        
        //����
        if(!cmdtable[lastcmdptr].fptr) return;
        err= (*((int (*)())cmdtable[lastcmdptr].fptr))();
        
        if (err == QUIT) return;

        currutmp->mode = currstat = cmdmode;

        if (err == XEASY)
        {
          refresh();
          sleep(1);
        }
        else if (err != XEASY + 1 || err == RC_FULL)
          refscreen = YEA;

        cmd = cmd0[cmdmode] = cmdtable[lastcmdptr].key;
        get_lightbar_color(bar_color);//��sbar_color
      }

      if (cmd >= 'a' && cmd <= 'z')
        cmd &= ~0x20;
      while (++i <= total)
      {
        if (cmdtable[i].key == cmd)
          break;
      }
    }  //end of switch.

    if (i > total || !HAS_PERM(cmdtable[i].level))
      continue;

    if (refscreen)
    {
      showtitle(cmdtitle, tmpbuf);
      show_menu(cmdtable);
      move(1, 0);
      outs(mystatus);
      refscreen = NA;
    }

    if(!HAVE_HABIT(HABIT_LIGHTBAR))
      cursor_clear(MENU_ROW + pos, menu_column);
    else
    {
      move(MENU_ROW + pos, 0);
      clrtoeol();
      prints("%*s  \033[0;37m[\033[1;33m%c\033[0;37m]%-27s\033[m ",
        menu_column,"",cmdtable[lastcmdptr].desc[0],cmdtable[lastcmdptr].desc + 1);
      if(is_menu_stat())
        outs(movie2[pos]);
    }

    n = pos = -1;
    while (++n <= (lastcmdptr = i))  //�p��Ӷ��O�i��ܪ��ĴX��
    {
      if (HAS_PERM(cmdtable[n].level))
        pos++;
    }
    
    if(!HAS_HABIT(HABIT_LIGHTBAR))
      cursor_show(MENU_ROW + pos, menu_column);
    else
    {
      move(MENU_ROW + pos, 0);
      clrtoeol();
      cursor_show(MENU_ROW+pos, menu_column);
      move(MENU_ROW + pos, menu_column+2);
      prints("\033[m%s[%c]%-27s\033[m ",
        bar_color, cmdtable[lastcmdptr].desc[0], cmdtable[lastcmdptr].desc+1);     
      if(is_menu_stat())
        outs(movie2[pos]);
      move(b_lines, 0);
    }  

  } while (((cmd = igetkey()) != EOF) || refscreen);

  abort_bbs();
}
/* INDENT OFF */

/*-------------------------------------------------------*/
/* menu.h       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : menu items                                   */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

// wildcat : ����u�Ϊ��ſ�� :p
int null_menu()
{
  pressanykey("�o�O�@�Ӫſ�� :p ");
  return 0;
}

void domenu();
/* ----------------------------------------------------- */
/* The set user menu for administrator                   */
/* ----------------------------------------------------- */


static struct one_key adminuser[] = {
'1',  null_menu,	PERM_ADMIN,	"1\033[1;31m�`�N:�T��f�ֵ��U��!!\033[m",0,
'2',  null_menu,    	PERM_ADMIN,	"2\033[1;31m�`�N:�T������ק��]\033[m",0,
'U',  "SO/admin.so:m_user",
			PERM_ACCOUNTS,  "User          [�ϥΪ̸��]",1,
'M',  "SO/register.so:m_newuser",
                        PERM_ACCOUNTS,	"Make User     [�s�W�ϥΪ�]",1,
'F',  "SO/admin.so:search_key_user",
                	PERM_ACCOUNTS,	"Find User     [�j�M�ϥΪ�]",1,  
'R',  "SO/admin.so:m_register",   	
                        PERM_ACCOUNTS,	"Register      [�f�ֵ��U��]",1,
'\0', NULL, 0, NULL, 0};

int
m_user_admin()
{
  domenu(ADMIN, "�]�w�ϥΪ�", 'U',adminuser);
  return 0;
}

/* ----------------------------------------------------- */
/* administrator's maintain menu                         */
/* ----------------------------------------------------- */

int XFile();

#ifdef  HAVE_MAILCLEAN
int m_mclean();
#endif

int hialan_test()
{
  u_verify();
}

static struct one_key adminlist[] = {
'H',  hialan_test, 		0, "Hialan test", 0,
'U',  m_user_admin, PERM_ACCOUNTS, "User Menu     [�ϥΪ̳]�w]",0,
'N',  "SO/admin.so:m_newbrd",
                       PERM_BOARD, "New Board     [�}�ҷs�ݪO]",1,
'S',  "SO/admin.so:m_board",
               		PERM_BOARD,"Set Board     [ �]�w�ݪO ]",1,
'X',  XFile,        	PERM_SYSOP,"Xfile         [�ק�t����]",0,
'C',  "SO/admin.so:reload_cache",
                	PERM_SYSOP,"Cache Reload  [ ��s���A ]",1,
'G',  "SO/admin.so:adm_givegold", 
                	PERM_SYSOP,"Give $$       [ �o����� ]",1,
#ifdef HAVE_MAILCLEAN
//'M',  "SO/admin.so:m_mclean, PERM_BBSADM, "Mail Clean    �M�z�ϥΪ̭ӤH�H�c",1,
#endif
'M',  "SO/admin.so:x_reg",
                     PERM_ACCOUNTS,"Merge         [�f�֭ײz��]",1,
'\0', NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* class menu                                            */
/* ----------------------------------------------------- */

int board(), local_board(), Boards(), ReadSelect() ,
    New(),Favor(),favor_edit(),good_board(),voteboard();

static struct one_key classlist[] = {
'V',  voteboard, PERM_BASIC,"VoteBoard    �ݪO�s�p�t��",0,
'C',      board, 	  0,"Class        ���������ݪO",0,
'N',        New, 	  0,"New          �Ҧ��ݪO�C��",0,
'L',local_board, 	  0,"Local        �����ݪO�C��",0,
'G', good_board, 	  0,"GoodBoard    �u�}�ݪO�C��",0,
'B',      Favor, PERM_BASIC,"BoardFavor   �ڪ��̷R�ݪO",0,
'F', favor_edit, PERM_BASIC,"FavorEdit    �s��ڪ��̷R",0,
'S', ReadSelect, 	  0,"Select       ��ܬݪO",0,
0,NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* Ptt money menu                                        */
/* ----------------------------------------------------- */

static struct one_key finlist[] = {
'0',  "SO/buy.so:bank",     0,      "0Bank           \033[1;36m�Ȧ�\033[m",1,
'1',  "SO/song.so:ordersong",0,     "1OrderSong      \033[1;33m�I�q��\033[m     $5g/��",1,
'2',  "SO/buy.so:p_cloak",  0,      "2Cloak          �{������/�{��  $2g/��     (�{���K�O)",1,
'3',  "SO/buy.so:p_from",   0,      "3From           �ק�G�m       $5g/��",1,
'4',  "SO/buy.so:p_exmail", 0,      "4BuyMailbox     �ʶR�H�c�W��   $100g/��",1,
'5',  "SO/buy.so:p_spmail", 0,      "5VenMailbox     ��X�H�c�W��   $80g/��",1,
'6',  "SO/buy.so:p_fcloak", 0,      "6UltimaCloak    �׷������j�k   $500g      �i�ä[����",1,
'7',  "SO/buy.so:p_ffrom",  0,      "7PlaceBook      �G�m�ק��_��   $1000g     User�W���F�i��G�m",1,
'8',  "SO/buy.so:p_ulmail", 0,      "8NoLimit        �H�c�L�W��     $100000g   �H�c�W���L����",1,
0, NULL, 0, NULL,0};

int
finance()
{
  domenu(FINANCE, "���Ĥ���", '0', finlist);
  return 0;
}

/* ----------------------------------------------------- */
/* Talk menu                                             */
/* ----------------------------------------------------- */

int t_users(), t_idle(), t_query(), t_pager();
// t_talk();
/* Thor: for ask last call-in message */
int t_display();
int t_bmw();

static struct one_key talklist[] = {

'L',  t_users,      0,              "List          �u�W�W��",0,
'P',  t_pager,      PERM_BASIC,     "Pager         �������A",0,
'I',  t_idle,       0,              "Idle          ��w�ù�",0,
'Q',  t_query,      0,              "QueryUser     �d�ߨϥΪ�",0,
/*
'T',  t_talk,       PERM_PAGE,      "Talk          [��H���]",0,
 */
'C',  "SO/chat.so:t_chat",PERM_CHAT,"ChatRoom      �s�u���",1,
/*
'D',  t_display,    0,              "Display       [���y�^�U]",0,
*/
'D',  t_bmw,        PERM_PAGE,      "Display       ���y�^�U", 0,
/*
'X',  XFile,        PERM_XFILE,     "Xfile         [�ק�t����]",0,
*/
0, NULL, 0, NULL,0};

/*-------------------------------------------------------*/
/* powerbook menu                                        */
/* ----------------------------------------------------- */

int my_gem();

static struct one_key powerlist[] = {
'G',  my_gem,              0,       "Gem           [�ڪ����]",0,
'-',  null_menu,           0,	    "----- ������ �\\�� -----",0,
'P',  "SO/pnote.so:p_read",0,       "Phone Answer  [ť���d��]",1,
'C',  "SO/pnote.so:p_call",0,       "Call phone    [�e�X�d��]",1,
'R',  "SO/pnote.so:p_edithint",0,   "Record        [���w���]",1,

0, NULL, 0, NULL,0};

int
PowerBook()
{
  domenu(POWERBOOK, "�U�Τ�U", 'G', powerlist);
  return 0;
}
/* ----------------------------------------------------- */
/* Habit menu                                            */
/* ----------------------------------------------------- */
int u_habit(), change_lightbar(), win_formchange(), edit_loginplan();
static struct one_key userhabit[] = {
'H', u_habit,		     0,  "Habit         [�ߦn�]�w]",0,
'B', change_lightbar,        0,	 "Bar Change    [�����C��]",0,
'W', win_formchange,	     0,  "Win Form      [�����~�[]",0,
'L', edit_loginplan,PERM_BASIC,  "Login Plan    [�W���]�w]",0,
0, NULL, 0, NULL,0};

static int
UserHabit()
{
  domenu(UMENU, "�ߦn�]�w", 'H', userhabit);
  return 0;
}
/* ----------------------------------------------------- */
/* User menu                                             */
/* ----------------------------------------------------- */

extern int u_editfile();
int u_info(), u_cloak(), u_list(), 
    PowerBook(), ListMain();

#ifdef REG_MAGICKEY
int u_verify();
#endif

static struct one_key userlist[] = {
'P',  PowerBook,    PERM_BASIC,	 "PowerBook     [�U�Τ�U]",0,
'I',  u_info,       PERM_BASIC,  "Info          [�ק���]",0,
'H',  UserHabit,    0, 	         "Habit         [�ߦn�]�w]",0,
'L',  ListMain,     PERM_LOGINOK,"List          [�]�w�W��]",0, 
'F',  u_editfile,   PERM_LOGINOK,"FileEdit      [�ӤH�ɮ�]",0,
'C',  u_cloak,      PERM_CLOAK,  "Cloak         [���αK�k]",0,

#ifdef REG_FORM
'R', "SO/register.so:u_register",PERM_POST,"Register      [����U��]",1,
#endif

#ifdef REG_MAGICKEY
'V',  u_verify,	    PERM_BASIC,  "Verify        [����U�X]",0,
#endif
'U',  u_list,       PERM_BASIC,  "Users         [���U�W��]",0,
0, NULL, 0, NULL,0};


/* ----------------------------------------------------- */
/* service menu                                          */
/* ----------------------------------------------------- */
int note();

static struct one_key servicelist[] = {
'F',  finance,      PERM_LOGINOK,   "Finance       [�ӫ~�j��]",0,
  
'V',  "SO/vote.so:all_vote",
                    PERM_LOGINOK,   "Vote          [�벼����]",1,
'N',  note,         PERM_LOGINOK,   "Note          [�g�d���O]",0,
'G',  "SO/soman.so:soman",
		    PERM_LOGINOK,   "Game          [�|�֤���]",1,
0,  NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* mail menu                                             */
/* ----------------------------------------------------- */
int m_new(), m_read(), m_send(),m_sysop(),mail_mbox(),mail_all(),
    setforward(),mail_list();

#ifdef INTERNET_PRIVATE_EMAIL
int m_internet();
#endif

static struct one_key maillist[] = {
'N',  m_new,        PERM_READMAIL,  "New           [�\\Ū�s�H]",0,
'R',  m_read,       PERM_READMAIL,  "Read          [�H��C��]",0,
'S',  m_send,       PERM_BASIC,     "Send          [�����H�H]",0,
'M',  mail_list,    PERM_BASIC,     "Mailist       [�s�ձH�H]",0,
'I',  m_internet,   PERM_INTERNET,  "Internet      [�����l��]",0,
'F',  setforward,   PERM_LOGINOK,   "Forward       [���H��H]",0,
'O',  m_sysop,      0,              "Op Mail       [�ԴA����]",0,
'Z',  mail_mbox,    PERM_INTERNET,  "Zip           [���]���]",0,
'A',  mail_all,     PERM_SYSOP,     "All           [�t�γq�i]",0,
0, NULL, 0, NULL,0};



/* ----------------------------------------------------- */
/* main menu                                             */
/* ----------------------------------------------------- */

static int
admin()
{
  domenu(ADMIN, "�����Ѥj", 'X', adminlist);
  return 0;
}

static int
BOARD()
{
  domenu(CLASS, "�ݪO�C��", 'G', classlist);
  return 0;
}

static int
Mail()
{
  domenu(MAIL, "�l����", 'R', maillist);
  return 0;
}

int
static Talk()
{
  domenu(TMENU, "��ѿ��", 'L', talklist);
  return 0;
}

static int
User()
{
  domenu(UMENU, "�ӤH�]�w", 'H', userlist);
  return 0;
}


static int
Service()
{
  domenu(PMENU, "�U�تA��", 'F', servicelist);
  return 0;
}


int Announce(), Boards(), Goodbye(), board(), Favor();


struct one_key cmdlist[] = {
'A',  Announce,     0,              "Announce      �Ѧa���",0,
'B',  BOARD,        0,              "Board         �ݪO�\\��",0,
'C',  board,        0,              "Class         �����ݪO",0,
'F',  Favor,        PERM_BASIC,     "Favor         �ڪ��̷R",0,
'M',  Mail,         PERM_BASIC,     "Mail          �H��\\��",0,
'T',  Talk,         0,              "Talk          �ͤѻ��a",0,
'U',  User,         0,              "User          �ӤH�u��",0,
'S',  Service,      PERM_BASIC,     "Service       �U�تA��",0,
'0',  admin,        PERM_ADMIN,     "0Admin        �t�κ޲z",0,
'G',  Goodbye,      0,              "Goodbye       ���t�d��",0,
0, NULL, 0, NULL,0};
/* INDENT ON */
