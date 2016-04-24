

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
:100000007401F5A4C280110ED280110E80F67D001D
:0D0010007EC87FC80000DFFCDEF8DDF422B2
:00000001FF
*/

#define ROMMASK 0xFFFF
unsigned char rom[ROMMASK+1];
char filedata[0x80000];
unsigned int filelen;
unsigned int fileoff;


char newline[1024];
unsigned int data[1024];
char xstring[32];

int get_newline ( void )
{
    unsigned int ra;

    ra=0;
    for(;fileoff<filelen;fileoff++)
    {
        switch(filedata[fileoff])
        {
            case 0x0D:
            {
                break;
            }
            case 0x0A:
            {
                newline[ra]=0;
//printf("%s\n",newline);
                fileoff++;
                return(0);
            }
            default:
            {
                newline[ra++]=filedata[fileoff];
                break;
            }
        }
    }
    if(ra)
    {
        newline[ra]=0;
        return(0);
    }
    return(1);
}

unsigned char fromhex ( unsigned char x )
{
    if(x>0x39)
    {
        return((x-7)&0xF);
    }
    else
    {
        return((x)&0xF);
    }
}




int main ( int argc, char *argv[] )
{
    unsigned int ra;
    unsigned int rb;
    unsigned int rc;
    unsigned int rd;
    unsigned int add;
    unsigned int baseadd;
    unsigned int line;
    FILE *fp;

    if(argc<2)
    {
        printf(".hex file not specified\n");
        return(1);
    }
    fp=fopen(argv[1],"rt");
    if(fp==NULL)
    {
        printf("Error opening file [%s]\n",argv[1]);
        return(1);
    }
    filelen=fread(filedata,1,sizeof(filedata),fp);
    fclose(fp);
    printf("%u bytes read\n",filelen);
    memset(rom,0xFF,sizeof(rom));
    fileoff=0;
    baseadd=0;
    line=0;
    while(1)
    {
        if(get_newline()) break;
        line++;
        //printf("%s\n",newline);
        if(newline[0]!=':')
        {
            printf("skipping %u\n",line);
            continue;
        }
        rd=0;
        rc=0;
        for(ra=1;newline[ra];ra++)
        {
            if(ra&1)
            {
                rb=fromhex(newline[ra]);
            }
            else
            {
                rb<<=4;
                rb|=fromhex(newline[ra]);
                //printf("%02X\n",rb);
                rc+=rb;
                data[rd++]=rb;
            }
        }
        //printf("%u %X\n",rd,rc);
        if(rc&0xFF)
        {
            printf("Checksum Error 0x%X line %u\n",rc,line);
            return(1);
        }
        if(rd<5)
        {
            printf("Length Error 0x%X line %u\n",rd,line);
            return(1);
        }
        if(rd!=(data[0]+5))
        {
            printf("Length Error 0x%X line %u\n",rd,line);
            return(1);
        }
        add=data[1];
        add<<=8;
        add|=data[2];
        switch(data[3])
        {
            case 0x00: //data
            {
                for(ra=0;ra<data[0];ra++)
                {
                    rom[(baseadd+add)&ROMMASK]=data[ra+4];
                    add++;
                }
                break;
            }
            case 0x02: //extended segment address
            {
                printf("type 02 not supported\n");
                break;
            }
            case 0x03: //start segment address
            {
                printf("type 03 not supported\n");
                break;
            }
            case 0x04: //extended linear address
            {
                printf("type 04 not supported\n");
                break;
            }
            case 0x05: //start linear address
            {
                printf("type 05 not supported\n");
                break;
            }


        }
        if(data[3]==0x01) break; //eof
    }
    rb=0;
    for(ra=0;ra<sizeof(rom);ra++) if(rom[ra]!=0xFF) rb=ra;
    //align on 256 byte pages
    rb+=255;
    rb&=~255;
    fp=fopen("rom.h","wt");
    if(fp==NULL)
    {
        printf("Error creating file [rom.h]\n");
        return(1);
    }
    fprintf(fp,"\n");
    fprintf(fp,"#define ROMLEN 0x%X\n",rb);
    fprintf(fp,"static const unsigned char rom[ROMLEN]=\n");
    fprintf(fp,"{\n");
    for(ra=0;ra<rb;ra++)
    {
        fprintf(fp,"  0x%02X, //0x%04X\n",rom[ra],ra);
    }
    fprintf(fp,"};\n");
    fprintf(fp,"\n");
    fclose(fp);
    return(0);
}

