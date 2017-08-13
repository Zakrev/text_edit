#define DBG_LVL 1
#include "../debug.h"
#include "term.h"

int main(int args, char ** arg)
{
	teView view;

	if(0 != init_view_terminal(&view))
		return 1;
	switch(args){
		case 2:
			run_editor_terminal(&view);
			break;
		default:
			PERR("Need arguments: file_name");
	}

	close_view_terminal(&view);
	
	return 0;
}