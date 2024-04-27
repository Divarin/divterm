#include <conio.h>
#include <stdio.h>
#include <stdlib.h>


int main(void)
{
    unsigned char* inbuffer;
    unsigned char* outbuffer;
    char name[6];
    unsigned char c;

    name[0]=6;
    name[1]=0;
    name[2]=0;

    inbuffer = (unsigned char *) malloc (1);
    outbuffer = (unsigned char *) malloc (1);

    //fast();

	if(!cbm_open(2,2,0,name))
        puts("Unable to open modem.");

    do
    {
        if(cbm_read(2, inbuffer, 1) > 0)
             putchar(*inbuffer);

        c = getchar();

        if (c)
        {
            putchar(c);
            *outbuffer = c;
            cbm_write(2, outbuffer,1);
        }

    } while(c != '.');

    cbm_close(2);

}