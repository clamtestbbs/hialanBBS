/*hialan �� spring, �ഫ id �εn�O�{��!!*/

#include "bbs.h"
extern struct BCACHE *brdshm;

#define FN_TRANB "tranb"

typedef struct{
  char brdname[IDLEN + 1];	//�쥻���ݪO�W��
  char title[BTLEN + 1];	//�쥻���ݪO���D
  char BM[IDLEN * 3 + 1];	//�쥻���ݪO�O�D
  
  char newname[IDLEN + 1];	//�s���ݪO�W��
  char newBM[IDLEN * 3 + 1];	//�s���ݪO�O�D
  
  time_t trantime;		//�ഫ�ӽЮɶ�
  
  char note[80];		//�Ƶ�(�p�G�H��n�[���򪺸�)
} tranbinfo;

#define TBI_ORI 0  //����l�O�W
#define TBI_NEW 1  //���s���O�W

static int belong_binfo(tranbinfo *ptr, int flag)
{
  // > 0	��ܸӥӽЪ���m
  //<= 0	��ܸ�ƥ��ӽ�
  char fname[80];
  int fd, size=sizeof(tranbinfo);
  tranbinfo tbinfo;
  int can=0, tag=0;
  
  sprintf(fname, "etc/%s", FN_TRANB);
  
  if((fd = open(fname, O_RDONLY)) >= 0)
  {
    while (read(fd, &tbinfo, size) == size)
    {
      can++;
      
      if(flag == TBI_ORI)
      {
        if(!strcmp(tbinfo.brdname, ptr->brdname))
        {
          tag = 1;
          break;
        }
      }
      else if(flag == TBI_NEW)
      {
        if(!strcmp(tbinfo.newname, ptr->newname) && strcmp(tbinfo.brdname, ptr->brdname))
        {
          tag = 1;
          break;
        }
      }
    }
    close(fd);
  }
  
  if (tag)
    return can;
  else
    return -1;
}

static void tranb_data(tranbinfo *ptr)
{
    move(2, 0);
    prints("�ݪO�W��:  %s\n"
           "�ݪO���D:  %s\n"
           "�ݪO�O�D:  %s\n"
           "�ӽЮɶ�:  %s\n"
           "�s�ݪO�W��:%s\n"
           "�s�ݪO�O�D:%s", 
             ptr->brdname, ptr->title, ptr->BM, ctime(&ptr->trantime),
             ptr->newname, ptr->newBM);
}

static int tranb_add()
{
  char bname[IDLEN + 1];
  boardheader *bptr;
  tranbinfo newinfo;
  char buf[256];
  int pos;//�̴X�����!!
  
  clear();
  
  make_blist();
  namecomplete(MSG_SELECT_BOARD, bname);
  
  bptr=&brdshm->bcache[getbnum (bname)] - 1;
  if(!userid_is_BM(cuser.userid, bptr->BM))
  {
    pressanykey("�A���O�ݪO�O�D!");
    return -1;
  }

  strcpy(newinfo.brdname, bptr->brdname);
  pos = belong_binfo(&newinfo, TBI_ORI);
  if(pos > 0)
  {
    sprintf(buf, "etc/%s", FN_TRANB);
    rec_get(buf, &newinfo, sizeof(tranbinfo), pos);
    
    clear();
    prints("�z�w�g�ӽйL�F, �O��%2d�����!!", pos);

    tranb_data(&newinfo);
           
    if(getans("�аݬO�_�ק�ӽ�(Y/N)�H[N]") !='y')
      return -1;
  }
  else
  {
    strcpy(newinfo.newname, newinfo.brdname);
    strcpy(newinfo.newBM, newinfo.BM);
    strcpy(newinfo.title, bptr->title);
    strcpy(newinfo.BM, bptr->BM);
    newinfo.trantime = time(0);
  }
  
  clear();
  stand_title("�ݪO�򥻸��");
  move(1, 0);
  prints("�ݪO�W��:  %s\n�ݪO���D:  %s\n�ݪO�O�D:  %s\n�ӽЮɶ�:  %s", 
           newinfo.brdname, newinfo.title, newinfo.BM, ctime(&newinfo.trantime));
  
  if(getans("�O�_�T�w(Y/n)�H[N]") != 'y')
  {
    pressanykey("�ϥΪ̨����C");
    return 0;
  }
  
  do
  {
    getdata(5, 0, "�п�J�s���ݪO�W��: ", newinfo.newname, IDLEN + 1, DOECHO, newinfo.newname);
  }while(!newinfo.newname[0] || belong_binfo(&newinfo, TBI_NEW) > 0);
  
  getdata(6, 0, "�п�J�s���O�D�W��: ", newinfo.newBM, IDLEN*3 + 1, DOECHO, newinfo.newBM);
  if(!newinfo.newBM[0])
  {
    pressanykey("��J���~, �ϥέ�]�w!!");
    strcpy(newinfo.newBM, newinfo.BM);
  }
  
  sprintf(buf, "etc/%s", FN_TRANB);
  if(pos<=0)
    rec_add(buf, &newinfo, sizeof(tranbinfo));
  else
    substitute_record(buf, &newinfo, sizeof(tranbinfo), pos);
  
  return 0;
}

static int tranb_show()
{
  char buf[80];
  tranbinfo tinfo;
  int pos;
  
  clear();
  make_blist();
  namecomplete(MSG_SELECT_BOARD, buf);

  strcpy(tinfo.brdname, buf);
  pos = belong_binfo(&tinfo, TBI_ORI);
  
  if(pos <= 0)
  {
    pressanykey("�ӬݪO�|���ӽ�!!");
    return -1;
  }
  
  sprintf(buf, "etc/%s", FN_TRANB);
  rec_get(buf, &tinfo, sizeof(tranbinfo), pos);
  
  tranb_data(&tinfo);
  pressanykey(NULL);
  
}

static void tranb_all_line(tranbinfo *ptr, int line)
{
  move(line, 0);
  clrtoeol();
  prints("%-4.4s %-13s %-13s %s", ptr->title, ptr->brdname, ptr->newname, ptr->newBM);
}

static tranb_all()
{
  char buf[80];
  tranbinfo binfo[MAXBOARD];
  int fd, size=sizeof(tranbinfo), i, pos;
  
  sprintf(buf, "etc/%s", FN_TRANB);

  if((fd = open(buf, O_RDONLY)) >= 0)
  {
    i = 0;
    
    while (read(fd, binfo+i, size) == size)
      i++;
    
    close(fd);
  }
  else
  {
    pressanykey("�}�ɿ��~!!");
    return -1;
  }
  
  if(i==0)
  {
    pressanykey("�ثe�èS������ӽ�!!");
    return 0;
  }

  sprintf(buf,"%s [�u�W %d �H]",BOARDNAME, count_ulist());
  showtitle("�Ҧ��C��", buf);
  outs("    �����N�䴫��!!\n");
  prints("%s���O ��l�O�W      �s�O�W        �s�O�D                                        \033[m", COLOR2);

  for(pos=0, i--;i>=0;i--, pos++)
  {
    if(pos == p_lines)
    {
      pressanykey(NULL);
      pos = 0;
    }
    tranb_all_line(binfo+i, 3+pos);
  }
  pressanykey(NULL);
}

int tranb()
{
  char buf[5];
  stand_title("�ݪO�ഫ�ӽе{��");
  
  getdata(1, 0, "�аݭn: [1]�ӽЬݪO�ഫ [2]�d�ߥӽЬ��� [3]�ݩҦ��ӽ� [Q]�����H[Q] ", buf, 5, LCECHO, 0);
  if(buf[0] == '1')
    tranb_add();
  else if(buf[0] == '2')
    tranb_show();
  else if(buf[0] == '3')
    tranb_all();
  
  return 0;
}