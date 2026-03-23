#include <ata.h>
#include <utilities.h>
#include <fat16.h>

void init_bpb(){
	print_num(sizeof(struct BootSector));
}
