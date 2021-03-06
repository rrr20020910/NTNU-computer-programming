#include <stdint.h>
#include <stdio.h>

//array order:7,6,5,4,3,2,1,
//            G,F,E,D,C,B,A,
//            ' ',*
// uint32_t element[16] = {0x10020000,0x40010000,0x00080000,0x00000000,0x00000000,0x02000000,0x00000000
//                        ,0x80200000,0x00040000,0x00000000,0x00400000,0x00000000,0x00000000,0x20000000
//                        ,0x0C902000,0x0100Dfff};
// *,*,*,*
// *,*,*,*
// *,*,*,*
// *,*, ,*
// 5,F,7,*
//  ,D,G,
//  , ,2,*
// G,1,A,7

// uint32_t element[16] = {0x00000001,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000
//                        ,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000000,0x00000002
//                        ,0x0000000C,0xffffff0};

// uint32_t black = 0x520A0000
//         ,red = 0xA0640000
//         ,chess = 0xf36fDfff;

// int32_t color=0;

static int available_step[32*32];
uint32_t black = 0x00000000
        ,red =   0x00000000
        ,chess = 0x00000000;

uint32_t board_y[4] = {0x11111111,0x22222222,0x44444444,0x88888888};
uint32_t board_x[8] = {0x0000000F,0x000000F0,0x00000F00,0x0000F000,0x000F0000,0x00F00000,0x0F000000,0xF0000000};
int32_t _index[32] = {31,0,1,5,2,16,27,6,3,14,17,19,28,11,7,21,30,4,15,26,13,18,10,20,29,25,12,9,24,8,23,22};
uint32_t chess_move[32] = {0x00000012,0x00000025,0x0000004A,0x00000084,0x00000121,0x00000252,0x000004A4,0x00000848,
                        0x00001210,0x00002520,0x00004A40,0x00008480,0x00012100,0x00025200,0x0004A400,0x00084800,
                        0x00121000,0x00252000,0x004A4000,0x00848000,0x01210000,0x02520000,0x04A40000,0x08480000,
                        0x12100000,0x25200000,0x4A400000,0x84800000,0x21000000,0x52000000,0xA4000000,0x48000000};

uint32_t get_least_bit(uint32_t x)
{
    return x & (-x);
}

uint32_t get_first_bit(uint32_t x)
{
    x |= (x>>31);
    x |= (x>>16);
    x |= (x>>8);
    x |= (x>>4);
    x |= (x>>2);
    x |= (x>>1);
    return (x>>1)+1;
}

int32_t get_board_index(uint32_t mask)
{
    return _index[(mask*0x08ED2BE6)>>27];
}

uint32_t c_method_CL(uint32_t x)
{
    if(x)
    {
        uint32_t mask = get_least_bit(x);
        return (x^= mask) ? get_least_bit(x) : 0;
    }
    else
    {
        return 0;
    }    
}

uint32_t c_method_CR(uint32_t x)
{
    if(x)
    {
        uint32_t mask = get_first_bit(x);
        return (x^= mask) ? get_first_bit(x) : 0;
    }
    else
    {
        return 0;
    }    
}

uint32_t c_method(int32_t x)
{
    int32_t src = x;
    uint32_t co,ro;
    int32_t co_n = x / 4;
    int32_t ro_n = x % 4;
    x = ((board_y[ro_n] & chess) ^ (1<<src))>>(4*ro_n);
    ////printf("\nx:%08x\n",x);
    if(co_n==0)
    {
        co = c_method_CL(x);
    }
    else if(co_n==1)
    {
        co = c_method_CL(x & 0x11111100);
    }
    else if(co_n==2)
    {
        uint32_t part = 0;
        if((x & 0x00000011) == 0x00000011)
        {
            part |= 1;
        }
        co = part|c_method_CL(x&0x11111000);
    }
    else if(co_n==3)
    {
        uint32_t part = 0;
        part = c_method_CR(x&0x00000111);
        co = part|c_method_CL(x&0x11110000);
    }
    else if(co_n==4)
    {
        uint32_t part = 0;
        part = c_method_CR(x&0x00001111);
        co = part|c_method_CL(x&0x11100000);
        ////printf("\nx:%08x\npart:%08x\nco:%08x\n",x,part,co);
    }
    else if(co_n==5)
    {
        uint32_t part = 0;
        part = c_method_CR(x&0x00011111);
        co = part|c_method_CL(x&0x11000000);
    }
    else if(co_n==6)
    {
        co = c_method_CR(x&0x00111111);
    }
    else if(co_n==7)
    {
        co = c_method_CR(x);
    }
    //co = x ^ 0x11111111;
    //printf("\nx:%08x\nco:%08x\n",x,co);
    x = ((board_x[co_n] & chess) ^ (1<<src))>>(4*co_n);
    if(ro_n==0)
    {
        ro = c_method_CL(x);
    }
    else if(ro_n==1)
    {
        ro = ((x&12)==12) ? 8 : 0;
    }
    else if(ro_n==2)
    {
        ro = ((x&3)==3) ? 1 : 0;
    }
    else if(ro_n==3)
    {
        ro = c_method_CR(x);
    }
    ro<<=(4*co_n);
    co<<=ro_n;
    ////printf("\nco:%08x\nro:%08x\n",co,ro);
    //0x44010400
    //printf("\nans:%08x\n",ro|co);
    return ro | co;
}

int *available(uint32_t element[],int color)
{
    for(int i=0;i<32*32;i++)
    {
        available_step[i] = 0;   
    }
    for(int i=0;i<16;i++)
    {
        if(i>=14)
        {
            if(i==15)
            {
                chess |= element[i];
            }
            continue;
        }
        else if(i<7)
        {
            black |= element[i];
        }
        else if(i>=7)
        {
            red |= element[i];
        }
        chess |= element[i];
    }
    // printf("black:0x%08x\n",black);
    // printf("red:0x%08x\n",red);
    // printf("chess:0x%08x\n",chess);
    // printf("' ':0x%08x\n",element[14]);
    // printf("'*':0x%08x\n",element[15]);
    for(int i=0;i<16;i++)
    {
        printf("%d:0x%08x\n",i,element[i]);
    }
    for(int i=0;i<14;i++)
    {
        uint32_t p = element[i];
        //printf("%d:0x%08x\n",i,p);
        while (p)
        {
            uint32_t mask = get_least_bit(p);
            p ^= mask;
            int32_t src = get_board_index(mask);
            uint32_t dest=0;
            if(i==0 & color==1)
            {
                dest = chess_move[src] & (element[7]|element[13]|element[14]);
            }
            else if(i==1 & color==1)
            {
                dest = c_method(src) & (red|element[14]);
            }
            else if(i==2 & color==1)
            {
                dest = chess_move[src] & (element[7]|element[8]|element[9]|element[14]);
            }
            else if(i==3 & color==1)
            {
                dest = chess_move[src] & (element[7]|element[8]|element[9]|element[10]|element[14]);
            }
            else if(i==4 & color==1)
            {
                dest = chess_move[src] & (red^element[12]^element[13]|element[14]);
            }
            else if(i==5 & color==1)
            {
                dest = chess_move[src] & (red^element[13]|element[14]);
            }
            else if(i==6 & color==1)
            {
                dest = chess_move[src] & (red^element[7]|element[14]);
            }
            else if(i==7 & color==-1)
            {
                dest = chess_move[src] & (element[0]|element[6]|element[14]);
            }
            else if(i==8 & color==-1)
            {
                dest = c_method(src) & (black|element[14]);
            }
            else if(i==9 & color==-1)
            {
                dest = chess_move[src] & (element[0]|element[1]|element[2]|element[14]);
            }
            else if(i==10 & color==-1)
            {
                dest = chess_move[src] & (element[0]|element[1]|element[2]|element[3]|element[14]);
            }
            else if(i==11 & color==-1)
            {
                dest = chess_move[src] & (black^element[5]^element[6]|element[14]);
            }
            else if(i==12 & color==-1)
            {
                dest = chess_move[src] & (black^element[6]|element[14]);
            }
            else if(i==13 & color==-1)
            {
                dest = chess_move[src] & (black^element[0]|element[14]);
            }
            //printf("dest:0x%08x\n",dest);
            while (dest)
            {
                uint32_t mask2 = get_least_bit(dest);
                dest ^= mask2;
                int32_t result  = get_board_index(mask2);
                src = (src%4)*8+(src/4);
                result = (result%4)*8+(result/4);
                //printf("asud\n");
                printf("%d,%d\n",src,result);
                available_step[src*32+result] = 1;
            }
        }
    }
    for(int i=0;i<32;i++)
    {
        if((element[15] & (1<<i))==(1<<i))
        {
            int _i = (i%4)*8+(i/4);
            available_step[_i*32+_i] = 1;    
            //printf("%d,%d\n",_i,_i);
        }
    }
    // for(int i=0;i<32;i++)
    // {
    //     for(int j=0;j<32;j++)
    //     {
    //         //printf("%d,%d:%d||",i,j,available_step[i*32+j]);
    //     }
    //     //printf("\n");
    // }
    return available_step;
}

// int main()
// {
//     scanf("%d",&color);
//     available();
//     return 0;
// }
