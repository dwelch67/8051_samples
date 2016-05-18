
//------------------------------------------------------------------------
//------------------------------------------------------------------------

void PUT16 ( unsigned int, unsigned int );
void PUT32 ( unsigned int, unsigned int );
unsigned int GET16 ( unsigned int );
unsigned int GET32 ( unsigned int );
void dummy ( unsigned int );

#define RCCBASE 0x40023800
#define RCC_CR          (RCCBASE+0x00)
#define RCC_CFGR        (RCCBASE+0x08)
#define RCC_APB1RSTR    (RCCBASE+0x20)
#define RCC_AHB1ENR     (RCCBASE+0x30)
#define RCC_APB1ENR     (RCCBASE+0x40)
#define RCC_BDCR        (RCCBASE+0x70)

#define GPIOABASE 0x40020000
#define GPIOA_MODER     (GPIOABASE+0x00)
#define GPIOA_OTYPER    (GPIOABASE+0x04)
#define GPIOA_OSPEEDR   (GPIOABASE+0x08)
#define GPIOA_PUPDR     (GPIOABASE+0x0C)
#define GPIOA_BSRR      (GPIOABASE+0x18)
#define GPIOA_AFRL      (GPIOABASE+0x20)

#define GPIOCBASE 0x40020800
#define GPIOC_MODER     (GPIOCBASE+0x00)
#define GPIOC_OTYPER    (GPIOCBASE+0x04)
#define GPIOC_OSPEEDR   (GPIOCBASE+0x08)
#define GPIOC_PUPDR     (GPIOCBASE+0x0C)
#define GPIOC_IDR       (GPIOCBASE+0x10)
#define GPIOC_BSRR      (GPIOCBASE+0x18)

#define USART2BASE 0x40004400
#define USART2_SR       (USART2BASE+0x00)
#define USART2_DR       (USART2BASE+0x04)
#define USART2_BRR      (USART2BASE+0x08)
#define USART2_CR1      (USART2BASE+0x0C)

//PA2 is USART2_TX alternate function 1
//PA3 is USART2_RX alternate function 1

//------------------------------------------------------------------------
int uart2_init ( void )
{
    unsigned int ra;

    ra=GET32(RCC_AHB1ENR);
    ra|=1<<0; //enable port A
    PUT32(RCC_AHB1ENR,ra);

    ra=GET32(RCC_APB1ENR);
    ra|=1<<17; //enable USART2
    PUT32(RCC_APB1ENR,ra);

    ra=GET32(GPIOA_MODER);
    ra&=~(3<<4); //PA2
    ra&=~(3<<6); //PA3
    ra|=2<<4; //PA2
    ra|=2<<6; //PA3
    PUT32(GPIOA_MODER,ra);

    ra=GET32(GPIOA_OTYPER);
    ra&=~(1<<2); //PA2
    ra&=~(1<<3); //PA3
    PUT32(GPIOA_OTYPER,ra);

    ra=GET32(GPIOA_OSPEEDR);
    ra|=3<<4; //PA2
    ra|=3<<6; //PA3
    PUT32(GPIOA_OSPEEDR,ra);

    ra=GET32(GPIOA_PUPDR);
    ra&=~(3<<4); //PA2
    ra&=~(3<<6); //PA3
    PUT32(GPIOA_PUPDR,ra);

    ra=GET32(GPIOA_AFRL);
    ra&=~(0xF<<8); //PA2
    ra&=~(0xF<<12); //PA3
    ra|=0x7<<8; //PA2
    ra|=0x7<<12; //PA3
    PUT32(GPIOA_AFRL,ra);

    ra=GET32(RCC_APB1RSTR);
    ra|=1<<17; //reset USART2
    PUT32(RCC_APB1RSTR,ra);
    ra&=~(1<<17);
    PUT32(RCC_APB1RSTR,ra);

    //PUT32(USART2_CR1,(1<<13));

    //16000000/(16*115200) = 8.68  8+10/16ths or do you round that to 11?
    PUT32(USART2_BRR,0x8A);
    PUT32(USART2_CR1,(1<<3)|(1<<2)|(1<<13));

    return(0);
}
//------------------------------------------------------------------------
void uart2_send ( unsigned int x )
{
    while(1) if(GET32(USART2_SR)&(1<<7)) break;
    PUT32(USART2_DR,x);
}
//------------------------------------------------------------------------
unsigned int uart2_recv ( void )
{
    while(1) if((GET32(USART2_SR))&(1<<5)) break;
    return(GET32(USART2_DR));
}
//------------------------------------------------------------------------
void hexstrings ( unsigned int d )
{
    //unsigned int ra;
    unsigned int rb;
    unsigned int rc;

    rb=32;
    while(1)
    {
        rb-=4;
        rc=(d>>rb)&0xF;
        if(rc>9) rc+=0x37; else rc+=0x30;
        uart2_send(rc);
        if(rb==0) break;
    }
    uart2_send(0x20);
}
//------------------------------------------------------------------------
void hexstring ( unsigned int d )
{
    hexstrings(d);
    uart2_send(0x0D);
    uart2_send(0x0A);
}

#define  MOSI  11
#define  SCK   10
#define  RST   12
#define  MISO  2
#define  SS    3

void spi_delay ( void )
{
    dummy(0);
}

void spi_mosi ( unsigned int x )
{
    unsigned int ra;

    ra=1<<MOSI;
    if(x==0) ra<<=16;
    PUT32(GPIOC_BSRR,ra);
}
void spi_sck ( unsigned int x )
{
    unsigned int ra;

    ra=1<<SCK;
    if(x==0) ra<<=16;
    PUT32(GPIOC_BSRR,ra);
}
void spi_rst ( unsigned int x )
{
    unsigned int ra;

    ra=1<<RST;
    if(x==0) ra<<=16;
    PUT32(GPIOC_BSRR,ra);
}
void spi_ss ( unsigned int x )
{
    unsigned int ra;

    ra=1<<SS;
    if(x==0) ra<<=16;
    PUT32(GPIOC_BSRR,ra);
}
unsigned int spi_miso ( void )
{
    unsigned int ra;

    ra=GET32(GPIOC_IDR);
    if(ra&(1<<MISO)) return(1);
    return(0);
}

void isp_program_enable ( void )
{
    unsigned int ra;
    unsigned int rb;

    spi_ss(0);
    spi_delay();
    //MOSI they sample on rising we change on/at falling
    rb=0xAA55AC53;
    for(ra=0x80000000;ra;ra>>=1)
    {
        spi_mosi(ra&rb);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
    }
    spi_delay();
    spi_ss(1);
}

void isp_chip_erase ( void )
{
    unsigned int ra;
    unsigned int rb;

    spi_ss(0);
    spi_delay();
    //MOSI they sample on rising we change on/at falling
    rb=0xAA558A;
    for(ra=0x800000;ra;ra>>=1)
    {
        spi_mosi(ra&rb);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
    }
    spi_delay();
    spi_ss(1);
}
void isp_command_write ( unsigned int opcode, unsigned int address, unsigned int data )
{
    unsigned int ra;
    unsigned int rb;

    spi_ss(0);
    spi_delay();
    //MOSI they sample on rising we change on/at falling
    rb=0xAA55;
    for(ra=0x8000;ra;ra>>=1)
    {
        spi_mosi(ra&rb);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
    }
    for(ra=0x80;ra;ra>>=1)
    {
        spi_mosi(ra&opcode);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
    }
    for(ra=0x8000;ra;ra>>=1)
    {
        spi_mosi(ra&address);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
    }
    for(ra=0x80;ra;ra>>=1)
    {
        spi_mosi(ra&data);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
    }
    spi_delay();
    spi_ss(1);
}
void isp_command_write_four ( unsigned int opcode, unsigned int address, unsigned int data )
{
    unsigned int ra;
    unsigned int rb;

    spi_ss(0);
    spi_delay();
    //MOSI they sample on rising we change on/at falling
    rb=0xAA55;
    for(ra=0x8000;ra;ra>>=1)
    {
        spi_mosi(ra&rb);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
    }
    for(ra=0x80;ra;ra>>=1)
    {
        spi_mosi(ra&opcode);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
    }
    for(ra=0x8000;ra;ra>>=1)
    {
        spi_mosi(ra&address);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
    }
    for(ra=0x80000000;ra;ra>>=1)
    {
        spi_mosi(ra&data);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
    }
    spi_delay();
    spi_ss(1);
}
unsigned int isp_command_read ( unsigned int opcode, unsigned int address )
{
    unsigned int ra;
    unsigned int rb;
    unsigned int data;

    spi_ss(0);
    spi_delay();
    //MOSI they sample on rising we change on/at falling
    rb=0xAA55;
    for(ra=0x8000;ra;ra>>=1)
    {
        spi_mosi(ra&rb);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
        spi_delay();
    }
    for(ra=0x80;ra;ra>>=1)
    {
        spi_mosi(ra&opcode);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
        spi_delay();
    }
    for(ra=0x8000;ra;ra>>=1)
    {
        spi_mosi(ra&address);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
        spi_delay();
    }
    data=0;
    spi_mosi(0);
    //INPUT we sample on rising
    for(ra=0x80;ra;ra>>=1)
    {
        spi_delay();
        spi_sck(1);
        spi_delay();
        if(spi_miso()) data|=ra;
        spi_sck(0);
    }
    spi_delay();
    spi_ss(1);
    spi_delay();
    return(data);
}

unsigned int isp_command_read_four ( unsigned int opcode, unsigned int address )
{
    unsigned int ra;
    unsigned int rb;
    unsigned int data;

    spi_ss(0);
    spi_delay();
    //MOSI they sample on rising we change on/at falling
    rb=0xAA55;
    for(ra=0x8000;ra;ra>>=1)
    {
        spi_mosi(ra&rb);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
        spi_delay();
    }
    for(ra=0x80;ra;ra>>=1)
    {
        spi_mosi(ra&opcode);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
        spi_delay();
    }
    for(ra=0x8000;ra;ra>>=1)
    {
        spi_mosi(ra&address);
        spi_delay();
        spi_sck(1);
        spi_delay();
        spi_sck(0);
        spi_delay();
    }
    data=0;
    spi_mosi(0);
    //INPUT we sample on rising
    for(ra=0x80000000;ra;ra>>=1)
    {
        spi_delay();
        spi_sck(1);
        spi_delay();
        if(spi_miso()) data|=ra;
        spi_sck(0);
    }
    spi_delay();
    spi_ss(1);
    spi_delay();
    return(data);
}


//-------------------------------------------------------------------------
unsigned int next_rand ( unsigned int x )
{
    if(x&1)
    {
        x=x>>1;
        x=x^0xBF9EC099;
    }
    else
    {
        x=x>>1;
    }
    return(x);
}

//------------------------------------------------------------------------
int notmain ( void )
{
    unsigned int ra;
    unsigned int rb;
    unsigned int prand;
    unsigned int addr;

    uart2_init();
    hexstring(0x12345678);

    ra=GET32(RCC_AHB1ENR);
    ra|=1<<2; //enable GPIOC
    PUT32(RCC_AHB1ENR,ra);

    ra=GET32(GPIOC_MODER);
    ra&=~(3<<(RST<<1));
    ra|=(1<<(RST<<1));
    PUT32(GPIOC_MODER,ra);

    ra=GET32(GPIOC_OTYPER);
    ra&=~(3<<(RST<<0));
    PUT32(GPIOC_OTYPER,ra);

    ra=GET32(GPIOC_OSPEEDR);
    ra|=(1<<(RST<<1));
    PUT32(GPIOC_OSPEEDR,ra);
    //pupdr
    ra=GET32(GPIOC_PUPDR);
    ra&=~(3<<(RST<<1));
    PUT32(GPIOC_PUPDR,ra);

    spi_rst(1);
    dummy(0);
    dummy(1);
    dummy(2);
    dummy(3);
    spi_rst(0);
    dummy(0);
    dummy(1);
    dummy(2);
    dummy(3);

    ra=GET32(GPIOC_MODER);
    ra&=~(3<<(MOSI<<1));
    ra&=~(3<<(SCK<<1));
    ra&=~(3<<(RST<<1));
    ra&=~(3<<(MISO<<1));
    ra&=~(3<<(SS<<1));
    ra|=(1<<(MOSI<<1));
    ra|=(1<<(SCK<<1));
    ra|=(1<<(RST<<1));
    //ra|=(1<<(MISO<<1));
    ra|=(1<<(SS<<1));
    PUT32(GPIOC_MODER,ra);

    ra=GET32(GPIOC_OTYPER);
    ra&=~(3<<(MOSI<<0));
    ra&=~(3<<(SCK<<0));
    ra&=~(3<<(RST<<0));
    ra&=~(3<<(MISO<<0));
    ra&=~(3<<(SS<<0));
    PUT32(GPIOC_OTYPER,ra);

    ra=GET32(GPIOC_OSPEEDR);
    ra|=(1<<(MOSI<<1));
    ra|=(1<<(SCK<<1));
    ra|=(1<<(RST<<1));
    ra|=(1<<(MISO<<1));
    ra|=(1<<(SS<<1));
    PUT32(GPIOC_OSPEEDR,ra);
    //pupdr
    ra=GET32(GPIOC_PUPDR);
    ra&=~(3<<(MOSI<<1));
    ra&=~(3<<(SCK<<1));
    ra&=~(3<<(RST<<1));
    ra&=~(3<<(MISO<<1));
    ra&=~(3<<(SS<<1));
    ra|=(1<<(MISO<<1));
    PUT32(GPIOC_PUPDR,ra);

    spi_rst(0); //should already be
    spi_ss(1);
    spi_sck(0);
    spi_delay();
    spi_delay();

    isp_program_enable();

    ra=isp_command_read_four(0x38,0);    hexstring(ra);
    ra>>=8;
    if(ra!=0x001E28FF)
    {
        hexstring(0xBADBAD01);
        return(1);
    }
    ra=isp_command_read(0x60,0);    hexstring(ra);

    ra=isp_command_read_four(0x61,0);    hexstring(ra);
    ra=isp_command_read_four(0x61,4);    hexstring(ra);
    ra=isp_command_read_four(0x61,8);    hexstring(ra);
    ra=isp_command_read_four(0x32,0);    hexstring(ra);

    isp_chip_erase();
    while(1)
    {
        ra=isp_command_read(0x60,0); hexstring(ra);
        if(ra&1) break;
    }
    ra=isp_command_read_four(0x30,0);    hexstring(ra);

    //AT89LP214  2kbytes  32 bytes page size 64 pages address 0x0000h to 0x07FFh
    prand=0x12345;
    for(addr=0x000;addr<0x800;addr+=4)
    {
        if((addr&0xFF)==0) hexstring(addr);

        prand=next_rand(prand);
        isp_command_write_four(0x50,addr,prand);
        while(1)
        {
            ra=isp_command_read(0x60,0); //hexstring(ra);
            if(ra&1) break;
        }
    }
    prand=0x12345;
    for(addr=0x000;addr<0x800;addr+=4)
    {
        if((addr&0xFF)==0) hexstring(addr);

        prand=next_rand(prand);
        ra=isp_command_read_four(0x30,addr);
        if(ra!=prand)
        {
            hexstrings(addr);
            hexstrings(ra);
            hexstring(prand);
        }
    }
    isp_chip_erase();
    while(1)
    {
        ra=isp_command_read(0x60,0);
        if(ra&1) break;
    }
    hexstring(0x12345678);
    return(0);
}
//------------------------------------------------------------------------
//------------------------------------------------------------------------
