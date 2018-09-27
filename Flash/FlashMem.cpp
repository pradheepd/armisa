#include "systemc.h"
#include "PeripheralDefs.h"
#include "libelf.h"
#include "stdio.h"
# include <gelf.h>

static unsigned char *FData;

bool FReadCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    memcpy(data,&FData[addr-FLASH_START_ADDR],datalen);

    return true;
}

bool FWriteCB(unsigned int addr, unsigned char *data, unsigned int datalen)
{
    memcpy(&FData[addr-FLASH_START_ADDR],data, datalen);

    return true;
}


FLASH::FLASH(const char *name): sc_module(name), slaveb("Flash"){

    slaveb.register_Slave_Addr(FLASH_START_ADDR, FLASH_START_ADDR+FLASH_SIZE);
    slaveb.register_read_cb(FReadCB);
    slaveb.register_write_cb(FWriteCB);

    FData = (unsigned char *)malloc(FLASH_SIZE);

    int fd ;
	Elf * e ;
	char *p , pc [4* sizeof ( char )];
	Elf_Scn * scn ;
	Elf_Data * data ;
	GElf_Shdr shdr ;
	size_t n, shstrndx, sz ;

    if ( elf_version ( EV_CURRENT ) == EV_NONE )
		printf("ELF library initialization failed: %s", elf_errmsg (-1));

    if ( elf_getshdrstrndx (e , &shstrndx ) != 0)
		printf("elf_getshdrstrndx() failed : %s .", elf_errmsg (-1));

    scn = NULL ;

	while (( scn = elf_nextscn (e , scn )) != NULL ) {

		if ( gelf_getshdr ( scn , &shdr ) != &shdr )
			printf("getshdr() failed : %s." , elf_errmsg ( -1));

		data = NULL;

		data = elf_getdata (scn, data);

		if(shdr.sh_addr != 0){
			FWriteCB(shdr.sh_addr, (unsigned char *)data->d_buf, data->d_size);
		}

		free(data);

		//printf("Section %-4.4jd %s\n", ( uintmax_t )	elf_ndxscn ( scn ) , name );
        
	}

    putchar('\n');
	elf_end ( e );
	close ( fd );
}