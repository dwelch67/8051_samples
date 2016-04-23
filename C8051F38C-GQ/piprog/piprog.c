
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

// 2  outer corner
// 4
// 6
// 8  TX out
// 10 RX in

#define PBASE 0x20000000

extern void PUT32 ( unsigned int, unsigned int );
extern void PUT16 ( unsigned int, unsigned int );
extern void PUT8 ( unsigned int, unsigned int );
extern unsigned int GET32 ( unsigned int );
extern unsigned int GETPC ( void );
extern void BRANCHTO ( unsigned int );
extern void dummy ( unsigned int );

extern void uart_init ( void );
extern unsigned int uart_lcr ( void );
extern void uart_flush ( void );
extern void uart_send ( unsigned int );
extern unsigned int uart_recv ( void );
extern unsigned int uart_check ( void );
extern void hexstring ( unsigned int );
extern void hexstrings ( unsigned int );
extern void timer_init ( void );
extern unsigned int timer_tick ( void );

extern void leds_off ( void );


#define ARM_TIMER_CTL   (PBASE+0x0000B408)
#define ARM_TIMER_CNT   (PBASE+0x0000B420)

static unsigned int last_time;
static void c2_delay_init ( void )
{
    last_time=GET32(ARM_TIMER_CNT);
}
static void c2_delay ( void )
{
    unsigned int now_time;

    while(1)
    {
        now_time=GET32(ARM_TIMER_CNT);
        if((now_time-last_time)>=500) break;
    }
    last_time=now_time;
}


static unsigned char rdata[256];
static unsigned char sdata[256];


#define GPFSEL2 (PBASE+0x00200008)
#define GPSET0  (PBASE+0x0020001C)
#define GPCLR0  (PBASE+0x00200028)
#define GPLEV0  (PBASE+0x00200034)

#define C2CK 23
#define C2D  24

static void c2ck_output ( void )
{
    unsigned int ra;

    ra=GET32(GPFSEL2);
    ra&=~(7<<9);
    ra|=1<<9;
    PUT32(GPFSEL2,ra);
}

static void c2d_output ( void )
{
    unsigned int ra;

    ra=GET32(GPFSEL2);
    ra&=~(7<<12);
    ra|=1<<12;
    PUT32(GPFSEL2,ra);
}

static void c2d_input ( void )
{
    unsigned int ra;

    ra=GET32(GPFSEL2);
    ra&=~(7<<12);
    PUT32(GPFSEL2,ra);
}

static unsigned int c2d_read_input ( void )
{
    return(GET32(GPLEV0)&(1<<C2D));
}

static void c2d_set ( void )
{
    PUT32(GPSET0,1<<C2D);
}

static void c2d_clr ( void )
{
    PUT32(GPCLR0,1<<C2D);
}

static void c2ck_set ( void )
{
    PUT32(GPSET0,1<<C2CK);
}

static void c2ck_clr ( void )
{
    PUT32(GPCLR0,1<<C2CK);
}

static void c2_init ( void )
{
    //250Mhz timer.
    //need to measure 2us so
    //1/2us is 500,000
    //250000000/500000 = 500
    PUT32(ARM_TIMER_CTL,0x00000000);
    PUT32(ARM_TIMER_CTL,0x00000200);

    c2ck_output();
    c2d_input();
}

static void c2_reset ( void )
{
    unsigned int ra;

    c2_delay_init();
    c2d_input();
    c2ck_set();
    for(ra=0;ra<15;ra++) c2_delay();
    c2ck_clr();
    for(ra=0;ra<15;ra++) c2_delay();
    c2ck_set();
    for(ra=0;ra<15;ra++) c2_delay();
}

//void STROBE_C2CK(void)
//{
    //GPIO_PinOutSet(C2CK_PORT, C2CK_PIN);
    //GPIO_PinOutClear(C2CK_PORT, C2CK_PIN);
    //GPIO_PinOutClear(C2CK_PORT, C2CK_PIN);
    //GPIO_PinOutSet(C2CK_PORT, C2CK_PIN);
    //GPIO_PinOutSet(C2CK_PORT, C2CK_PIN);
    //GPIO_PinOutSet(C2CK_PORT, C2CK_PIN);
//}

static void c2_strobe ( void )
{
    c2_delay();
    c2ck_clr();
    c2_delay();
    c2_delay();
    c2ck_set();
    c2_delay();
}

static unsigned int c2_read_byte ( void )
{
    unsigned int ra;

    unsigned int rx;

    //assume c2d input
    //assume c2ck high
    c2_delay_init();
    c2_strobe();
    c2d_output();
    c2d_clr();
    c2_strobe();
    c2_strobe();
    c2_strobe();
    c2_strobe();
    c2d_input();
    c2_strobe();
    for(rx=1;rx;)
    {
        if(c2d_read_input()) rx=0;
        c2_strobe();
    }
    rx=0;
    for(ra=0;ra<8;ra++)
    {
        rx>>=1;
        if(c2d_read_input()) rx|=0x80;
        c2_strobe();
    }
    //c2_strobe();
    return(rx);
}

static unsigned int c2_read_address ( void )
{
    unsigned int ra;

    unsigned int rx;

    //assume c2d input
    //assume c2ck high
    c2_delay_init();
    c2_strobe();
    c2d_output();
    c2d_clr();
    c2_strobe();
    c2d_set();
    c2_strobe();
    c2d_input();
    c2_strobe();
    rx=0;
    for(ra=0;ra<8;ra++)
    {
        rx>>=1;
        if(c2d_read_input()) rx|=0x80;
        c2_strobe();
    }
    //c2_strobe();
    return(rx);
}

static void c2_write_address ( unsigned int addr )
{
    unsigned int ra;
    unsigned int rx;

    //assume c2d input
    //assume c2ck high
    c2_delay_init();
    c2_strobe();
    c2d_output();
    c2d_set();
    c2_strobe();
    c2_strobe();
    rx=addr;
    for(ra=0;ra<8;ra++)
    {
        if(rx&1) c2d_set();
        else     c2d_clr();
        c2_strobe();
        rx>>=1;
    }
    c2d_input();
    c2_strobe();
}

static void c2_write_byte ( unsigned int data )
{
    unsigned int ra;
    unsigned int rx;

    //assume c2d input
    //assume c2ck high
    c2_delay_init();
    c2_strobe();
    c2d_output();
    c2d_set();
    c2_strobe();
    c2d_clr();
    c2_strobe();
    c2_strobe();
    c2_strobe();
    rx=data;
    for(ra=0;ra<8;ra++)
    {
        if(rx&1) c2d_set();
        else     c2d_clr();
        c2_strobe();
        rx>>=1;
    }
    c2d_input();
    c2_strobe();
    for(rx=1;rx;)
    {
        if(c2d_read_input()) rx=0;
        c2_strobe();
    }
}
static void c2_inbusy ( void )
{
    while(1)
    {
        if((c2_read_address()&2)==0) break;
    }
}
static void c2_outready ( void )
{
    while(1)
    {
        if(c2_read_address()&1) break;
    }
}

//0xAD for the 0x28 DEVID part
#define FPDAT 0xAD

static void c2_command ( unsigned int x )
{
    c2_write_address(FPDAT);
    c2_write_byte(x);
    c2_inbusy();
}
static int c2_response ( void )
{
    unsigned int ra;

    c2_outready();
    ra=c2_read_byte();
    if(ra!=0x0D)
    {
        hexstrings(0x0DBADBAD);
        hexstring(ra);
        return(1);
    }
    return(0);
}


static int c2_erase_page ( unsigned int  p )
{
    c2_command(0x08); //Page Erase
    if(c2_response()) return(1);
    c2_write_byte(p);
    c2_inbusy();
    if(c2_response()) return(1);
    c2_write_byte(0x00);
    c2_inbusy();
    if(c2_response()) return(1);
    return(0);
}
static int c2_read_block ( unsigned int add, unsigned int len )
{
    unsigned int ra;

    c2_command(0x06); //Read Block
    if(c2_response()) return(1);
    c2_write_byte((add>>8)&0xFF);
    c2_inbusy();
    c2_write_byte((add>>0)&0xFF);
    c2_inbusy();
    c2_write_byte(len);
    c2_inbusy();
    if(c2_response()) return(1);
    for(ra=0;ra<len;ra++)
    {
        c2_outready();
        rdata[ra]=c2_read_byte();
    }
    return(0);
}
//------------------------------------------------------------------------
int notmain ( void )
{
    unsigned int ra;
    unsigned int rb;
    unsigned int errors;

    leds_off();
    uart_init();
    hexstring(0x12345678);
    hexstring(GETPC());

    c2_init();
    c2_reset();
    ra=c2_read_byte();
    hexstring(ra);
    if(ra!=0x28)
    {
        hexstring(0x00BADBAD);
        return(1);
    }
    c2_write_address(0x01);
    ra=c2_read_byte();
    hexstring(ra);
    c2_write_address(0x00);
    ra=c2_read_byte();
    hexstring(ra);
    //ADDRESSES
    //0x00 DEVICE ID
    //0x01 REVISION ID
    //0x02 FLASH CONTROL
    //0xAD FLASH READ
    //0xB4 FPDAT

    hexstring(0x11111111);

    //flash init/halt
    c2_reset();
    c2_write_address(0x02);
    c2_write_byte(0x02);
    c2_write_byte(0x04);
    c2_write_byte(0x01);
    for(ra=0;ra<15;ra++) c2_delay();


    c2_command(0x01); //Get Version
    if(c2_response()) return(1);
    ra=c2_read_byte();  hexstring(ra);

    c2_command(0x02); //Get Derivative
    if(c2_response()) return(1);
    ra=c2_read_byte();  hexstring(ra);

    //C8051F38C-GQ 16Kbytes,  64 pages
    //erase 8192 bytes, why doesnt 16K work?
    for(ra=0;ra<32;ra++)
    {
        hexstring(ra);
        if(c2_erase_page(ra)) return(1);
    }
    errors=0;
    for(ra=0;ra<8192;ra+=256)
    {
        hexstring(ra);
        if(c2_read_block(ra,256)) return(1);
        for(rb=0;rb<256;rb++)
        {
            if(rdata[rb]!=0xFF)
            {
                hexstrings(0xBAD);
                hexstrings(ra+rb);
                hexstrings(rdata[rb]);
                errors++;
            }
        }
    }
    if(errors) return(1);



////28 01 00

////1. Perform an Address Write with a value of FPDAT.
////2. Perform a Data Write with the Block Write command.
////3. Poll on InBusy using Address Read until the bit clears.
    //c2_command(0x07); //block write
////4. Poll on OutReady using Address Read until the bit set.
////5. Perform a Data Read instruction. A value of 0x0D is okay.
    //if(c2_response()) return(1);
////6. Perform a Data Write with the high byte of the address.
    //c2_write_byte(0x00);
////7. Poll on InBusy using Address Read until the bit clears.
    //c2_inbusy();
////8. Perform a Data Write with the low byte of the address.
    //c2_write_byte(0x00);
////9. Poll on InBusy using Address Read until the bit clears.
    //c2_inbusy();
////10. Perform a Data Write with the length.
    //c2_write_byte(0x03);
////11. Poll on InBusy using Address Read until the bit clears.
    //c2_inbusy();
    //c2_write_byte(0x28);
    //c2_inbusy();
    //c2_write_byte(0x01);
    //c2_inbusy();
    //c2_write_byte(0x00);
////12. Perform a Data Write with the data. This will write the data to the flash. Repeat steps 11 and 12 for each byte specified by the
////length field.
////13. Poll on OutReady using Address Read until the bit set.
////14. Perform a Data Read instruction. A value of 0x0D is okay.
    //if(c2_response()) return(1);

    return(0);
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------


//-------------------------------------------------------------------------
//
// Copyright (c) 2014 David Welch dwelch@dwelch.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------
