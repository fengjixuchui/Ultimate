#include <stdarg.h>
#include "printf.h"
#include "string.h"

//只能将数值字母转换成整数值,比如将"2345"转换成2345,因为字符2的ASCII码是50,减去字符0的ASCII码48就可以得到数值2
int skip_atoi(const char **s)
{
	int i=0;
	while (is_digit(**s))					//先判断是否是数值字母,宏定义#define is_digit(c)	((c) >= '0' && (c) <= '9')
		i = i*10 + *((*s)++) - '0';			//如果是数值字母,则将当前字符转换成数值( *((*s)++) - '0' ),并拼入已转换的整数值( i*10 + *((*s)++) - '0' )
	return i;
}

//将长整型变量值转换成指定进制规格的字符串,base指定进制数,precision提供显示精度值(不是浮点精度),不管number函数将整数值转换成大写还是小写字母,它最高支持36进制的数值转换
static char * number(char * str, long num, int base, int size, int precision,	int type)
{
	char c,sign,tmp[50];										//c用来存放填充的进制前缀后前导字符,sign正负
	const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";//默认大写共36个符号(36进制)
	int i;
	if (base < 2 || base > 36)									//不支持小于2或大于36的进制
		return 0;
	if (type&SMALL)												//定义了使用小写字母
		digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	if (type&LEFT)												//对齐后字符放在左边则字符前面将不填充0,而在后面填充空格,填充0将破坏数据大小,则令ZEROPAD位域为0表示不填充0
		type &= ~ZEROPAD;										//#define ZEROPAD	1//显示的字符前面填充0取代空格,对于所有的数字格式用前导零而不是空格填充字段宽度,如果出现-标志或者指定了精度(对于整数)则忽略该标志
	c = (type & ZEROPAD) ? '0' : ' ' ;							//c用来存放填充的进制前缀后前导字符,如果左对齐,则前面已经令ZEROPAD位域为0,这个表达式c=空格,如果右对齐且ZEROPAD位域为1,这个表达式c=0,表示显示的字符前面填充0取代空格,因为右对齐的话字符显示在右边,前导用0填充不会破坏数据

	sign = 0;													//如果是有符号数且获取到的数小于0
	if (type&SIGN && num < 0) {
		sign='-';												//-符号
		num = -num;												//将负数转换成一般正数
	} else														//如果是无符号数或者获取到的是正数再根据对齐方式决定前导填充什么符号//#define PLUS	4//有符号的值若为正则显示带加号的符号,若为负则显示带减号的符号
		sign=(type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);	//如果限定了要在正数前面显示加号,则sign=+,如果没有限定要在正数前面显示加号,否则接着计算后面的条件表达式看是前导空格还是0,没有规定SPACE的话c=0
	if (sign)
		size--;
	if (type & SPECIAL)											//#define SPECIAL	32//0x
		if (base == 16)											//十六进制的0x将占用两个宽度
			size -= 2;
		else if (base == 8)										//八进制的0将占用一个宽度
			size--;
	i = 0;
	if (num == 0)												//如果取出的数等于0,将字符串结束符'0'存入临时数组tmp第一位中
		tmp[i++]='0';											//如果取出的数不等于0则除去进制直到num=0
	else while (num!=0)
		tmp[i++]=digits[do_div(num,base)];						//将整数值转换成字符串(按数值倒序排列),然后再将tmp数组中的字符倒序插入到显示缓冲区,do_div(num,base)的值等于num除以base的余数,余数部分即是digits数组的下标索引值
	if (i > precision)											//进制转换完的数值的长度大于精度,则精度等于进制转换完的数值的长度,比如%5.3限定,1000转十进制=>1000,i=4大于精度3,精度变成4,再由剩下的数据域宽度 = 剩下的数据域宽度 - 精度,得到剩下的数据域宽度等于1,也就是要填充的位数1
		precision=i;											//否则精度不变,比如%5.3限定,10转十进制=>10,i=2小于精度3,精度还是3,再由剩下的数据域宽度 = 剩下的数据域宽度 - 精度,得到剩下的数据域宽度等于2,也就是要填充的位数2
	size -= precision;											//剩下的数据域宽度 = 数据域宽度 - 精度,至此得到数据域宽度,也就是要填充的位数
	if (!(type & (ZEROPAD + LEFT)))								//如果显示的字符前面填充的是空格且对齐后字符放在右边才可以在前导填充空格,否则放到后边if (!(flags & LEFT))再填充,因为如果对齐后字符放在左边还有进制前缀未填充
		while(size-- > 0)
			*str++ = ' ';
	if (sign)													//填充前面决定的前导符号,空格或者+号,如果既不是空格也不是+号,而是0就不填充避免破坏数据,前面的if(sign)也没有执行,至此已经填充完毕(包括0 + 空格)
		*str++ = sign;
	if (type & SPECIAL)											//#define SPECIAL	32//0x
		if (base == 8)
			*str++ = '0';										//八进制前缀0
		else if (base==16) 
		{
			*str++ = '0';										//十六进制前缀0x或0X
			*str++ = digits[33];								//x或X字符
		}
	if (!(type & LEFT))											//对齐完字符在右边的情况可以用前导0填充而不破坏数据
		while(size-- > 0)										//c用来存放填充的进制前缀后前导字符,如果左对齐,则前面已经令ZEROPAD位域为0,这个表达式c=空格,如果右对齐且ZEROPAD位域为1,这个表达式c=0,表示显示的字符前面填充0取代空格,因为右对齐的话字符显示在右边,前导用0填充不会破坏数据
			*str++ = c;
																//比如%5.3限定,10转十进制=>10,i=2小于精度3,精度还是3,再由剩下的数据域宽度 = 剩下的数据域宽度 - 精度,得到剩下的数据域宽度等于2,也就是要填充的位数2,至此要填充进制前缀后的数值部分了
	while(i < precision--)										//i等于除去进制得到几位,precision精度
		*str++ = '0';
	while(i-- > 0)												//填充完前缀后的前导符,逆序复制有效数值过去
		*str++ = tmp[i];
	while(size-- > 0)											//前面会改变field_width数据的条件是(填充空格且右对齐/填充0且右对齐),而此处的条件是剩下的条件左对齐,则填充空格
		*str++ = ' ';
	return str;
}

int vsprintf(char * buf,const char *fmt, va_list args)
{
	char * str,*s;												//str直接指向缓冲区buf,s用在字符串格式转换的临时字符串指针
	int flags;													//格式对齐控制位域
	int field_width;											//宽度
	int precision;												//精度,特别注意,这里的精度不是浮点数精度,这个函数不支持浮点数f
	int len,i;
	int qualifier;												//'h', 'l', 'L' or 'Z' for integer fields基本上只实现了l

	for(str = buf; *fmt; fmt++)									//str直接指向缓冲区buf
	{

		if(*fmt != '%')											//该循环体会逐个解析字符串,如果字符不为%就认为它是个可显示字符,直接将其存入缓冲区buf中,否则进一步解析其后的字符串格式
		{
			*str++ = *fmt;
			continue;
		}
		flags = 0;												//按照字符串规定,符号%后面可接 - + 空格 # 0等格式符,如果下一个字符是上述格式符,则设置标志变量flags的标志位(标志位定义在printk.h),随后计算出数据区域的宽度
		repeat:
			fmt++;												//后续的自增都是指向下一地址
			switch(*fmt)
			{
				case '-':flags |= LEFT;							//#define LEFT	16//对齐后字符放在左边
				goto repeat;
				case '+':flags |= PLUS;							//#define PLUS	4//有符号的值若为正则显示带加号的符号,若为负则显示带减号的符号
				goto repeat;
				case ' ':flags |= SPACE;						//#define SPACE	8//有符号的值若为正则显示时带前导空格但是不显示符号,若为负则带减号符号,+标志会覆盖空格标志
				goto repeat;
				case '#':flags |= SPECIAL;						//#define SPECIAL	32//0x
				goto repeat;
				case '0':flags |= ZEROPAD;						//#define ZEROPAD	1//显示的字符前面填充0取代空格,对于所有的数字格式用前导零而不是空格填充字段宽度。如果出现-标志或者指定了精度(对于整数)则忽略该标志
				goto repeat;
			}
																//这部分程序可提取出后续字符串中的数字,并将其转化为数值以表示数据区域的宽度,如果下一个字符不是数字而是字符*,那么数据区域的宽度将由可变参数提供,根据可变参数值亦可判断数据区域的对齐显示方式(左/右对齐)
			field_width = -1;									//不限定宽度默认为-1
			if(is_digit(*fmt))
				field_width = skip_atoi(&fmt);					//得到数据域的宽度
			else if(*fmt == '*')								//如果下一个字符不是数字而是字符*,那么数据区域的宽度将由可变参数提供,例如printf("dnumber = %*.*f\n", width, precision, dnumber);
			{
				fmt++;											//有限定*号则指向下一位
				field_width = va_arg(args, int);				//通过可变参数取得数据域宽度
				if(field_width < 0)								//数据域宽度是负数
				{
					field_width = -field_width;					//数据域宽度必须是正数,负负得正
					flags |= LEFT;								//字符放在左边
				}
			}
																//获取数据区宽度后,下一步提取出显示数据的精度,如果数据区域的宽度后面跟有字符.,说明其后的数值是显示数据的精度,这里采用与计算数据区域宽度相同的方法计算出显示数据的精度,随后还要获取显示数据的规格
			precision = -1;										//不限定精度默认为-1,如果有写.但没有后续精度限定则前两个if都不执行,精度设置为0
			if(*fmt == '.')
			{
				fmt++;											//有限定小数点则指向下一位
				if(is_digit(*fmt))
					precision = skip_atoi(&fmt);				//将数值字母转换成整数值
				else if(*fmt == '*')
				{	
					fmt++;
					precision = va_arg(args, int);				//通过可变参数取得数据域精度
				}
				if(precision < 0)								//如果精度小于0则精度设置为0
					precision = 0;
			}
																//获取显示数据的规格,比如%ld格式化字符串中的字母l,就表示显示数据的规格是长整型
			qualifier = -1;										//不限定规格默认为-1,基本上只实现了l
			if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'Z')
			{	
				qualifier = *fmt;								//获得数据规格
				fmt++;											//有限定规格则指向下一位
			}
																//宽度和精度限定不常用,接下来进入平常经常使用得格式符解析
																//经过逐个格式符的解析,数据区域的宽度和精度等信息皆已获取,现在将遵照这些信息把可变参数格式化成字符串,并存入buf缓冲区,下面进入可变参数的字符串转化过程,目前支持的格式符有c s o p x X d i u n %等
																//可变参数在接下去读取就是所要匹配的数据了printf("dnumber = %*.*f\n", width, precision, dnumber);现在args指向的下一参数是dnumber了,而不是前面的宽度和精度了
			switch(*fmt)										//判断数据格式
			{
				case 'c':										//如果匹配出格式符c,那么程序将可变参数转换为一个字符,并根据数据区域的宽度和对齐方式填充空格符,这就是%c格式符的功能,有了字符显示功能,字符串将很快实现
					if(!(flags & LEFT))							//如果对齐后字符要放在右边
						while(--field_width > 0)				//比如宽度是2,那么将会填充1个空格凑成两个宽度
							*str++ = ' ';						//补齐前面得空格
					*str++ = (unsigned char)va_arg(args, int);	//从可变参数列表取出一个字符结合填充到缓冲区
					while(--field_width > 0)					//如果对齐后字符要放在左边,补齐后面的空格
						*str++ = ' ';
					break;

				case 's':										//整个显示过程会把字符串的长度与显示精度进行比对,根据数据区的宽度和精度等信息截取待显示字符串的长度并补齐空格符,涉及到内核通用库函数strlen
					s = va_arg(args,char *);
					if(!s)										//如果字符串只有结束符
						s = '\0';
					len = strlen(s);							//字符串不是只有一个结束符则获取长度
					if(precision < 0)							//如果不限定精度默认为-1,改写成字符串长度
						precision = len;
					else if(len > precision)					//如果限定了精度并且len大于精度则精度由len决定
						len = precision;
					
					if(!(flags & LEFT))							//如果对齐后字符要放在右边则先填充空格
						while(len < field_width--)
							*str++ = ' ';
					for(i = 0;i < len ;i++)						//将字符串依次复制到缓冲区buf
						*str++ = *s++;
					while(len < field_width--)					//如果对齐后字符要放在左边则最后填充空格
						*str++ = ' ';
					break;
																//下面这部分程序是八进制/十进制/十六进制以及地址值的格式化显示功能,借助number函数实现可变参数的数字格式化功能,并根据各个格式符的功能置位响应的标志位(怎么对齐/填充)供number函数使用
				case 'o':										//八进制,搞懂了number函数就行,进制很简单
					if(qualifier == 'l')						//如果规格是长整型
						str = number(str,va_arg(args,unsigned long),8,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),8,field_width,precision,flags);
					break;

				case 'p':										//地址值
					if(field_width == -1)						//如果未限定数据域宽度
					{
						field_width = 2 * sizeof(void *);		//sizeof(void *) = 4
						flags |= ZEROPAD;						//#define ZEROPAD	1//显示的字符前面填充0取代空格,对于所有的数字格式用前导零而不是空格填充字段宽度,如果出现-标志或者指定了精度(对于整数)则忽略该标志
					}
					str = number(str,(unsigned long)va_arg(args,void *),16,field_width,precision,flags);
					break;

				case 'x':										//十六进制的处理过程x设置为小写,其余跟X一样
					flags |= SMALL;
				case 'X':
					if(qualifier == 'l')						//如果规格是长整型
						str = number(str,va_arg(args,unsigned long),16,field_width,precision,flags);
					else
						str = number(str,va_arg(args,unsigned int),16,field_width,precision,flags);
					break;

				case 'd':										//十进制
				case 'i':										//有符号处理过程跟无符号差别在于flags的SIGN域
					flags |= SIGN;
				case 'u':
					if(qualifier == 'l')						//如果规格是长整型
						str = number(str,va_arg(args,long),10,field_width,precision,flags);
					else
						str = number(str,va_arg(args,int),10,field_width,precision,flags);
					break;
																//函数的最后一部分代码负责格式化字符串的扫尾工作
				case 'n':										//格式符%n的功能是把目前已格式化的字符串长度返回给函数的调用者,意思是把刚刚接收的数据的字符个数赋给对应的变量
					if(qualifier == 'l')						//如果规格是长整型
					{
						long *ip = va_arg(args,long *);			//对应的变量,CLanguageMINEOS项目里有标准库printf的测试程序
						*ip = (str - buf);						//printf("This is%n a test program\n", count);//count = 7
					}
					else
					{
						int *ip = va_arg(args,int *);
						*ip = (str - buf);
					}
					break;

				case '%':										//如果格式化字符串中出现字符%%,则把第一个格式符%视为转义符,经过格式化解析后,最终只显示一个字符%
					*str++ = '%';
					break;

				default:										//如果在格式符解析过程中出现任何不支持的格式符,比如f,则不做任何处理,直接将其视为字符串输出到buf缓冲区
					*str++ = '%';	
					if(*fmt)
						*str++ = *fmt;
					else										//碰到字符串结束符fmt指针回退并break,退出外层循环后又因为碰到结束符,for循环结束,整个函数结束
						fmt--;
					break;
			}

	}
	*str = '\0';												//格式化后的字符串结束符
	return str - buf;											//abc\0,则str - buf = 3 - 0 = 3,长度为3
}

int sprintf(char * buf,const char * fmt,...)
{
	int count = 0;

	va_list args;
	va_start(args, fmt);
	count = vsprintf(buf,fmt, args);
	va_end(args);

	return count;
}

int printf(const char *fmt, ...)
{
	char buf[1000];
	int count = 0;
	
	va_list args;
	va_start(args, fmt);
	count = vsprintf(buf,fmt, args);
	va_end(args);
	putstring(buf);

	return count;
}