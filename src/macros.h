#include <stdio.h>

#pragma once


#ifndef inf
#define inf(f,...) fprintf(stdout,"\033[1;32m[INF]\033[0m : "f" \n",##__VA_ARGS__)
#endif//inf

#ifndef dbg
#define dbg(f,...) do{ /*fprintf(stdout,"\033[1;34m[DBG]\033[0m : "f" \n",##__VA_ARGS__)*/ }while(0)
#endif//dbg

#ifndef trc
#define trc(f,...) fprintf(stdout,"\033[1;35m[TRC]\033[0m : "f" \n",##__VA_ARGS__)
#endif//trc


#ifndef wrn
#define wrn(f,...) fprintf(stdout,"\033[1;43m[WRN]\033[0m : "f" \n",##__VA_ARGS__)
#endif//wrn

#ifndef err
#define err(f,...) fprintf(stdout,"\033[1;41m[ERR]\033[0m : "f" \n",##__VA_ARGS__)
#endif//err

#ifndef assertf
#define assertf(x,f,...) if(!(x)) { err(" \033[1;43m[assertion failed]\033[0m "f,##__VA_ARGS__); assert(x); }
#endif//assertf

#ifndef todo
#define todo(f,...) fprintf(stdout,"\033[1;44m[TODO]\033[0m [file: %s, line %lld] : "f"\n",__FILE__,__LINE__,##__VA_ARGS__);
#endif//todo

#ifndef mreset
#define mreset(target) memset(target,0,sizeof(target))
#endif//mreset

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) sizeof(a)/sizeof(a[0])
#endif//ARRAY_LEN

#ifndef INT_MIN
#define INT_MIN -2147483648
#endif//INT_MIN


#define reset_console() printf("\033[0m")