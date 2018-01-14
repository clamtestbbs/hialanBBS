/*-------------------------------------------------------*/
/* user.c       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* author : opus.bbs@bbs.cs.nthu.edu.tw                  */
/* target : user configurable setting routines           */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#define _USER_C_

#include "bbs.h"
#undef MAXMONEY
#define MAXMONEY ((u->totaltime*10) + (u->numlogins*100) + (u->numposts*1000))

extern int numboards;
extern boardheader *bcache;
extern void resolve_boards();

char *sex[8] = { MSG_BIG_BOY, MSG_BIG_GIRL, MSG_LITTLE_BOY, MSG_LITTLE_GIRL,
                 MSG_MAN, MSG_WOMAN, MSG_PLANT, MSG_MIME };

/* ------------------- */
/* password encryption */
/* ------------------- */
int chkpasswd(char *passwd, char *test)
{
  extern char *crypt();
  static char pwbuf[14];
  char *pw;

  strncpy(pwbuf, test, 14);
  pw = crypt(pwbuf, passwd);
  return (!strncmp(pw, passwd, 14));
}

int bad_user_id(char *userid)
{
  register char ch;

  if (belong("etc/baduser",userid))
    return 1;

  if (strlen(userid) < 2)
    return 1;

  if (not_alpha(*userid))
    return 1;

  if (!ci_strcmp(userid, str_new))
    return 1;

  while (ch = *(++userid))
  {
    if (not_alnum(ch))
      return 1;
  }
  return 0;
}

void user_display(userec *u, int real)
{
  int diff; 
  int day = 0, hour = 0, min = 0;
  char genbuf[128];

  if (u->sex >= 8) 
    u->sex = 7;

  clrtobot();
  sethomedir(genbuf, u->userid);
  outs("[1;33m���w�w�w�w�w�w�w�w�w�w�w�w[42m   �ϥΪ̸��   [40m�w�w�w�w�w�w�w�w�w�w�w�w��\n");
  prints("\
  [32m�^��N���G[37m%-16.16s[32m�ʺ١G[37m%-20.20s[32m�ʧO�G[37m%-8.8s
  [32m�u��m�W�G[37m%-16.16s[32m
  [32m�X�ͤ���G[37m19%02i�~%02i��%02i��  [32m��Mail�G[37m%-40s\n",
    u->userid,u->username,sex[u->sex],
    u->realname,
    u->year, u->month, u->day, u->email); 
  prints("  [32m�W�����ơG[37m%-16d[32m���U����G[37m%s"
    ,u->numlogins,ctime(&u->firstlogin)); 
  prints("  [32m�峹�ƥءG[37m%-16d[32m�e���W���G[37m%s"
    ,u->numposts,ctime(&u->lastlogin)); 
  prints("  [32m�p�H�H�c�G[37m%-4d ��         [32m�Q���o��G[37m%s"
    ,rec_num(genbuf, sizeof(fileheader)),ctime(&u->dtime));
  prints(
"  [32m�����ƶq�G[37m%-16ld[32m�ȹ��ƶq�G[37m%-16ld[32m\n"
"  [32m�ȹ��W���G[37m%-16ld[32m�H�c�W���G[37m%d ��\n"
"  [32m�H����ơG[37m%-16ld[32m�n�_���ơG[37m%-16ld[32m�߱��G[37m%-4.4s\n"
"  [32m�o�T���ơG[37m%-16d[32m���T���ơG[37m%d\n"
"  [32m�W���a�I�G[37m%s \n"

,u->goldmoney,u->silvermoney
,MAXMONEY,(u->exmailbox+MAXKEEPMAIL)
,u->bequery,u->toquery,u->feeling,u->sendmsg,u->receivemsg
,u->lasthost
);

  if (real)
  {
    strcpy(genbuf, "bTCPRp#@XWBA#VSA?crFG??????????");
    for (diff = 0; diff < 31; diff++)
      if (!(u->userlevel & (1 << diff)))
        genbuf[diff] = '-';
    prints("  [32m�{�Ҹ�ơG[37m%-50.50s\n",u->justify);
    prints("  [32m�ϥ��v���G[37m%-32s\n",genbuf);
  }
  diff = u->totaltime / 60;
  day = diff / 1440;
  hour = (diff/60)%24;
  min = diff%60;
  prints("  [32m�W���ɶ��G[37m%d�� %d�p�� %d��\n",day,hour,min);

  if (u->userlevel >= PERM_BM)
  {
    int i;
    boardheader *bhdr;
    resolve_boards();

    outs("  [32m����O�D�G[37m");

    for (i = 0, bhdr = bcache; i < numboards; i++, bhdr++)
    {
      if(userid_is_BM(u->userid, bhdr->BM))
          {
              outs(bhdr->brdname);
              outc(' ');
          }
    }
    outc('\n');
  }
  outs("[33m���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��[m\n");

  if (!real)
    outs((u->userlevel & PERM_LOGINOK) ?
      "�z�����U�{�Ǥw�g�����A�w��[�J����" :
      "�p�G�n���@�v���A�аѦҥ������G���z���U");

#ifdef  NEWUSER_LIMIT
  if (!real)
    if ((u->lastlogin - u->firstlogin < 3 * 86400) && !HAS_PERM(PERM_POST))
      outs("�s��W���A�T�ѫ�}���v��");
#endif
}


void uinfo_query(userec *u, int real, int unum)
{
  userec x;
  register int i, fail, mail_changed;
  char ans, buf[STRLEN];
  char genbuf[200];
  int flag=0,temp;

  user_display(u, real);

  fail = mail_changed = 0;

  memcpy(&x, u, sizeof(userec));
  {
    char *choose[6] = {msg_choose_cancel ,"11)�ק���","22)�]�w�K�X","33)�]�w�v��","44)�M���b��","55)��ID"};
    ans = getans2(b_lines, 0, real ? "" : "�п�� ", choose, real ? 6 : 3, 0);
  }
  
  if (ans > '2' && !real)
    ans = '0';

  if (ans == '1' || ans == '3')
  {
    clear();
    i = 2;
    move(i++, 0);
    outs(msg_uid);
    outs(x.userid);
  }

  switch (ans)
  {
  case '1':       /*�o�̦��g�J�� .PASSWDS  �ҥH��V�ֶV�n..hialan.020729*/
    move(0, 0);
    outs("�гv���ק�C");
    getdata(i++, 0,"�z���ʺ١G",x.username, 24, DOECHO,x.username);
    strip_ansi(x.username,x.username,STRIP_ALL);

    getdata(i++, 0,"�{�b�߱��G",x.feeling, 5, DOECHO,x.feeling);
    x.feeling[4] = '\0';
    getdata(i++, 0,"�q�l�H�c�G",buf, 50, DOECHO,x.email);
    if(belong_spam(BBSHOME"/etc/spam-list",x.email))
    {
      pressanykey("��p,�����������A�� E-Mail �H�c��m");
      strcpy(buf,x.email);
    }
    if (strcmp(buf,x.email) && !not_addr(buf))
    {
      strcpy(x.email,buf);
      mail_changed = 1 - real;
    }
    strip_ansi(x.email,x.email,STRIP_ALL);

    //�ק�ʧO���a��
    {
      char *choose_sex[8]={"11)����","22)�j��","33)���}","44)����","55)����","66)����","77)�Ӫ�","88)�q��"};

      buf[0] = getans2(i++, 0, "�ʧO ", choose_sex, 8, u->sex + '1');
      if (buf[0] >= '1' && buf[0] <= '8')
        x.sex = buf[0] - '1';
    }
    
    while (1)
    {
      sprintf(genbuf,"%02i/%02i/%02i",u->year,u->month,u->day);
      getdata(i, 0, "�X�ͦ~�� 19", buf, 3, DOECHO, genbuf);
      x.year  = (buf[0] - '0') * 10 + (buf[1] - '0');
      getdata(i, 0, "�X�ͤ��   ", buf, 3, DOECHO, genbuf+3);
      x.month = (buf[0] - '0') * 10 + (buf[1] - '0');
      getdata(i, 0, "�X�ͤ��   ", buf, 3, DOECHO, genbuf+6);
      x.day   = (buf[0] - '0') * 10 + (buf[1] - '0');
      if (!real && (x.month > 12 || x.month < 1 ||
        x.day > 31 || x.day < 1 || x.year > 90 || x.year < 40))
        continue;
      i++;
      break;
    }

    if (real)
    {
      unsigned long int l;

// wildcat:���n�� user ���U���ç�W�r? 
      getdata(i++, 0,"�u��m�W�G",x.realname, 20, DOECHO,x.realname);
      strip_ansi(x.realname,x.realname,STRIP_ALL);
      sprintf(genbuf, "%d", x.numlogins);
      if (getdata(i, 0,"�W�u���ơG", buf, 10, DOECHO,genbuf))
        if ((l = atoi(buf)) >= 0)
          x.numlogins = (int) l;
      sprintf(genbuf,"%d", u->numposts);
      if (getdata(i++, 25, "�峹�ƥءG", buf, 10, DOECHO,genbuf))
        if ((l = atoi(buf)) >= 0)
          x.numposts = l;
      sprintf(genbuf, "%ld", x.silvermoney);
      if (getdata(i, 0,"���W�ȹ��G", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.silvermoney = l;
      sprintf(genbuf, "%ld", x.goldmoney);
      if (getdata(i, 25,"���W�����G", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.goldmoney = l;
      sprintf(genbuf, "%ld", x.sendmsg);
      if (getdata(i, 0,"�o���y�ơG", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.sendmsg = l;
      sprintf(genbuf, "%ld", x.receivemsg);
      if (getdata(i++, 25,"�����y�ơG", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.receivemsg = l;
      sprintf(genbuf, "%ld", x.bequery);
      if (getdata(i, 0,"�H��סG", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.bequery = l;
      sprintf(genbuf, "%ld", x.toquery);
      if (getdata(i++, 25,"�n�_�סG", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.toquery = l;
      sprintf(genbuf, "%ld", x.totaltime);
      if (getdata(i++, 0,"�W���ɼơG", buf, 10, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.totaltime = l;
      sprintf(genbuf, "%d", x.exmailbox);
      if (getdata(i++, 0,"�ʶR�H�c�ơG", buf, 4, DOECHO,genbuf))
        if ((l = atol(buf)) >= 0)
          x.exmailbox = (int) l;
      sprintf(genbuf, "%d", x.songtimes);
      if (getdata(i++, 0,"�I�q���ơG", buf, 4, DOECHO, genbuf))
        if ((l = atol(buf)) >= 0)
          x.songtimes = (int) l;
          
      getdata(i++, 0,"�{�Ҹ�ơG", x.justify, 39, DOECHO, x.justify);
      getdata(i++, 0,"�̪���{�����G", x.lasthost, 24, DOECHO,x.lasthost);


      fail = 0;
    }
    break;

  case '2':
    i = 19;
    if (!real)
    {
      if (!getdata(i++, 0, "�п�J��K�X�G", buf, PASSLEN, PASS,0) ||
        !chkpasswd(u->passwd, buf))
      {
        outs("\n\n�z��J���K�X�����T\n");
        fail++;
        break;
      }
    }

    if (!getdata(i++, 0, "�г]�w�s�K�X�G", buf, PASSLEN, PASS,0))
    {
      outs("\n\n�K�X�]�w����, �~��ϥ��±K�X\n");
      fail++;
      break;
    }
    strncpy(genbuf, buf, PASSLEN);

    getdata(i++, 0, "���ˬd�s�K�X�G", buf, PASSLEN, PASS,0);
    if (strncmp(buf, genbuf, PASSLEN))
    {
      outs("\n\n�s�K�X�T�{����, �L�k�]�w�s�K�X\n");
      fail++;
      break;
    }
    buf[8] = '\0';
    strncpy(x.passwd, genpasswd(buf), PASSLEN);
    break;

  case '3':
    i = DL_func("SO/admin.so:va_setperms",x.userlevel);
    //i = setperms(x.userlevel);
    if (i == x.userlevel)
      fail++;
    else {
      flag=1;
      temp=x.userlevel;
      x.userlevel = i;
    }
    break;

  case '4':
    i = QUIT;
    break;

  case '5':
    if (getdata(b_lines - 3, 0, "�s���ϥΪ̥N���G", genbuf, IDLEN + 1,
        DOECHO,x.userid))
    {
      if (searchuser(genbuf))
      {
        outs("���~! �w�g���P�� ID ���ϥΪ�");
        fail++;
      }
      else
      {
        strcpy(x.userid, genbuf);
      }
    }
    break;

  default:
    return;
  }

  if (fail)
  {
    pressanykey(NULL);
    return;
  }

//  if (getans(msg_sure_ny) == 'y')
  if (getans2(b_lines-1, 0, msg_sure, 0, 2, 'n') == 'y')
  {
    if (flag) 
      DL_func("SO/admin.so:va_Security",temp,i,cuser.userid,x.userid);
    if (strcmp(u->userid, x.userid))
    {
      char src[STRLEN], dst[STRLEN];

      sethomepath(src, u->userid);
      sethomepath(dst, x.userid);
      f_mv(src, dst);
      setuserid(unum, x.userid);
    }
    memcpy(u, &x, sizeof(x));
    substitute_record(fn_passwd, u, sizeof(userec), unum);
    if (mail_changed)
    {
#ifdef  REG_EMAIL
      x.userlevel &= ~PERM_LOGINOK;
      mail_justify(cuser);
#endif
    }

    if (i == QUIT)
    {
      char src[STRLEN], dst[STRLEN];

      sprintf(src, "home/%s", x.userid);
      sprintf(dst, "tmp/%s", x.userid);
// wildcat : ²��h�F , ���n�� rm home/userid
      if(dashd(src))
        f_mv(src , dst);
/*
woju
*/
      log_usies("KILL", x.userid);
      x.userid[0] = '\0';
      setuserid(unum, x.userid);
    }
    else
       log_usies("SetUser", x.userid);
    substitute_record(fn_passwd, &x, sizeof(userec), unum);
  }
}


int u_info()
{
  move(1, 0);
  update_data(); 
  uinfo_query(&cuser, 0, usernum);
  strcpy(currutmp->username, cuser.username);
  strcpy(currutmp->feeling, cuser.feeling);
  return 0;
}


int u_cloak()
{
  pressanykey((currutmp->invisible ^= 1) ? MSG_CLOAKED : MSG_UNCLOAK);
  return XEASY;
}


unsigned u_habit()
{
  unsigned fbits;
  register int i;
  int ch;
  char *choice[2]={"��","��"};
  
  fbits = cuser.habit;
  stand_title("�ϥΪ̳ߦn�]�w");
  move(2, 0);
  outs("�Ш̷ӨϥβߺD�ۦ�վ�(�����}, �欰��)�G\n");
  move(4, 0);
  for (i = 0; i < NUMHABITS; i++)
  {
    prints("%4s%c. %-28s %-7s\n"
       , " ", 'A' + i, habitstrings[i],((fbits >> i) & 1 ? choice[0] : choice[1]));

    if(i == 4)      /*�ݪO�C����ܦb�� 5 �� , �л\��*/
    { 
      move( i%NUMHABITS + 4, i <= 14 ? 36 : 72);
      prints((fbits >> i) & 1 ? "�峹��" : "�s��  ");
      outs("\n");
    }
  }
  clrtobot();
  move(b_lines, 0);
  outs("\033[1;37;46m  [��/��]�W�U  [Enter/q]���}  [�ť���]  ����  \033[m");
  
  i = ch = 0;
  while( ch != 'q' && ch != 'Q' && ch != '\r')
  { 
    move(i + 4, 0);
    outs("��");

    move(b_lines-3, 4);
    prints("\033[1m�ﶵ�G\033[1;36m%s\033[m", habitstrings[i]);
    move(b_lines-2, 4);
    prints("\033[1m�����G\033[1;36m%s\033[m", habit_help_strings[i]);

    ch = igetkey();
    move(i + 4, 0);
    outs("  ");
    
    move(b_lines-3, 4);
    prints("%50s", " ");
    move(b_lines-2, 4);
    prints("%80s", " ");
    
    switch(ch)
    {
    case KEY_UP:
      i--;
      if(i < 0) i = NUMHABITS - 1;
      break;
      
    case KEY_DOWN:
      i++;
      if(i >= NUMHABITS) i = 0;
      break;
    case ' ':
      if (i < 0 || i >= NUMHABITS)
        bell();
      else
      {
        fbits ^= (1 << i);
        move( i%NUMHABITS + 4, i <= 14 ? 36 : 72);
        if(i == 4)	/*�ݪO�C����ܦb�� 5 ��*/
          prints((fbits >> i) & 1 ? "�峹��" : "�s��  ");
        else
          prints((fbits >> i) & 1 ? choice[0] : choice[1]);
      }
      /* for HABIT_COLOR, �M���F�m��Ҧ��]��, i==1�O��ܲĤG��HABIT,
         ��HABIT_COLOR���b�ĤG��HABIT��, �o��]�n�����վ�.
                                                  - add by wisely - */
      if( i == 1) showansi ^= 1;
    }
  }
  update_data(); /* SiE: sync data prev bug user */
  cuser.habit = fbits;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum); /* �O�� */
  return RC_FULL;
}


static void showsig(char *uid, int i)
{
  char genbuf[STRLEN];
  clear();
  sethomefile(genbuf, uid, "sig.0");
  genbuf[strlen(genbuf) - 1] = i + '0';
  move(3, 0);
  prints("      [1;33m���w�w�w�w�w�w�w�w�w�w�w�w[42m   ñ�W�� %d   [40m�w�w�w�w�w�w�w�w�w�w�w�w��\033[m", i);
  show_file(genbuf, 4, MAXSIGLINES, ONLY_COLOR);
  move(4 + MAXSIGLINES, 0);
  prints("      [1;33m���w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w�w��\033[m", i);
}

void showplans(char* uid)
{
  char genbuf[200];
  char *choose_plans[5] = {"11)�򥻸��","22)ñ�W��","33)��ؤ峹","44)�d����(�L/�o)", msg_choose_cancel};
  
  if (!strcmp(uid, STR_GUEST))	/* alan.000330: guest�����n���ӤH��ذ� */
  {
    pressanykey("guest �L���\\��I");
    return;
  }

  sethomefile(genbuf, uid, fn_plans);
  if (dashf(genbuf))
    more(genbuf,YEA);
  else
  {
    clear();
    sprintf(genbuf,"%s �ثe�S���W��", uid);
    move(0,0);    
    clrtoeol();
    outs(genbuf);
  }

  switch (getans2(b_lines, 0, "", choose_plans, 5, 0))
  {
    case '1':    
      my_query(uid);
      break;
    case '2':
    {
      char *choose[10]={"11", "22", "33", "44", "55", "66", "77", "88", "99", msg_choose_cancel};
      char *choose_tmp[10];
      int i,n=0; //���X�� ñ�W��
      
      for(i=0;i<9;i++)
      {
        sethomefile(genbuf, uid, "sig.0");
        genbuf[strlen(genbuf) - 1] = i + '1';
        if(dashf(genbuf)) 
        {
          choose_tmp[n++] = choose[i];
          showsig(uid, i+1);
          pressanykey_old(NULL);
        }
      }
      
      if(n==0)
      {
        pressanykey("%s �S������ñ�W��", uid);
        break;
      }
      
      choose_tmp[n++] = choose[9];
      
      do
      {
        i = getans2(b_lines, 0, "�п��ñ�W��:", choose_tmp, n, 'q');;
        if(i >= '1' && i <= '9') showsig(uid, i-'0');
      }while(i>='1' && i<='9');
    }
      break;
    case '3':
      user_gem(uid);
      break;
    case '4':
      if (!strcmp(cuser.userid, STR_GUEST))
        pressanykey("guest �L���\\��I");
      else
        DL_func("SO/pnote.so:va_do_pnote",uid);
    default:
      break;
  }
  return;
}


int u_editfile()
{
  int mode;
  char ans, ans2, buf[128], msg1[64], msg2[16];
  char *choose[13]={"11)ñ�W�� �@",
  		    "22)ñ�W�� �G",
  		    "33)ñ�W�� �T",
  		    "44)ñ�W�� �|",
  		    "55)ñ�W�� ��",
  		    "66)ñ�W�� ��",
  		    "77)ñ�W�� �C",
  		    "88)ñ�W�� �K",
  		    "99)ñ�W�� �E",
  		    "00)�W����",
  		    "cC)�ۭq��Э�!!",
  		    "sS)�ڦ��l��W��",
  		    msg_choose_cancel};
  		    
  char *choose2[3]={"eE)�s��", "dD)�R��", msg_choose_cancel};
  
  ans = win_select("�ϥΪ��ɮ�", "�n�ݭ����ɮ� ?", choose , 13, 'q');
  switch(ans)
  {
    case '0':
      clear();
      sethomefile(buf, cuser.userid, fn_plans);
      more(buf, YEA);
      mode = EDITPLAN;
      strcpy(msg2, "�W����");
      break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      showsig(cuser.userid, ans-'0');
      sethomefile(buf, cuser.userid, "sig.0");
      buf[strlen(buf) - 1] = ans;
      mode = EDITSIG;
      strcpy(msg2, "ñ�W��");
      break;

    case 'c':
    {
      char buf[51];
      int unum;
      
      unum = do_getuser(cuser.userid, &xuser);
      clear();
      move(2,0);
      outs("�A�������� : ");
      outs(xuser.cursor);
      getdata(3, 0, "�п�J�s���:", buf, 51, DOECHO, xuser.cursor);
      if(buf[0] != '\0')
        strcpy(xuser.cursor, buf);
      
      if(buf[0] != '\0')
      {
        pressanykey("�ק��Ц��\\!!");
        substitute_record(fn_passwd, &xuser, sizeof(userec), unum);
      }
      else
        pressanykey("��Х��ק�!!");
      
      
      return 0;
    }

/*
    case 's':
      sethomefile(buf, cuser.userid, "spam-list");
      clear();
      show_file(buf, 2, 2, STRIP_ALL);
      strcpy(msg2, "�ڦ��l��W��");
      break;
*/    
    case 's':
      sethomefile(buf, cuser.userid, "spam-list");
      clear();
      ListEdit(buf);
      return 0;
      
    default:
      return;
  }
    
  ans2= getans2(b_lines, 0, msg1, choose2, 3, 'q');
  if (ans2 == 'e')
  {
    setutmpmode(mode);
    if (vedit(buf, NA) == -1)
      pressanykey("%s �S�����", msg2);
    else
      pressanykey("%s ��s����", msg2);
  }
  else if (ans2 == 'd')
    unlink(buf);
  return 0;
}

/* --------------------------------------------- */
/* �C�X�Ҧ����U�ϥΪ�                            */
/* --------------------------------------------- */
extern struct UCACHE *uidshm;
int usercounter, totalusers, showrealname;
ushort u_list_special;

static int
u_list_CB(uentp)
  userec *uentp;
{
  static int i;
  char permstr[8], *ptr;
  register int level;

  if (uentp == NULL)
  {
    move(2, 0);
    clrtoeol();
    prints("[7m  �ϥΪ̥N��   %-25s   �W��  �峹  %s  �̪���{���     [0m\n",
      showrealname ? "�u��m�W" : "�︹�ʺ�"
      ,HAS_PERM(PERM_SEEULEVELS) ? "����" : "");
    i = 3;
    return 0;
  }

  if (bad_user_id(uentp->userid))  /* Ptt */
    return 0;

  if (uentp->userlevel < u_list_special)
    return 0;

  if (i == b_lines)
  {
    prints(COLOR1"  �w��� %d/%d �H(%d%%)  "COLOR2"  (Space)[30m �ݤU�@��  [32m(Q)[30m ���}  [0m",
      usercounter, totalusers, usercounter * 100 / totalusers);
    i = igetch();
    if (i == 'q' || i == 'Q')
      return QUIT;
    i = 3;
  }
  if (i == 3)
  {
    move(3, 0);
    clrtobot();
  }

  level = uentp->userlevel;
  strcpy(permstr, "----");
  if (level & PERM_SYSOP)
    permstr[0] = 'S';
  else if (level & PERM_ACCOUNTS)
    permstr[0] = 'A';
  else if (level & PERM_DENYPOST)
    permstr[0] = 'p';

  if (level & (PERM_BOARD))
    permstr[1] = 'B';
  else if (level & (PERM_BM))
    permstr[1] = 'b';

  if (level & (PERM_XEMPT))
    permstr[2] = 'X';
  else if (level & (PERM_LOGINOK))
    permstr[2] = 'R';

  if (level & (PERM_CLOAK | PERM_SEECLOAK))
    permstr[3] = 'C';

  ptr = (char *) Etime(&uentp->lastlogin);
  ptr[18] = '\0';
  prints("%-14s %-27.27s%5d %5d  %s  %s\n",
    uentp->userid, showrealname ? uentp->realname : uentp->username
    ,uentp->numlogins, uentp->numposts,
    HAS_PERM(PERM_SEEULEVELS) ? permstr : "", ptr);
  usercounter++;
  i++;
  return 0;
}


int u_list()
{
  setutmpmode(LAUSERS);
  showrealname = u_list_special = usercounter = 0;
  totalusers = uidshm->number;
  if (HAS_PERM(PERM_SEEULEVELS))
  {
    char *choose[2]={"11)�S����", "22)����"};

    if (getans2(b_lines, 0, "�[�ݡH", choose, 2, '1') != '2')
      u_list_special = 32;
  }
  if (HAS_PERM(PERM_CHATROOM) || HAS_PERM(PERM_SYSOP))
  {
    char *choose[2]={"11)�u��m�W", "22)�ʺ�"};
    
    if (getans2(b_lines, 0, "��ܡH", choose, 2, '1') != '2')
      showrealname = 1;
  }
  u_list_CB(NULL);
  if (rec_apply(fn_passwd, u_list_CB, sizeof(userec)) == -1)
  {
    outs(msg_nobody);
    return XEASY;
  }
  move(b_lines, 0);
  clrtoeol();
  prints(COLOR1"  �w��� %d/%d ���ϥΪ�(�t�ήe�q�L�W��)  "COLOR2"  (�Ы����N���~��)  [m",
    usercounter, totalusers);
  igetkey();
  return 0;
}

#ifdef REG_MAGICKEY
/* shakalaca.000712: new justify */
int u_verify()
{
  char keyfile[80], buf[80], inbuf[15], *key;
  FILE *fp;

  if (HAS_PERM(PERM_LOGINOK))
  {
    pressanykey("�z�w�g�q�L�{��, �����n��J MagicKey");
    return XEASY;
  }

  sethomefile(keyfile, cuser.userid, fn_magickey);
  if (!dashf(keyfile))
  {
    if(win_select("�{�ҽX", "�z�٥��o�{�ҫH, �n�o�X��?", 0, 2, 'y') == 'y')
      mail_justify(cuser);
    
    return XEASY;
  }

  if (!(fp = fopen(keyfile, "r")))
  {
    pressanykey("�}���ɮצ����D, �гq������");
    fclose(fp);
    return XEASY;
  }

  fgets(buf, 80, fp);
  fclose(fp);  

  for(key=buf;*key;key++)
    if(*key == '\n')
    {
      *key='\0';
      break;
    }
  key = buf;
  
  getdata(b_lines, 0, "�п�J MagicKey�G", inbuf, 14, DOECHO, 0);
  if (*inbuf)
  {
    if (strcmp(key, inbuf))
      pressanykey("���~, �Э��s��J.");
    else
    {
      int unum = getuser(cuser.userid);
      
      unlink(keyfile);
      pressanykey("���߱z�q�L�{��, �w��[�J :)");
      cuser.userlevel |= (PERM_PAGE | PERM_POST | PERM_CHAT | PERM_LOGINOK);
      mail2user(cuser.userid, "[���U���\\�o]", BBSHOME"/etc/registered", 0);
      substitute_record (fn_passwd, &cuser, sizeof (cuser), unum);
    }
  }

  return RC_FULL;
}
#endif
