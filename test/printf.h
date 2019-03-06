#ifndef __PRINTF_H__
#define __PRINTF_H__

#include <stdarg.h>

//都是2的自然数次方,形成互不干扰的bit位域0101 0101
#define ZEROPAD	1		//显示的字符前面填充0取代空格,对于所有的数字格式用前导零而不是空格填充字段宽度,如果出现-标志或者指定了精度(对于整数)则忽略该标志
#define SIGN	2		//无符号还是有符号long,这里的有符号应该是要不要打印+号
#define PLUS	4		//有符号的值若为正则显示带加号的符号,若为负则显示带减号的符号
#define SPACE	8		//有符号的值若为正则显示时带前导空格但是不显示符号,若为负则带减号符号,+标志会覆盖空格标志
#define LEFT	16		//对齐后字符放在左边
#define SPECIAL	32		//0x
#define SMALL	64		//小写

#define is_digit(c)	((c) >= '0' && (c) <= '9')

#define do_div(n,base) ({ \
int __res; \
__asm__("divq %%rcx":"=a" (n),"=d" (__res):"0" (n),"1" (0),"c" (base)); \
__res; })
												//div将整数值num除以进制规格base,被除数由RDX:RAX组成,由于num变量是个8B的长整型变量,余数部分__res即是digits数组的下标索引值,整个表达式最终值等于__res
												//寄存器约束=a令RAX=n,寄存器约束=d令rcx=__res,通用约束1令RDX=0,寄存器约束c令rcx=base

//获取字符串长度
inline int strlen(char *String)
{												//repne不相等则重复,重复前判断ecx是否为零,不为零则减1,scasb查询di中是否有al中的字符串结束字符0,如果有则退出循环,将ecx取反后减去1得到字符串长度
	register int __res;							//输出约束:ecx剩下字符串的长度(不包括末尾0)
	__asm__	__volatile__	(	"cld	\n\t"
					"repne	\n\t"
					"scasb	\n\t"
					"notl	%0	\n\t"
					"decl	%0	\n\t"
					:"=c"(__res)
					:"D"(String),"a"(0),"0"(0xffffffff)
					:
				);								//输入约束:寄存器约束D令String的首地址约束在edi,立即数约束令al=0,通用约束0令ecx=0xffff ffff
	return __res;								//ecx=0xffff ffff,al=0,edi指向String首地址,方向从低到高
}

#endif