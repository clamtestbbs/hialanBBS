/* 使用錢的函數 */

#include "bbs.h"

/* 花錢選單 */
unsigned 
setuperm(pbits, nb, money)  /* 買權限用 */
  unsigned pbits;
  char nb;
  int money;
{
  register int i;
  
  i = nb - 'a';
  if (!((pbits >> i) & 1))
  {
    pbits ^= (1 << i);
    degold(money);
  }
  return(pbits);
}


/*買賣記錄*/
void 
tradelog(userid, i)
  char *userid;  
  int i;
{
  time_t now = time(0);
  char genbuf[200];
  char *item[4] = {"信箱上限","永久隱身","永久修改故鄉","信件無上限"};

  now = time(NULL) - 6 * 60;
  sprintf(genbuf, "在 %s 使用者 [1;32m%s[m 購買了[1;36m%s[m的權限",
    Cdate(&now),cuser.userid,item[i]);
  f_cat("log/trade.log",genbuf);
}

void
p_cloak()
{
  if (getans(currutmp->invisible ? "確定要現身?[y/N]" : "確定要隱身?[y/N]") != 'y')
    return;

  if(!currutmp->invisible)
  {
    if (check_money(2,GOLD)) 
      return;
    degold(2);
  }
  currutmp->invisible %= 2;
  pressanykey((currutmp->invisible ^= 1) ? MSG_CLOAKED : MSG_UNCLOAK);
  return;
}

void
p_fcloak()
{
  register int i;

  if (check_money(500,GOLD) || HAS_PERM(PERM_CLOAK))
  {
    if (HAS_PERM(PERM_CLOAK))
      pressanykey("你已經可以隱形了還來買，嫌錢太多啊？");
    return;
  }
  if (getans("確定要花 $500 學習終極隱身大法？[y/N]") != 'y')
    return;
  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  i = setuperm(cuser.userlevel,'g',500);
  update_data();
  cuser.userlevel = i;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  tradelog(cuser.userid,1);
  pressanykey("恭喜您已經學會了終極隱身大法!!");
  return;
}

void
p_from()
{
  
  if (check_money(5, GOLD)) 
    return;

  if (getans("確定要改故鄉?[y/N]") != 'y')
    return;

  if (getdata(b_lines, 0, "請輸入新故鄉:", currutmp->from, 17, DOECHO,0))
  {
    degold(5);
    currutmp->from_alias=0;
  }
  return;
}

void
p_ffrom()
{
  register int i;

  if(check_money(1000,GOLD) || HAS_PERM(PERM_FROM) || HAS_PERM(PERM_SYSOP))
  {
    if(HAS_PERM(PERM_FROM) || HAS_PERM(PERM_SYSOP))
      pressanykey("你已經可以修改故鄉了還來買，嫌錢太多啊？");
    return;
  }
  
  if (getans("確定要花 $1000 購買修改故鄉寶典？[y/N]") != 'y')
     return;
  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  i=setuperm(cuser.userlevel,'t',1000);
  update_data();
  cuser.userlevel=i;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  tradelog(cuser.userid,2);
  pressanykey("恭喜你得到了修改故鄉寶典");
  return;
}


void
p_exmail()
{
  char ans[4],buf[100];
  int  n;
  if(cuser.exmailbox >= MAXEXKEEPMAIL )
  {
    pressanykey("容量最多增加 %d 封，不能再買了。", MAXEXKEEPMAIL);
    return;
  }
  sprintf(buf,"您曾增購 %d 封容量，還要再買多少?",cuser.exmailbox);
  getdata(b_lines, 0, buf,ans,3,LCECHO,"10");
  n = atoi(ans);
  if(!ans[0] || !n )
    return;
  if(n+cuser.exmailbox > MAXEXKEEPMAIL ) 
     n = MAXEXKEEPMAIL - cuser.exmailbox;
  if(check_money(n*100,GOLD))
    return;
  degold(n*100);
  inmailbox(n);
  return;
}

void
p_spmail()
{
  char ans[4],buf[128];
  int n;
  if (HAVE_PERM(PERM_SYSOP) || HAVE_PERM(PERM_MAILLIMIT)) 
  {
    if(HAVE_PERM(PERM_MAILLIMIT))
      pressanykey("已經信箱無上限了, 不能賣啦!!");
    return;
  }
  if(HAS_PERM(PERM_LOGINOK))  // 是否通過身分認證 
  {
    if(cuser.exmailbox <=0)
    {
      pressanykey("Sorry,您沒有買過信箱所以不能賣 !!");
      return;
    }
    else  // 如果都 check 過了，主程式  
    {
      sprintf(buf,"你目前有 %d 封信箱容量可以賣出，你要賣出幾封 ? ",cuser.exmailbox);
      getdata(b_lines, 0, buf, ans, 3, LCECHO, "10");
      n = atoi(ans);
      if(!ans[0] || !n )
        return;
      if (n > cuser.exmailbox)  
      {
        n = cuser.exmailbox;
        pressanykey("超過可以賣出的數量!! 賣出數量改為 %d", cuser.exmailbox);
      }

      ingold(n*80);
      demailbox(n);
    }
  }
  sprintf(buf,"您目前賣出了 %d 封信，還有剩下 %d 封增加信箱容量",n, cuser.exmailbox);  
  pressanykey(buf);
  chkmailbox();
}


void
p_ulmail()
{
  register int i;
  if(check_money(100000,GOLD) || HAS_PERM(PERM_MAILLIMIT))
  {
    if(HAS_PERM(PERM_MAILLIMIT))
      pressanykey("你的信箱已經沒有限制了還來買，嫌錢太多啊？");
    return;
  }
  if (getans("確定要花 $100000 購買無上限信箱?[y/N]") != 'y')
    return;
  rec_get(fn_passwd, &xuser, sizeof(xuser), usernum);
  i=setuperm(cuser.userlevel,'f',100000);
  update_data();
  cuser.userlevel=i;
  substitute_record(fn_passwd, &cuser, sizeof(userec), usernum);
  tradelog(cuser.userid,3);
  pressanykey("恭喜您已經得到了無上限的信箱!!");
  return;
}

void
p_give()
{
   int money;
   char id[IDLEN+1],buf[256],reason[60];
   FILE *fp=fopen("tmp/givemoney","w");
   fileheader mymail;
   time_t now;
   time(&now);
   move(12,0);
   update_data();
   usercomplete("輸入對方的ID：", id);
   if (!id[0] || !getdata(14, 0, "要轉多少錢過去？", buf, 9, LCECHO,0)) return;
   money = atoi(buf);
   if(check_money(money,SILVER)) return;
   if(money > 0)
   {
     demoney(money);
     money *= 0.9;
     inumoney(id, money);
     sprintf(buf,"作者: %s \n"
                 "標題:[轉帳通知] 送你 %d 元唷！\n"
                 "時間: %s\n",cuser.userid,money,ctime(&now));
     fputs(buf,fp);
     while(!getdata(15,0,"請輸入理由：",reason,60,DOECHO ,"錢太多"));
     sprintf(buf,"[1;32m%s[37m 送你 [33m%d [37m元。\n"
                 "他的理由是：[33m %s [m",cuser.userid,money,reason);
     fputs(buf,fp);
     fclose(fp);
     sprintf(buf,"home/%s", id);
     stampfile(buf, &mymail);
     strcpy(mymail.owner, cuser.userid);
     f_mv ("tmp/givemoney",buf);
     sprintf(mymail.title,"[轉帳通知] 送你 %d 元唷！",money);
     sprintf(buf,"home/%s/.DIR",id);
     rec_add(buf, &mymail, sizeof(mymail));
     sprintf(buf,"[1;33m%s %s [37m把現金 [33m%d 元 [37m轉帳給[33m %s[37m",
     Cdate(&now),cuser.userid,money,id);
     f_cat("log/bank.log",buf);
   }
   return;
}


void
exchange()
{
  char buf[100], ans[10];
  int i, Money = 0;
  time_t now = time(0);
  
  move(12, 0);
  clrtobot();
  prints("你身上有金幣 %d 元,銀幣 %d 元\n", cuser.goldmoney, cuser.silvermoney);
  outs("\n金幣 ： 銀幣  =  1 ： 10000\n");
  if (!getdata(17, 0, "(1)銀幣換金幣  (2)金幣換銀幣 ", ans, 3, LCECHO, 0)) 
    return;

  if (ans[0] < '1' || ans[0] > '2') 
    return;

  i = atoi(ans);
  while (Money <= 0 || 
    (i == 1 ? (Money > cuser.silvermoney) : (Money > cuser.goldmoney)))
  {
    if (i == 1)
      getdata(18,0,"要拿多少銀幣來換？ ",ans,10,LCECHO,0);
    else
      getdata(18,0,"要拿多少金幣來換？ ",ans,10,LCECHO,0);
    if(!ans[0]) return;
    Money = atol(ans);
  }
  if(i == 1)
    sprintf(buf,"是否要轉換銀幣 %d 元 為金幣 %d ? [y/N]",Money,Money/10000);
  else
    sprintf(buf,"是否要轉換金幣 %d 元 為銀幣 %d ? [y/N]",Money,Money*10000);
  getdata(19,0,buf,ans,3,LCECHO,0);
  if(ans[0] == 'y')
  {
    if(i == 1)
    {       
      Money *= 1.05;
      demoney(Money);
      ingold(Money/10500);
      sprintf(buf,"[1;36m%s %s [37m把銀幣 [33m%d 元 [37m轉換為金幣 %d 元",
        Cdate(&now),cuser.userid,Money, Money/10500);
    }
    else
    {
      degold(Money);
      inmoney(Money*9500);
      sprintf(buf,"[1;32m%s %s [37m把金幣 [33m%d 元 [37m轉換為銀幣 %d 元",
        Cdate(&now),cuser.userid,Money, Money*9500);
    }
    f_cat("log/bank.log",buf);
    pressanykey("你身上有金幣 %d 元,銀幣 %d 元",cuser.goldmoney,cuser.silvermoney);
  }
  else
    pressanykey("取消.....");
}


/* 金庫 */
void
bank()
{
  char buf[10];
 
  if (lockutmpmode(BANK)) 
    return;

  setutmpmode(BANK);
  stand_title("神殿銀行");
  
  if (count_multi() > 1)
  {
    pressanykey("您不能派遣分身進入銀行呦 !");
    unlockutmpmode();    
    return;
  }
  
  counter(BBSHOME"/log/counter/銀行","使用銀行",0);
  move(2, 0);
  update_data();
  prints("\033[1;36m%12s\033[0;1m 您好呀！歡迎光臨本銀行。"
"[1;36m��������������������������������������������������������������\n"
"��[32m您現在有銀幣[33m %12d [32m元，金幣 [33m%12d[32m 元[36m        �鱋n"
"�僓����������������������������������������������������������槙n"
"�� 目前銀行提供下列三項服務：                               \n��",
    cuser.userid, cuser.silvermoney, cuser.goldmoney);
    move(6, 0);
      outs(""\
"��[33m1.[37m 轉帳 -- 必須扣掉總額的 10% 作為手續費 (限銀幣)[36m         �鱋n"
"��[33m2.[37m 匯兌 -- 銀幣/金幣 兌換 (抽取 5% 手續費) [36m               �鱋n"
"��[33mQ.[37m 離開銀行[36m                                               �鱋n"
"��������������������������������������������������������������[m");
  getdata(12, 0, "  請輸入您需要的服務：", buf, 3, DOECHO, 0);
  if (buf[0] == '1')
    p_give();
  else if (buf[0] == '2')
    exchange();

  update_data();  
  pressanykey("謝謝光臨，下次再來！");
  unlockutmpmode();
}

