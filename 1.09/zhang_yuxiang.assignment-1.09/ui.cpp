#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#include "core.h"
#include "character.h"
#include "debug.h"
#include "dungeon.h"
#include "move.h"
#include "npc.h"
#include "objectfactory.h"
#include "pc.h"
#include "ui.h"

bool UI::inspect = false;

static void getPositionStr(char buf[50], Character *c)
{
	int i;
	for(i=0; i<50; i++) buf[i]=' ';

	int cx, cy, pcx, pcy;
	c->getLocation(&cx, &cy);
	pc->getLocation(&pcx, &pcy);

	int dx = cx - pcx;
	int dy = cy - pcy;
	
	if (dx>0)
		sprintf(buf +  5, "%02d east             ",  dx);
	else
		sprintf(buf +  5, "%02d west             ", -dx);
	if (dx==0)
		sprintf(buf +  5, "00         ");
	if (dy>0)
		sprintf(buf + 15,          "%02d south  |",  dy);
	else
		sprintf(buf + 15,          "%02d north  |", -dy);
	if (dy==0)
		sprintf(buf + 15, "00        |");
}

static int compareNPC(const void *a, const void *b)
{
	int ax, ay, bx, by, pcx, pcy;

	(*(Character **)a)->getLocation(&ax, &ay);
	(*(Character **)b)->getLocation(&bx, &by);
	pc->getLocation(&pcx, &pcy);
	
	return (ABS((ax-pcx))+ABS((ay-pcy)))
	     - (ABS((bx-pcx))+ABS((by-pcy)));
}

int UI::eListSelect(int select)
{
	int i;
	char buf[50];

	mvprintw(0, 1, "Listing Equipment "
	"(press ESC or Q to quit)");
	
	int color = COLOR_GREEN;	
	attron(COLOR_PAIR(color));
		
	mvprintw(1, 20, "  use A-L or UP/DOWN keys to select ");
	mvprintw(2, 20, "  use T to take off ");
	
	attroff(COLOR_PAIR(color));

	for (i=0; i<12; i++)
	{
		Item *item = pc->equipment->slots[i];
		
		sprintf(buf, "%c %s %s  %-11s", (char)'A'+i, 
		select == i ? "[" : " ",
		select == i ? "]" : " ",
		ObjectFactory::TYPE[pc->equipment->getSlotType(i)].c_str());

		mvprintw(i+1, 2, "%s", buf);

		if (item)
		{
			int color = item->color;		
			attron(COLOR_PAIR(color));
			mvprintw(i+1, 5, "%c", item->symb);
			attroff(COLOR_PAIR(color));
		}
		mvprintw(i+1, 19, "|");
	}
	
	mvprintw(12+1, 1, 
	"------------------+");

	return select;
}

int UI::iListSelect(int select)
{
	dungeon->printDungeon();

	char buf[50];

	mvprintw(0, 1, "Listing Inventory "
	"(press ESC or Q to quit)");
	
	if (inspect && select>=0 && select<(int)pc->inventory->slots.size())
	{
		dungeon->printDungeon();
		
		int color = COLOR_GREEN;	
		attron(COLOR_PAIR(color));
		
		mvprintw(1, 20, "  use I to toggle off inspect ");
		int s = pc->inventory->slots.size();
		Item *item = pc->inventory->slots[select];
		
		mvprintw(s+2, 0, "%s ", item->name.c_str());
		mvprintw(s+3, 0, "%s ", item->desc.c_str());
		attroff(COLOR_PAIR(color));
	}
	else
	{	
		int color = COLOR_GREEN;	
		attron(COLOR_PAIR(color));
		
		mvprintw(1, 20, "  use 0-9 or UP/DOWN keys to select ");
		mvprintw(2, 20, "  use D to drop ");
		mvprintw(3, 20, "  use I to inspect ");
		mvprintw(4, 20, "  use W to wear ");
		mvprintw(5, 20, "  use X to expunge ");

		if (select>=0 && select<(int)pc->inventory->slots.size())
		{		
			int s = 12;
			Item *item = pc->inventory->slots[select];

			mvprintw(s+2, 0, "%s ", item->name.c_str());
			mvprintw(s+3, 0, "   hit = %d ", item->hit);
			mvprintw(s+4, 0, "   dam = %d-%d ", 
					item->dam->min(), item->dam->max());
			mvprintw(s+5, 0, " dodge = %d ", item->dodge);
			mvprintw(s+6, 0, "   def = %d ", item->  def);
			mvprintw(s+7, 0, " speed = %d ", item->speed);
			mvprintw(s+8, 0, "  attr = %d ", item-> attr);
			mvprintw(s+9, 0, "   val = %d ", item->  val);
		}
		attroff(COLOR_PAIR(color));
	}

	for (int i=0; i<(int)pc->inventory->slots.size(); i++)
	{
		Item *item = pc->inventory->slots[i];
		
		sprintf(buf, "%d %s %s  %-11s", i, 
		select == i ? "[" : " ",
		select == i ? "]" : " ",
		ObjectFactory::TYPE[item->type].c_str());

		mvprintw(i+1, 2, "%s", buf);

		int color = item->color;		
		attron(COLOR_PAIR(color));
		mvprintw(i+1, 5, "%c", item->symb);
		attroff(COLOR_PAIR(color));

		mvprintw(i+1, 19, "|");
	}
	
	mvprintw(pc->inventory->slots.size()+1, 1, 
	"------------------+");
	
	return select;
}

static void mListFrom(int start)
{
	int i;
	char buf[50];

	int nummon = dungeon->nummon();

	mvprintw(0, 1, "Listing Monster %d to %d out of %d "
	"(press ESC or Q to quit)", 
	start+1, MIN((start+21), nummon), nummon);

	std::sort(dungeon->npcv.begin(), dungeon->npcv.end(), compareNPC);

	for (i=start; i<MIN((start+21), nummon); i++)
	{
		Character *c = dungeon->npcv[i];
		getPositionStr(buf, c);
		
		mvprintw(i-start+1, 2, "  %s", buf);

		int color = c->getColor();
		
		attron(COLOR_PAIR(color));
		mvprintw(i-start+1, 2, "%c", c->getSymb());
		attroff(COLOR_PAIR(color));

	}
	mvprintw(MIN((start+21), nummon)-start+1, 1, 
	"----------------------------+");
}

static void oListFrom(int start)
{
	int i;
	char buf[50];

	int numobj = dungeon->numobj();

	mvprintw(0, 1, "Listing Object %d to %d out of %d "
	"(press ESC or Q to quit)", 
	start+1, MIN((start+21), numobj), numobj);

	for (i=start; i<MIN((start+21), numobj); i++)
	{
		Item *item = dungeon->itemv[i];
		
		sprintf(buf, "     %-12s", 
		ObjectFactory::TYPE[item->type].c_str());

		mvprintw(i-start+1, 1, "   %s", buf);

		int color = item->color;		
		attron(COLOR_PAIR(color));
		mvprintw(i-start+1, 2, "%c", item->getSymb());
		attroff(COLOR_PAIR(color));

		mvprintw(i-start+1, 21, "|");
	}
	mvprintw(MIN((start+21), numobj)-start+1, 1, 
	"--------------------+");
}

int UI::setInspect(bool inspect)
{
	UI::inspect = inspect;
	
	return 0;
}

int UI::printEquipmentSlots()
{
	mvprintw(23, 44, "Equipment");

	int col=55;
	for (int i=0; i<12; i++)
	{
		Item *item = pc->equipment->slots[i];
		
		mvprintw(22, col+i*2, "%c", (char) 'A'+i);

		mvprintw(23, col+i*2-1, "'");
		
		if (item)
		{
			int color = item->color;
			attron(COLOR_PAIR(color));
			mvprintw(23, col+i*2, "%c", item->symb);
			attroff(COLOR_PAIR(color));
		}
		else
		{
			mvprintw(23, col+i*2, " ");
		}
	}
	mvprintw(23, 78, "'");

	return 0;
}

int UI::printHP()
{
	int colored = int(16 * pc->hp/(double)pc->hpmax()+0.5);

	mvprintw(23, 1, "HP ""                ");

	char buf[50];
	sprintf(buf, "%d/%d""                ", pc->hp, pc->hpmax());
	buf[16] = 0;

	char coloredsubstr[colored + 1];
	memset(coloredsubstr, 0, sizeof(coloredsubstr));
	strncpy(coloredsubstr, buf, colored);
	
	attron(COLOR_PAIR(COLOR_HP));
	mvprintw(23, 4, "%s", coloredsubstr);
	attroff(COLOR_PAIR(COLOR_HP));
	
	if (colored < 16)
		mvprintw(23, 4 + colored, "%s", buf + colored);

	return 0;
}

int UI::printMP()
{
	int colored = int(16 * pc->mp/(double)pc->mpmax()+0.5);

	mvprintw(23, 21, "MP ""                ");

	char buf[50];
	sprintf(buf, "%d/%d""                ", pc->mp, pc->mpmax());
	buf[16] = 0;

	char coloredsubstr[colored + 1];
	memset(coloredsubstr, 0, sizeof(coloredsubstr));
	strncpy(coloredsubstr, buf, colored);
	
	attron(COLOR_PAIR(COLOR_MP));
	mvprintw(23, 24, "%s", coloredsubstr);
	attroff(COLOR_PAIR(COLOR_MP));
	
	if (colored < 16)
		mvprintw(23, 24 + colored, "%s", buf + colored);

	return 0;
}

int UI::printMonsterHP()
{
	NPC *att = pc->attacking;

	if (!att)
	{
		clearRow(0);
		return -1;
	}

	int colored = int(16 * att->hp/(double)(att->hpmax())+0.5);

	mvprintw(0, 1, "HP ""                ");

	char buf[50];
	sprintf(buf, "%d/%d""                ", att->hp, att->hpmax());
	buf[16] = 0;

	char coloredsubstr[colored + 1];
	memset(coloredsubstr, 0, sizeof(coloredsubstr));
	strncpy(coloredsubstr, buf, colored);
	
	attron(COLOR_PAIR(COLOR_HP));
	mvprintw(0, 4, "%s", coloredsubstr);
	attroff(COLOR_PAIR(COLOR_HP));
	
	if (colored < 16)
		mvprintw(0, 4 + colored, "%s", buf + colored);

	// monster info	
	mvprintw(0, 20, " <- ");

	attron(COLOR_PAIR(att->getColor()));
	mvprintw(0, 24, "%c", att->getSymb());
	attroff(COLOR_PAIR(att->getColor()));

	mvprintw(0, 26, "targeted");

	return 0;
}

int UI::clearRow(int row)
{
	int i;
	for(i=0; i<80; i++)
		mvaddch(row, i, ' ');
	
	return 0;
}

int UI::reprint()
{
	dungeon->printDungeon();

	printHP();
	printMP();

	printEquipmentSlots();
	
	return 0;
}

int UI::eList()
{
	refresh();
	
	int select = 0;

	// default select a non-empty slot
	for (int i=0; i<12; i++)
		if (pc->equipment->slots[i])
		{
			select = i;
			break;
		}

	eListSelect(select);

	while (1)
	{
		int ch = getch();
		
		int quit = 0;
		
		int index = -1;
		if (ch>='A' && ch<='L') index = ch-'A';
		if (ch>='a' && ch<='l') index = ch-'a';
		if (ch>='0' && ch<='9') index = ch-'0';

		if (index >= 0)
		{
			eListSelect(select = index);
			continue;
		}

		switch (ch)
		{
			case 'Q':
			case 'q':
			case 27: // ESC
				quit = 1;
				break;
			case KEY_DOWN:
				select = (select + 1) % 12; 
				eListSelect(select);
				refresh();
				break;
			case KEY_UP:
				select = (select - 1 + 12) % 12;
				eListSelect(select);
				refresh();
				break;
			case 'T':
			case 't':
				pc->takeOffItem(select);
				eListSelect(select);
				printEquipmentSlots();
				refresh();
				break;
			default:
				if (pc->equipment->isEmpty())
					quit = 1;
				break;
		}
		if (quit)
			break;
	}

	clearRow(0);
	clearRow(22);
	
	return 0;
}

int UI::iList()
{
	refresh();

	int select = 0;

	iListSelect(select);

	while (1)
	{
		int ch = getch();
		
		int quit = 0;
		
		switch(ch)
		{
			case 'Q':
			case 'q':
			case 27: // ESC
				quit = 1;
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
			case '0':
				if (ch-'0' >= (int)pc->inventory->slots.size())
					break;
				else
					select = ch-'0';
				iListSelect(select);
				refresh();
				break;
			case KEY_DOWN:
				if (pc->inventory->slots.empty()) 
				{
					break;
				}
				select = (select + 1) % 
					pc->inventory->slots.size();
				iListSelect(select);
				refresh();
				break;
			case KEY_UP:
				if (pc->inventory->slots.empty()) 
				{
					break; 
				}
				select = (select - 1 + pc->inventory->slots.size()) % 
					pc->inventory->slots.size();
				iListSelect(select);
				refresh();
				break;
			case 'D':
			case 'd':
				if (pc->inventory->slots.empty()) 
				{
					break;
				}
				pc->dropItem(select);
				reprint();
				if (select>=(int)pc->inventory->slots.size())
					iListSelect(--select);
				else
					iListSelect(select);
				refresh();
				break;
			case 'I':
			case 'i':
				if (pc->inventory->slots.empty()) 
				{
					break;
				}
				reprint();
				setInspect(!inspect);
				iListSelect(select);
				refresh();
				break;
			case 'X':
			case 'x':
				if (pc->inventory->slots.empty()) 
				{
					break;
				}
				pc->inventory->expunge(select);	
				reprint();
				if (select>=(int)pc->inventory->slots.size())
					iListSelect(--select);
				else
					iListSelect(select);
				refresh();
				break;
			case 'W':
			case 'w':
				if (pc->inventory->slots.empty())
				{
					break;
				}
				pc->wearItem(select);
				reprint();
				if (select>=(int)pc->inventory->slots.size())
					iListSelect(--select);
				else
					iListSelect(select);
				refresh();
				break;
			case 'C':
			case 'c':
				sList();
				reprint();
				iListSelect(select);
				break;
			default:
				if (pc->inventory->slots.empty())
				//	quit = 1;
				break;
		}
		if (quit)
			break;
	}
	
	int i;
	for(i=0; i<80; i++)
	{
		mvprintw(0,  i, " ");
		mvprintw(22, i, " ");
	}

	return 0;
}

int UI::mList()
{
	int from = 0;

	mListFrom(from);
	refresh();

	while(1)
	{
		int ch = getch();
		
		int quit = 0;
		
		switch(ch)
		{
			case 'Q':
			case 'q':
			case 27: // ESC
				quit = 1;
				break;
			case KEY_DOWN:
				if (from>21-nummon && from<nummon-21) from++;
				mListFrom(from);
				refresh();
				break;
			case KEY_UP:
				from = MAX((from-1), 0);
				mListFrom(from);
				refresh();
				break;
		}
		if (quit)
			break;
	}
	
	int i;
	for(i=0; i<80; i++)
	{
		mvprintw(0,  i, " ");
		mvprintw(22, i, " ");
	}

	return 0;
}

int UI::oList()
{
	int from = 0;

	oListFrom(from);
	refresh();

	while (1)
	{
		int ch = getch();
		
		int quit = 0;
		
		switch(ch)
		{
			case 'Q':
			case 'q':
			case 27: // ESC
				quit = 1;
				break;
			case KEY_DOWN:
				if (from>21-nummon && from<nummon-21) from++;
				oListFrom(from);
				refresh();
				break;
			case KEY_UP:
				from = MAX((from-1), 0);
				oListFrom(from);
				refresh();
				break;
		}
		if (quit)
			break;
	}
	
	clearRow(0);
	clearRow(22);
	
	return 0;
}

int UI::sList()
{
	int color = COLOR_CYAN;	
	attron(COLOR_PAIR(color));
	
	mvprintw(19, 21, "B: Blast nearby area   cost: 250 ");
	mvprintw(20, 21, "H: Heal PC by 100      cost: 100 ");
	mvprintw(21, 21, "T: Teleport            cost: 500 ");

	attroff(COLOR_PAIR(color));

	int ch = getch(), cost;

	switch (ch)
	{
		case 'Q':
		case 'q':
		case 27: // ESC
			break;
		case 'B':
		case 'b':
			if (pc->mp < (cost = 250))
				break;
			for (int r=pcy-1; r<=pcy+1; r++)
			{
				for (int c=pcx-1; c<=pcx+1; c++)
				{
					if (r>=0 && r<21 &&
						c>=0 && c<80 && 
						dungeon->hmap[r][c]!=IMMUTABLE)
					{
						if (dungeon->hmap[r][c]>0)
						{
							dungeon->hmap[r][c]=0;
							dungeon->tmap[r][c]=PATH;
						}

						if (dungeon->cmap[r][c]&&!
							dungeon->cmap[r][c]->isPC())
							dungeon->cmap[r][c]->hp/=10;
					}
				}
			}
			pc->mp -= cost;
			pc->setLocation(pcx, pcy); // force update seen dungeon
			dungeon->printDungeon();
			refresh();	
			break;
		case 'H':
		case 'h':
			if (pc->mp < (cost = 100))
				break;
			pc->hp += pc->hpmax() * 0.1;
			if (pc->hp > pc->hpmax())
				pc->hp = pc->hpmax();
			pc->mp -= cost;
			UI::printHP();
			UI::printMP();
			break;
		case 'T':
		case 't':
			if (pc->mp < (cost = 500))
				break;
			while (1)
			{
				int r = rand() % 21;
				int c = rand() % 80;
				if (!dungeon->hmap[r][c] && !dungeon->cmap[r][c])
				{
					Move::move(pc, c, r);
					break;
				}
			}
			pc->mp -= 500;
			UI::printMP();
			break;
	}
	
	return 0;
}
