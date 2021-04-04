/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
//1
/* 
 * bitXor - x^y using only ~ and & 
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 1
 */
int bitXor(int x, int y) {
	int tmp1,tmp2;
	tmp1 = ~x & ~y;
	tmp2 = x&y;
  	return (~tmp1 & ~tmp2);
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
	int tmp = 1;
  return tmp<<31;

}
//2
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmax(int x) {  //??????
	int tmp1 = x+1;                                                                    
  return !(tmp1+tmp1) & (!!tmp1);
}
/* 
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allOddBits(int x) {
	//all the odd-numbered position(from 0 to 31) is set to 1 
	int checker = ((0xAA << 8 | 0xAA) << 16) | (0xAA << 8 | 0xAA);
	int tmp = checker & x;
  	return !(tmp ^ checker);
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return ~x + 1;
}
//3
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
	//if x in the range :0x30~0x39 solution1 failed~~~
	// int stdhigh = 3;
	// int stdlow = 9;
	// int high = x>>4;
	// int low = x & 0xff;

	// int tmphigh = ~stdhigh ^ high; //0xffffffff
	// int tmplow = stdlow ^ low; //010? 001?

	int cond1 = x+(~0x30+1); //x-0x30
	int cond2 = 0x39+(~x+1); //0x39-x


 	return !((cond1>>31) | (cond2>>31));
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  //if x!=0 :result==y,else result==z
	int tmp1 = !x+(~1+1);//1-1
	int tmp2 = !!x+(~1+1);//0-1
  	return (tmp1&y | tmp2&z);
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  //if x<=y result=1;else result=0
  int sign_x = x>>31;
  int sign_y = y>>31;
  int flag = sign_x ^ sign_y; //+ -?
  
  int result = (((y - x)>>31)&(!flag)) | (!sign_x)&flag&(sign_y); //yy-xx | 

  return !result;
	
	//z=1 : y<x z=0 x<=y
}
//4
/* 
 * logicalNeg - implement the ! operator, using all of 
 *              the legal operators except !
 *   Examples:  (3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int logicalNeg(int x) { //consider of neg numbers???
  	int sign = x>>31 & 1;
    int tmp = (sign^1) & (((~x+1)>>31 ^ x>>31)+~1+1); //if neg tmp==0;else if + tmp==0 if 0 tmp==1
    return tmp;
}
/* howManyBits - return the minimum number of bits required to represent x in
 *             two's complement
 *  Examples: howManyBits(12) = 5
 *            howManyBits(298) = 10
 *            howManyBits(-5) = 4
 *            howManyBits(0)  = 1
 *            howManyBits(-1) = 1
 *            howManyBits(0x80000000) = 32
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 90 
 *  Rating: 4
 */
int howManyBits(int x) {  //这题二分法和负数转换确实没想到，引用K神的~~~
	int bit16, bit8, bit4, bit2, bit1;
  // 注意,如果x为负，则x >> 31为0xffffffff
  int sign = x >> 31;
  // 因为正数记录从左到右第一个1,负数记录第一个0
  // 所以可以对操作数取反，将负数转为正数，正数不变，便于更好的计算
  x ^= sign;
  // 二分查找，先判断高16位有无存在1,并将范围缩小到高16位或低16位
  // 如果高16位存在0,则bit16 == 16,否则等于0
  bit16 = (!!(x >> 16)) << 4;
  x >>= bit16;
  // 查找剩余16位中的高8位是否存在1
  bit8 = (!!(x >> 8)) << 3;
  x >>= bit8;
  // 查找剩余8位中的高4位是否存在1
  bit4 = (!!(x >> 4)) << 2;
  x >>= bit4;
  // 查找剩余4位中的高2位是否存在1
  bit2 = (!!(x >> 2)) << 1;
  x >>= bit2;
  // 查找剩余2位中的高1位是否存在1
  bit1 = (!!(x >> 1)) << 0;
  x >>= bit1;
    // 注意return中要再加上一个符号位
  return bit16 + bit8 + bit4 + bit2 + bit1 + x + 1;


  
}
//float
/* 
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned floatScale2(unsigned uf) {
  //return 2*f f is the bit-level representation
  int s, k;
  int f;
  s = uf>>31;
  k = (uf<<1)>>24;
  if(k==0xff) //NaN
    return uf;
  if(k==0)
    return s<<31 | uf<<1; 
  else if(k!=0){
    k += 1;
    if(k==0xff)
      return s<<31 | 0x7f800000;
    else
      return s<<31 | (k << 23) | (uf & 0x7fffff);
  

  }
}
/* 
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int floatFloat2Int(unsigned uf) {  //这个地方没怎么看懂题目意思，以下也来自K神
  int exp = (uf >> 23) & 0xff;
  int sign = (uf >> 31) & 1;
  int frac = uf & 0x7fffff;
  int shiftBits = 0;
  // 0比较特殊，先判断0(正负0都算作0)
  if(!(uf & 0x7fffffff))
    return 0;
  // 判断是否为NaN还是无穷大
  if(exp == 0xff)
    return 0x80000000u;
  // 指数减去偏移量，获取到真正的指数
  exp -= 127;
  // 需要注意的是，原来的frac一旦向左移位，其值就一定会小于1，所以返回0
  if(exp < 0)
    return 0;
  // 获取M，注意exp等于-127和不等于-127的情况是不一样的。当exp != -127时还有一个隐藏的1
  if(exp != -127)
    frac |= (1 << 23);
  // 要移位的位数。注意float的小数点是点在第23位与第22位之间
  shiftBits = 23 - exp;
  // 需要注意一点，如果指数过大，则也返回0x80000000u
  if(shiftBits < 31 - 23)
    return 0x80000000u;
  // 获取真正的结果
  frac >>= shiftBits;
  // 判断符号
  if(sign == 1)
    return ~frac + 1;
  else
    return frac;
}
/* 
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 * 
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while 
 *   Max ops: 30 
 *   Rating: 4
 */
unsigned floatPower2(int x) {
  int k = x+127;
  if(k>0xfe)
    return 0x7f800000;
  else if(k<0)
    return 0;
  return k << 23;
}
