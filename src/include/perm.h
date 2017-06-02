/*-------------------------------------------------------*/
/* perm.h       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : permission levels of user & board            */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

#ifndef _PERM_H_
#define _PERM_H_

/* ----------------------------------------------------- */
/* These are the 16 basic permission bits.               */
/* ----------------------------------------------------- */

#define PERM_BASIC        000000000001
#define PERM_CHAT         000000000002
#define PERM_PAGE         000000000004
#define PERM_POST         000000000010
#define PERM_LOGINOK      000000000020
#define PERM_MAILLIMIT    000000000040
#define PERM_CLOAK        000000000100
#define PERM_SEECLOAK     000000000200
#define PERM_XEMPT        000000000400
#define PERM_DENYPOST     000000001000
#define PERM_BM           000000002000
#define PERM_ACCOUNTS     000000004000
#define PERM_CHATROOM     000000010000
#define PERM_BOARD        000000020000
#define PERM_SYSOP        000000040000
#define PERM_POSTMASK     000000100000 
#define PERM_BBSADM       000000100000 
#define PERM_NOTOP        000000200000 
#define PERM_XFILE        000000400000 
#define PERM_RESEARCH	  000001000000 
#define PERM_FROM         000002000000 
#define PERM_GOOD         000004000000 
#define PERM_22           000010000000 
#define PERM_ANNOUNCE     000020000000 
#define PERM_MG           000040000000 
#define PERM_SMG          000100000000 
#define PERM_AD           000200000000 
#define PERM_SAD          000400000000 
#define PERM_PAINT        001000000000 
#define PERM_SPAINT       002000000000 
#define PERM_SECRETARY    004000000000 
#define PERM_LSYSOP       010000000000 
#define PERM_WILDCAT      020000000000 

/* ----------------------------------------------------- */
/* These permissions are bitwise ORs of the basic bits.  */
/* ----------------------------------------------------- */

 
/* �s�ϥΪ֦̾����v�� */
/* wildcat :
   ���n�� �i��PERM_POST�����A�אּ����email�{�ҫ�観 */
#define PERM_DEFAULT    (PERM_BASIC | PERM_POST)

#define PERM_ADMIN      (PERM_ACCOUNTS | PERM_SYSOP)
#define PERM_ALLBOARD   (PERM_SYSOP | PERM_BOARD)
#define PERM_LOGINCLOAK (PERM_SYSOP | PERM_ACCOUNTS)
#define PERM_SEEULEVELS PERM_SYSOP
#define PERM_SEEBLEVELS (PERM_SYSOP | PERM_BM)
#define PERM_NOTIMEOUT  PERM_SYSOP

#define PERM_READMAIL   PERM_BASIC
#define PERM_FORWARD    PERM_BASIC      /* to do the forwarding */

#define PERM_INTERNET   PERM_LOGINOK    /* �����{�ҹL�����~��H�H�� Internet */

#define HAS_PERM(x)     ((x)?cuser.userlevel&(x):1)
#define HAVE_PERM(x)    (cuser.userlevel&(x))
#define PERM_HIDE(u) ((u)->userlevel & PERM_SYSOP && (u)->userlevel & PERM_DENYPOST)


/* ----------------------------------------------------- */
/* �U���v��������N�q                                    */
/* ----------------------------------------------------- */

#define NUMPERMS        32

#ifndef _ADMIN_C_
extern char *permstrings[];

#else
char *permstrings[] = {
  "���v�O",                   /* PERM_BASIC */
  "�i�J��ѫ�",                 /* PERM_CHAT */
  "��H���",                   /* PERM_PAGE */
  "�o���峹",                   /* PERM_POST */
  "���U�{�ǻ{��",               /* PERM_LOGINOK */
  "�H��L�W��",                 /* PERM_MAILLIMIT */
  "�����N",                     /* PERM_CLOAK */
  "�ݨ��Ԫ�",                   /* PERM_SEECLOAK */
  "�ä[�O�d�b��",               /* PERM_XEMPT */
  "���������N",                 /* PERM_DENYPOST */
  "�O�D",                       /* PERM_BM */
  "�b���`��",                   /* PERM_ACCOUNTS */
  "��ѫ��`��",                 /* PERM_CHATCLOAK */
  "�ݪO�`��",                   /* PERM_BOARD */
  "����",                       /* PERM_SYSOP */
  "BBSADM",                     /* PERM_BBSADM & PERM_POSTMASK */
  "���C�J�Ʀ�]",               /* PERM_NOTOP */
  "�޲z���W���",               /* PERM_XFILE */
  "��o�p��",                   /* PERM_RESEARCH */
  "�ק�G�m",                   /* PERM_FROM */
  "�����i���P",                 /* PERM_GOOD */
  "�S�Q��",                     /* PERM_ */
  "��ذ��`��",                 /* PERM_Announce */
  "�S�Ȳ�",                     /* PERM_MG */
  "�S�Ȳժ�",                   /* PERM_SMG */
  "��Ų�",                     /* PERM_AD */
  "��Ųժ�",                   /* PERM_SAD */
  "���u��",                     /* PERM_PAINT */
  "���u�ժ�",                   /* PERM_SPAINT */
  "����",                       /* PERM_SECRETARY */
  "�p����",                     /* PERM_LSYSOP */
  "���ߦѤj"                    /* PERM_CAVE */  
};

#endif
#endif                          /* _PERM_H_ */

/*-------------------------------------------------------*/
/* habit.h      ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : �ϥΪ̳ߦn�]�w                               */
/* create : 98/05/21                                     */
/*-------------------------------------------------------*/

#ifndef _HABIT_H_
#define _HABIT_H_

/* ----------------------------------------------------- */
/* these are habits in userec.habit                      */
/* ----------------------------------------------------- */

#define HABIT_MOVIE     000000000001    /* �}/���ʺA�ݪO */
#define HABIT_COLOR     000000000002    /* �m��/�¥դ��� */
#define HABIT_NOTE      000000000004    /* ��ܯd���O */
#define HABIT_ALARM	000000000010    /* �b�I���� */
#define HABIT_BELL	000000000020	/* �n�� */
#define HABIT_BOARDLIST	000000000040    /* �ݪO�C����ܤ峹�ƩάO�s�� */
#define HABIT_SEELOG	000000000100    /* �W�����h�ݬ����ƦW? */
#define HABIT_CYCLE	000000000200    /* �`�����\Ū */
#define HABIT_FEELING	000000000400
#define HABIT_FROM	000000001000
#define HABIT_LIGHTBAR	000000002000	/* ���ο�� */
#define HABIT_ALREADYSET	000000020000    /* �C���W�����]�w? */
#define HABIT_NOBRDHELP	000000040000	/* ����ܬݪO���� */

#define HAS_HABIT(x)     ((x)?cuser.habit&(x):1)
#define HAVE_HABIT(x)    (cuser.habit&(x))

#define HABIT_NEWUSER    (HABIT_MOVIE | HABIT_COLOR | HABIT_SEELOG | HABIT_ALARM | HABIT_BELL | HABIT_LIGHTBAR)
#define HABIT_GUEST	 (HABIT_MOVIE | HABIT_COLOR | HABIT_SEELOG | HABIT_LIGHTBAR)
/* ----------------------------------------------------- */
/* �U�سߦn�]�w������N�q                                */
/* ----------------------------------------------------- */


#ifndef _USER_C_
extern char *habitstrings[];

#else

#define NUMHABITS        13

char *habitstrings[] = {
/* HABIT_MOVIE */      "�ʺA�ݪO       ",
/* HABIT_COLOR */      "�m����ܼҦ�   ",
/* HABIT_NOTE  */      "�W����ܯd���O ",
/* HABIT_ALARM */      "�b�I����       ",
/* HABIT_BELL  */      "�n��           ",
/* HABIT_BOARDLIST */  "�ݪO�C�����   ",
/* HABIT_SEELOG	*/     "�W���[�ݦU������",
/* HABIT_CYCLE */      "�`�����\\Ū    ",
/* HABIT_FEELING */    "�W���]�w�߱�   ",
/* HABIT_FROM */       "�W���]�w�G�m   ",
/* HABIT_LIGHTBAR */   "�ϥο�����   ",
/* HABIT_ALREADYSET */ "�W�����ק�ߦn�]�w",
/* HABIT_NOBRDHELP */  "����ܬݪO����"
};


char *habit_help_strings[] ={
/* HABIT_MOVIE */      "�����n���n��ܰʺA�ݪO",
/* HABIT_COLOR */      "�����n���n��ܱm�⪺�e���A�p�G�������ܩҦ��e���|�ܦ�������",
/* HABIT_NOTE  */      "�W���ɡA��ܬO�_��ܯd���O",
/* HABIT_ALARM */      "��ܬO�_�n�t�Φb���I�o�e�T��",
/* HABIT_BELL  */      "��ܬO�_�n�t�εo�X�����n",
/* HABIT_BOARDLIST */  "�ݪO�C���ɡA�̫e����ܤ峹�Ʃνs��",
/* HABIT_SEELOG */     "�W���ɡA��ܬO�_�[�ݦU������",
/* HABIT_CYCLE */      "�b�峹�C���ɡA�ݨ�̫�@�g�|�۰ʦ^��Ĥ@�g�~��\\Ū",
/* HABIT_FEELING */    "�W���ɡA��ܬO�_�]�w�߱�",
/* HABIT_FROM */       "�W���ɡA��ܬO�_�]�w�G�m",
/* HABIT_LIGHTBAR */   "��ܬO�_�ϥο�����   ",
/* HABIT_ALREADYSET */ "�W���ɡA��ܬO�_�ק�ߦn�]�w",
/* HABIT_NOBRDHELP */  "��ܬݪO�ɡA����ܬݪO����  "
};

#endif

#endif                          /* _HABIT_H_ */