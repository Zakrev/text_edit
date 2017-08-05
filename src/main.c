#include "../headers/main.h"

int main(int args, char ** arg)
{
	FileText ftext;
	FilePos pos;
	char buff[512];
	
	if(0 != init_FileText(&ftext, (const unsigned char *)"\n", 1))
	        return -1;
	        
	if(0 != read_from_file_FileText(&ftext, arg[1]))
	        return -1;
	        
	if(0 != fill_pos_FileText(&ftext, &pos, NULL, 1, 0, ftext.esize))
	        return -1;
	        
	if(0 >= get_data_by_pos_FileText(&ftext, &pos, (unsigned char *)buff, 512))
	        return -1;
	
	if(0 != fill_pos_FileText(&ftext, &pos, NULL, 1, 0, 0))
	        return -1;
	
	if(0 != edit_lines_by_pos_FileText(&ftext, &pos, (const unsigned char *)buff, 512))
	        return -1;
	        
	if(0 != write_to_file_FileText(&ftext, "tmp_file"))
	        return -1;

	return close_file_FileText(&ftext);
}