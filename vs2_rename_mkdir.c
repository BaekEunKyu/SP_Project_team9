#include <ncurses.h>
#include <menu.h>
#include <panel.h>
#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <dirent.h>
#include <sys/stat.h>

chtype volatile main_ipt_ch; //getch ��ȯ���� ��


void EntityListWinDisplay(WINDOW *entity_list_win);
void DirectoryWinDisplay(WINDOW * directory_dispaly_win,const char * absDIRstr);
void getCurrDir(char **absDIRstr, long * maxpath);
void print_in_middle(WINDOW *win,int starty,int startx, int width, char *string,chtype color);
int print_current_entity(const char *entity_name, const int pce_mode, WINDOW *entity_detail_display_win);
//���丮 ���� ����
void Dlg_Mkdir(WINDOW *dlg_win_mkdir,const char *curDirName, char *usrIptName);
//�̸� ���� ���� 
void Dlg_Rename(WINDOW *dlg_win_rename, const char *prevName, char *newName);
//����Ű ǥ�� ������
void ShortCutWinDisplay(WINDOW *shortcut_display_win);
//�޸� �Ҵ� ���� 
void freeDynamicMem(MENU **my_menu, ITEM ***my_items, struct dirent ***d_ptrArray 
            , char ***rtn_entity_size_chr
            , char **absDIRstr
            ,const int *n_choices)
{
    int i;

    unpost_menu(*my_menu);
    free_menu(*my_menu);
    *my_menu=(MENU*)NULL;

    for(i=0; i<*n_choices; i++)
    {   
        if( (free_item((*my_items)[i]))!=E_OK)
        {
            endwin();
            perror("free_item()���� - ��ġ: freeDynamicMem() ");
            exit(-100);
        }

        (*my_items)[i]=(ITEM *)NULL;
    }
    (*my_items)=(ITEM **)NULL;

    for(i=0; i<*n_choices; i++)
    {
        free( (*d_ptrArray)[i] );
        (*d_ptrArray)[i]=(struct dirent *)NULL;
    }
    free(*d_ptrArray);  // scandir()�� ������� �� ���� �޸� ����.
    (*d_ptrArray)=(struct dirent **)NULL;

    for(i=0; i<*n_choices; i++){
        free((*rtn_entity_size_chr)[i]);
        (*rtn_entity_size_chr)[i]=(char *)NULL;
    }
    free(*rtn_entity_size_chr);
    *rtn_entity_size_chr=(char **)NULL;

    if(absDIRstr!=(char **)NULL) // �޸� �Ҵ������� �ʿ���� ���� �Լ��� NULL�� �����ϰ�
    {                // ó���ο��� if�� ����.
        free(*absDIRstr); // ���� ��� ���ڿ��� ������.
        *absDIRstr=(char *)NULL;
    }

}   // ���� �޸𸮸� ����ϴ� ������ ������ ����ִ� �޸𸮵��� 
    // �����Ӱ�~~~




int main(){
	//�ѱ��� �Է¹ޱ� ���� ����.
	setlocale(LC_ALL, "ko_KR.utf8");
	setenv("NCURSES_NO_UTF8_ACS", "1", 0);
	
	PANEL *my_panels[3];
	PANEL *top;

	WINDOW *entity_list_win, *shortcut_display_win, *entity_detail_display_win,
			 *directory_display_win, *dlg_win_mkdir, *dlg_win_rename, *dlg_win_chmod;//�������������
	
	ITEM **my_items;  //ITEM�� ���� ������ �迭�� ù ��° �ּҸ� ����
	MENU *my_menu;
	int n_choices; 	//�޴� �������� ������ ����
	struct dirent **d_ptrArray;
	char **rtn_entity_size_chr;
	char * absDIRstr;
	long maxpath;

	entity_list_win=(WINDOW *)NULL;		//
	shortcut_display_win=(WINDOW *)NULL;	//�ϴܺ� ����� ����
	directory_display_win=(WINDOW *)NULL;			//
	entity_detail_display_win = (WINDOW *)NULL;	//
	dlg_win_mkdir=(WINDOW *)NULL;
	absDIRstr=(char *)NULL;
	my_items=(ITEM **)NULL;
	rtn_entity_size_chr=(char **)NULL;
	my_menu=(MENU *)NULL;
	
	char usrIptName[100];  	//����ڰ� �Է��� ���ϸ� �Ǵ� ���丮�� ����

	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	ESCDELAY = 0;

	getCurrDir(&absDIRstr, &maxpath);

	refresh();

	directory_display_win=newwin(3,COLS,0,0);
	entity_list_win=newwin(LINES-7, COLS, 3, 0);
	entity_detail_display_win = newwin(1,COLS, LINES-4, 0);
	shortcut_display_win = newwin(3, COLS, LINES-3, 0);

	DirectoryWinDisplay(directory_display_win,absDIRstr);
	EntityListWinDisplay(entity_list_win);
	ShortCutWinDisplay(shortcut_display_win);

	while(1){
		if(main_ipt_ch!=0x03)
		if(main_ipt_ch!=KEY_F(12)){
			main_ipt_ch=wgetch(entity_list_win);
		}
		if(main_ipt_ch==KEY_F(12)){
			break;
		}

		switch(main_ipt_ch)
		{
			case 0x0b: 	//mkdir ���
				Dlg_Mkdir(dlg_win_mkdir,absDIRstr,&usrIptName[0]);
				if(usrIptName[0]!='\0') //��� ��ư ������ ��
       				{
                // ���丮�� �ٲ�� �̺�Ʈ�� ���������� �޴� �����.
                freeDynamicMem(&my_menu, &my_items, &d_ptrArray
                            , &rtn_entity_size_chr
                            , (char **)NULL //&absDIRstr
                            , &n_choices);
                // �޸� ���� ��..

                //  getCurrDir(&absDIRstr, &maxpath);
                //  DirectoryWinDisplay(directory_display_win, absDIRstr);

                // ���� ���͸� ������.
                // absDIRstr�� ���� �޸𸮰� �Ҵ�ǹǷ� ���߿� ���� ���ϱ�~~

                // Initialize items
               // initMenu(&n_choices,  &d_ptrArray, &my_items
                 //       ,&rtn_entity_size_chr
                   //     ,&entity_list_win
                     //   ,&my_menu ,absDIRstr);
                // �޴� ����� ��
            }
			top=(PANEL *)panel_userptr(top);
			top_panel(top);

			update_panels();
			mvwaddch(shortcut_display_win,1,COLS-2,'K'|A_BOLD|A_BLINK|COLOR_PAIR(4));
			break;

			 case 0x0e:  // Rename ���
                {
                    ITEM *curItem;
                    curItem = current_item(my_menu);
                    char *entity_name = (char *)item_name(curItem);

                    Dlg_Rename(dlg_win_rename, entity_name, &usrIptName[0]);

                    if(usrIptName[0]!='\0')
                    {
                        // ���丮�� �ٲ�� �̺�Ʈ�� ��
                        // �������� �޴� �����.
                        freeDynamicMem(&my_menu, &my_items, &d_ptrArray
                                , &rtn_entity_size_chr
                                , (char **)NULL //&absDIRstr
                                , &n_choices);
                        // �޸� ���� ��..

                   
                    // Initialize items
                   // initMenu(&n_choices,  &d_ptrArray, &my_items
                     //       ,&rtn_entity_size_chr
                       //     ,&entity_list_win
                       //     ,&my_menu ,absDIRstr);
                    // �޴� ����� ��
                    }
                }

                top = (PANEL *)panel_userptr(top);
                top_panel(top);
                update_panels();

                mvwaddch(shortcut_display_win, 1 , COLS-2, 'N'
                    | A_BOLD | A_BLINK | COLOR_PAIR(4) );
                break;
{   // ���� �� ����:/
            ITEM *curItem; 
            curItem = current_item(my_menu); // ���� ���밡 ����Ű�� �ִ� ������ �ҷ���.

            wmove(entity_detail_display_win, 0, 2);
            wclrtoeol(entity_detail_display_win);

            wprintw(entity_detail_display_win, "[%s] ", item_name(curItem));
        
            //print_current_entity(item_name(curItem), 0, entity_detail_display_win);
            
            wrefresh(entity_detail_display_win);

            wrefresh(shortcut_display_win);
            
            pos_menu_cursor(my_menu); // ��ü ���� ������� �Ű��� Ŀ���� �ٽ� �޴���...
        
        }   // ���� �� ��.   
      //-------------------------------------------------------------------------
        //wrefresh(entity_list_win);
        
        top = my_panels[0];  // entity_list_win�� �� ���� ǥ�õǰ�                             
        top_panel(top);                                                   

        update_panels();            
        doupdate();
		}

	wrefresh(entity_list_win);
	wrefresh(entity_detail_display_win);
	wrefresh(shortcut_display_win);
	}


	//�޸� ����
	delwin(directory_display_win);
	delwin(entity_list_win);
	delwin(entity_detail_display_win);
	delwin(shortcut_display_win);
	delwin(dlg_win_mkdir);
	delwin(dlg_win_rename);
	delwin(dlg_win_chmod);

	endwin();
	
	unsetenv("NCURSES_NO_UTF8_ACS");
	
	return 0;
}

void DirectoryWinDisplay(WINDOW *directory_display_win, const char *absDIRstr){
	box(directory_display_win,0,0);

	wmove(directory_display_win,1,2);
	wprintw(directory_display_win,"���� ���丮 : ");

 	wclrtoeol(directory_display_win);
	wattron(directory_display_win, A_BOLD | COLOR_PAIR(2));
	wprintw(directory_display_win, "%s",absDIRstr);
	wattroff(directory_display_win ,A_BOLD | COLOR_PAIR(2));
	mvwaddch(directory_display_win, 1, COLS-1, ACS_VLINE);

	wrefresh(directory_display_win);
}
void EntityListWinDisplay(WINDOW *entity_list_win)
{
	int max_x, max_y;

    // ��ü ����Ʈ �޴��� ���� ������ ����.

	getmaxyx(entity_list_win, max_y, max_x);    // �����찡 ���� �� �ִ� �ִ� ��ǥ
                            // �ᱹ ������ ���ϴ� ��ǥ.
	box(entity_list_win, 0, 0);
	print_in_middle(entity_list_win, 1, 0, max_x, "��ü �̸� / ũ��", COLOR_PAIR(1)|A_BOLD);

    // ��� ������ ESCŰ ���� ����
	mvwaddch(entity_list_win,0, max_x-14, ACS_TTEE);    // �� ���� ����
	mvwaddch(entity_list_win, 1, max_x-14, ACS_VLINE);
	mvwprintw(entity_list_win, 1, max_x-12, "����:");

	wattron(entity_list_win, A_BOLD | COLOR_PAIR(4));
	wprintw(entity_list_win, "[F12]");
	wattroff(entity_list_win, A_BOLD | COLOR_PAIR(4));

    // window�ȿ� ��-----�� ����
	mvwaddch(entity_list_win, 2, 0, ACS_LTEE);
	mvwhline(entity_list_win, 2, 1, ACS_HLINE, max_x-2);
	mvwaddch(entity_list_win, 2, max_x-1, ACS_RTEE);

	mvwaddch(entity_list_win, 2, max_x-14, ACS_BTEE);   // ESC������ ��  ���� ����

	wrefresh(entity_list_win);
} // EntityListWinDisplay() end
void ShortCutWinDisplay(WINDOW *shortcut_display_win)
{
	int win_x, win_y;

	char botLabelCtrl[] = {"Ctrl+"};
	char botLabelCopy[] = {"Copy( )"};
	char botLabelMove[] = {"Move( )"};
	char botLabelPaste[] = {"Paste( )"};
	char botLabelRename[] = {"Rename( )"};
	char botLabelChmod[] = {"Chmod( )"};
	char botLabelMkdir[] = {"Mkdir( )"};
	char botLabelDel[] = {"Delete( )"};
	char botLabelFMove[] = {"[ ]"};

	char botLabelKey[]={'c','o','v','n','h','k','d','/'};

	char *botLabel[9]={&botLabelCtrl[0],&botLabelCopy[0],&botLabelMove[0],
							 &botLabelPaste[0],&botLabelRename[0],&botLabelChmod[0],&botLabelMkdir[0],
							 &botLabelDel[0],&botLabelFMove[0]};

	int label_x;
	int i;

	getbegyx(shortcut_display_win, win_y, win_x);

	label_x = win_x+2;
	box(shortcut_display_win, 0, 0);

	wattron(shortcut_display_win, COLOR_PAIR(3)|A_BOLD);

	mvwaddstr(shortcut_display_win, 1, label_x, botLabel[0]);
	for(i = 1; i<9; i++){
		
		if(i==8)continue;
		label_x=(label_x+strlen(botLabel[i-1])+1);

		mvwaddstr(shortcut_display_win, 1, label_x, botLabel[i]);
		mvwaddch(shortcut_display_win, 1, label_x+strlen(botLabel[i])-2, botLabelKey[i-1]|COLOR_PAIR(4)|A_BOLD|A_UNDERLINE);
	}

	wattroff(shortcut_display_win, COLOR_PAIR(3)|A_BOLD);
	wrefresh(shortcut_display_win);
}

void getCurrDir(char **absDIRstr, long *maxpath)
{
	if((*maxpath = pathconf(".",_PC_PATH_MAX))==-1)
	{
		endwin();
		fprintf(stderr,"��� �̸� ���̸� �����ϴµ� ����\n");
		perror(" ");
		exit(-13);
	}
	if( (*absDIRstr = (char *)malloc(*maxpath)) == NULL )
	{
		endwin();
	        perror("����̸��� ���� ������ �Ҵ��ϴµ� �����Ͽ����ϴ�.");
	        exit(-14);
	}


	if( getcwd(*absDIRstr, *maxpath) == NULL )
	{
        	endwin();
	        fprintf(stderr, "���� �۾� ���丮�� ���� �����ϴ�.\n");
        	perror(" ");
	        exit(-15);
   	}
}

void print_in_middle(WINDOW *win, int starty, int startx, int width, char *string, chtype color)
{
	int length, x, y;
	float temp;

	if(win == NULL)
		win = stdscr;
	getyx(win, y, x);
	if(startx != 0)
        	x = startx;
	if(starty != 0)
	        y = starty;
	if(width == 0)
	        width=COLS;

	length = strlen(string);
	temp = (width - length)/2;
	x = startx + (int)temp;
	wattron(win, color);
	mvwprintw(win, y, x, "%s", string);
	wattroff(win, color);

}


void Dlg_Mkdir(WINDOW *dlg_win_mkdir, const char *curDirName, char *usrIptName)
{
    int y, x, max_y, max_x;
    int i;
    int attr_style;

    struct stat statbuf; // ���ϸ�� �������� ������ stat���� ����ü ����.
    mode_t perm=0000000;    // ���� �ʱⰪ

    chtype ipt;

    ipt=0;
    usrIptName[0]='\0';
    keypad(dlg_win_mkdir, TRUE); // initscr()������ �� �Լ� ȣ���� �ǹ̾���.

    getmaxyx(dlg_win_mkdir, max_y, max_x);  //max_y�� max_x�� �������� ũ�⸦ ��ġ 

    box( dlg_win_mkdir, 0, 0);	//�簢���� �׸� 
    
    wattron(dlg_win_mkdir, A_BOLD); //Ư�� �����쿡 ������ �Ӽ��� ���� 
    mvwprintw(dlg_win_mkdir, 0, 5, " ���丮 ���� (Mkdir)"); //Ư�� ��ġ�� ���ڿ� ��� 
    wattroff(dlg_win_mkdir, A_BOLD); //�����ߴ� �Ӽ��� ���� 

    wattron(dlg_win_mkdir, A_BOLD | COLOR_PAIR(8)); //����� �۾�ü 
    mvwprintw(dlg_win_mkdir, 2, 3, " ���� ���丮      ");

    mvwprintw(dlg_win_mkdir, 4, 3, " ���� ���͸� �̸� ");

    mvwprintw(dlg_win_mkdir, 6, 24,"  Ȯ ��  "); // ��ư 1

    mvwprintw(dlg_win_mkdir, 6, 40, "  �� ��  "); // ��ư 2
    wattroff(dlg_win_mkdir, A_BOLD | COLOR_PAIR(8));


    attr_style = A_BOLD | COLOR_PAIR(7);
    wmove(dlg_win_mkdir, 2, 23);     //����� ���� ������� ������ ���� Ŀ�� �̵� 

    wattron(dlg_win_mkdir, attr_style );  //

    for(i=23;i<max_x-2;i++)  	//�־��� ��� �� ��ġ�� ' ' ��� 
    {
        mvwaddch(dlg_win_mkdir,2, i, ' ');
        mvwaddch(dlg_win_mkdir,4, i,' ');
    }
    mvwprintw(dlg_win_mkdir, 2, 24, curDirName);  //���� ���丮 ���

    echo(); // ������� �Է��� ȭ�鿡 ǥ���ϱ� ����.
    keypad(dlg_win_mkdir, FALSE);  
    mvwgetnstr(dlg_win_mkdir, 4, 24, usrIptName, max_x-28);//������ ũ�⸸ŭ �Է¹��� 
    keypad(dlg_win_mkdir, TRUE); //Ű�� �Է¹��� 
    noecho(); //������� �Է��� ȭ�鿡 ǥ������ ����. 

    wattroff(dlg_win_mkdir, attr_style );

    mvwchgat(dlg_win_mkdir, 6, 24 ,9, A_BOLD|A_UNDERLINE, 8, NULL ); //���ڵ��� �Ӽ� ���� 
    mvwchgat(dlg_win_mkdir, 6, 40, 9, A_NORMAL, 8 ,NULL); //���ڵ��� �Ӽ� ���� 
    wmove(dlg_win_mkdir, 6, 24);  

    while( ( ipt=wgetch(dlg_win_mkdir) )!='\n')	//�Էµ� ���ڰ� ���Ͱ� �ƴҰ��
    {
        switch (ipt)    
        {
            case KEY_LEFT :     
                wmove(dlg_win_mkdir, 6, 24);
                wchgat(dlg_win_mkdir, 9, A_BOLD|A_UNDERLINE, 8, NULL );
                mvwchgat(dlg_win_mkdir, 6, 40, 9, A_NORMAL, 8 ,NULL);
                wmove(dlg_win_mkdir, 6, 24);
                break;
        
            case KEY_RIGHT:
                wmove(dlg_win_mkdir, 6, 40);
                wchgat(dlg_win_mkdir, 9, A_BOLD|A_UNDERLINE, 8, NULL );
                mvwchgat(dlg_win_mkdir, 6, 24, 9, A_NORMAL, 8 ,NULL);
                wmove(dlg_win_mkdir, 6, 40);
                break;

            case KEY_UP:
                mvwchgat(dlg_win_mkdir, 6, 24, 9, A_NORMAL, 8 ,NULL);
                mvwchgat(dlg_win_mkdir, 6, 40, 9, A_NORMAL, 8 ,NULL);

                wattron(dlg_win_mkdir, attr_style );
                for(i=23;i<max_x-2;i++)
                {
                    mvwaddch(dlg_win_mkdir,4, i,' ');
                }
                echo(); // ������� �Է��� ȭ�鿡 ǥ���ϱ� ����.
                keypad(dlg_win_mkdir, FALSE);
                mvwgetnstr(dlg_win_mkdir, 4, 24, usrIptName, max_x-28);
                keypad(dlg_win_mkdir, TRUE);

                noecho();
                wattroff(dlg_win_mkdir, attr_style );
                wmove(dlg_win_mkdir, 6, 24);
                wchgat(dlg_win_mkdir, 9, A_BOLD|A_UNDERLINE, 8, NULL );

                break;
        }
        wrefresh(dlg_win_mkdir); //������ ������ ȭ������ ���� 
    }

    getyx(dlg_win_mkdir, y, x); // ���� Ŀ���� ��ǥ������.
    // ����ڰ� ������ ��ǥ�� ��ġ�� [Ȯ��]���� [���]���� �����ϱ����.
    if( (x > 39 && x < 50) || usrIptName[0]=='\0') // ��� ����.
    {
        usrIptName[0]='\0';
    } else{
        // Ȯ�� �̹Ƿ� ���丮 ���� �ڵ� ����.
    
        if(lstat(curDirName, &statbuf)==-1)
        {
            perror(" ");
            perm=0000700;
        }
        perm = statbuf.st_mode; // �ٷ� �������丮�� ������ �״�� �̾�޵��� ����.

        mkdir(usrIptName, perm); // ���丮 ����. 
    }

}   // Dlg_Mkdir() end

void Dlg_Rename(WINDOW *dlg_win_rename, const char *prevName, char *newName)
{
    int y, x, max_y, max_x;
    int i;
    int attr_style;
    chtype ipt;

    newName[0]='\0';
    keypad(dlg_win_rename, TRUE); 

    getmaxyx(dlg_win_rename, max_y, max_x);

    box( dlg_win_rename, 0, 0 );

    wattron(dlg_win_rename, A_BOLD);
    mvwprintw(dlg_win_rename, 0, 5, " �̸� ���� (Rename)");
    wattroff(dlg_win_rename, A_BOLD);

    attr_style = A_BOLD | COLOR_PAIR(8);
    wattron(dlg_win_rename, attr_style );
    mvwprintw(dlg_win_rename, 2, 3, " ���� �̸�   ");

    mvwprintw(dlg_win_rename, 4, 3, " ���ο� �̸� ");

    mvwprintw(dlg_win_rename, 6, 24,"  Ȯ ��  "); 

    mvwprintw(dlg_win_rename, 6, 40, "  �� ��  "); 
    wattroff(dlg_win_rename, attr_style );


    attr_style = A_BOLD | COLOR_PAIR(7);
    wmove(dlg_win_rename, 2, 16);

    wattron(dlg_win_rename, attr_style );

    for(i=16;i<max_x-2;i++)
    {
        mvwaddch(dlg_win_rename,2, i, ' ');
        mvwaddch(dlg_win_rename,4, i,' ');
    }
    mvwprintw(dlg_win_rename, 2, 17, prevName);

    echo(); 
    
    


    
    ipt=0, i=0;
    wmove(dlg_win_rename, 4, 17);
    getyx(dlg_win_rename, y, x);
    while( (ipt=wgetch(dlg_win_rename))!='\n' && i < max_x-28 )
    {
        if(ipt==KEY_BACKSPACE)
        {
            if(i>0)
            {               
                wmove(dlg_win_rename, y, --x);
                waddch(dlg_win_rename, ' ');
                newName[i--]='\0';
            }

            wmove(dlg_win_rename, y, x);
            continue;
        }

        if(ipt<33 || ipt>126) 
        {
            wmove(dlg_win_rename, y, x);
            continue;           
        }


        newName[i++]=ipt;   
        getyx(dlg_win_rename, y, x);
    }
    newName[i]='\0';
    
    
    noecho();

    wattroff(dlg_win_rename, attr_style );


    mvwchgat(dlg_win_rename, 6, 24, 9, A_BOLD|A_UNDERLINE, 8, NULL );
    mvwchgat(dlg_win_rename, 6, 40, 9, A_NORMAL, 8 ,NULL);

    wmove(dlg_win_rename, 6, 24);

    ipt=0;
    while( ( ipt=wgetch(dlg_win_rename) )!='\n')
    {
        switch (ipt)    
        {
            case KEY_LEFT :     
                wmove(dlg_win_rename, 6, 24);
                wchgat(dlg_win_rename, 9, A_BOLD|A_UNDERLINE, 8, NULL );
                mvwchgat(dlg_win_rename, 6, 40, 9, A_NORMAL, 8 ,NULL);
                wmove(dlg_win_rename, 6, 24);
                break;
        
            case KEY_RIGHT:
                wmove(dlg_win_rename, 6, 40);
                wchgat(dlg_win_rename, 9, A_BOLD|A_UNDERLINE, 8, NULL );
                mvwchgat(dlg_win_rename, 6, 24, 9, A_NORMAL, 8 ,NULL);
                wmove(dlg_win_rename, 6, 40);

                break;
            case KEY_UP:     
                mvwchgat(dlg_win_rename, 6, 24, 9, A_NORMAL, 8 ,NULL);
                mvwchgat(dlg_win_rename, 6, 40, 9, A_NORMAL, 8 ,NULL);

                wattron(dlg_win_rename, attr_style );
                for(i=16;i<max_x-2;i++)
                {       
                    mvwaddch(dlg_win_rename,4, i,' ');
                }
                echo(); 
                
                keypad(dlg_win_rename, FALSE);
                mvwgetnstr(dlg_win_rename, 4, 17, newName, max_x-28);
                keypad(dlg_win_rename, FALSE);

                noecho();
                wattroff(dlg_win_rename, attr_style );
                wmove(dlg_win_rename, 6, 24);
                wchgat(dlg_win_rename, 9, A_BOLD|A_UNDERLINE, 8, NULL );


        }
        
        wrefresh(dlg_win_rename);
    }

    getyx(dlg_win_rename, y, x); 

    if( (x > 39 && x < 50) || newName[0]=='\0') 
    {
        newName[0]='\0';
    } else{
       
        rename(prevName, newName);
    }

} 