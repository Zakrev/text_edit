#include "term.h"

int main(int args, char ** arg)
{
	teView view;

	if(0 != init_view_terminal(&view)){
		fprintf(stderr, "fault init program\n");
		return 1;
	}
	run_editor_terminal(&view, args, arg);
	close_view_terminal(&view);
	
	return 0;
}