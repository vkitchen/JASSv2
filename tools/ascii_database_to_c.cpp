/*
	ASCII_DATABASE_TO_C.CPP
	-----------------------
	Copyright (c) 2016 Andrew Trotman
	Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
	
	Generate a set of table-driven methods with the same behaviour as the C runtime library
	ctype methods.  These methods assume plain 7-bit ASCII and so are locale ignoring.
*/
/*!
	@file
	@brief Generate C sourcecode for is() methods for ASCII
	@author Andrew Trotman
	@copyright 2016 Andrew Trotman

*/
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/*
	List of characteristics about a character.
*/
enum character_type
	{ 
	UPPER = 1,			///< character is uppercase
	LOWER = 2, 			///< character is lowercase
	DIGIT = 4, 			///< character is a digit
	CONTROL = 8, 		///< character is a control character
	PUNC = 16, 			///< character is punctuation
	SPACE = 32, 		///< character is a whitespace
	HEX = 64, 			///< character is a hexadecimal digit
	DNA = 128,			///< character is a DNA base (i.e in: {ACTGactg})
	};

/*
	For the enum above, what is the internal name we wish to use in ascii.cpp?
*/
const char *names[] =
	{
	"UPPER",
	"LOWER",
	"DIGIT",
	"CONTROL",
	"PUNC",
	"SPACE",
	"HEX",
	"DNA",
	};

/*
	PRINT_THIS()
	------------
*/
/*!
	@brief Print out the table entries the given character
	@param ch [in] the characer to print the detals of
*/
void print_this(long ch)
{
const char **pos;				///< pointer to the name of the given bit
uint32_t times = 0;			///< used to determine whether or not a '|' should be printed

/*
	Walk through the bits and if we find one set then write out the name of the bit
*/
pos = names;
while (ch > 0)
	{
	if ((ch & 1) != 0)
		{
		printf("%s%s", times == 0 ? "" : "|", *pos);
		times++;
		}
	ch >>= 1;
	pos++;
	}
}

/*
	MAIN()
	------
	Generate ascii.cpp
*/
int main(void)
{
int ch;		///< The character currently being examined.

/*
	Output the file's preamble
*/
time_t ltime;
time(&ltime);

puts("/*");
printf("\tThis file was generated on %s", ctime(&ltime));
printf("\tIt was generated by ascii_database_to_c as part of the JASS build process\n");
puts("*/");

printf("#include <stdint.h>\n");
printf("#include <ctype.h>\n");
printf("#include \"ascii.h\"\n");

/*
	Output the to() methods's table
*/
printf("const uint16_t JASS::ascii::ctype[] =\n\t{");
for (ch = 0; ch <= 0xFF; ch++)
	{
	uint32_t bits;		///< What is the current truth for this character?

	bits = 0;

	/*
		work out which bits to set and set them
	*/
	if (isupper(ch))
		bits |= UPPER;
	if (islower(ch))
		bits |= LOWER;
	if (isdigit(ch))
		bits |= DIGIT;
	if (iscntrl(ch))
		bits |= CONTROL;
	if (ispunct(ch))
		bits |= PUNC;
	if (isspace(ch))
		bits |= SPACE;
	if (isxdigit(ch))
		bits |= HEX;
	if (tolower(ch) == 'a' || tolower(ch) == 't' || tolower(ch) == 'c' || tolower(ch) == 'g')
		bits |= DNA;
		
	/*
		newline every 16 characters
	*/
	if (ch % 16 == 0)
		printf("\n\t");

	/*
		dump out the bits's names or a 0
	*/
	if (bits == 0)
		printf("0");
	else
		print_this(bits);

	/*
		add a comma unless we're done
	*/
	if (ch != 0xFF)
		printf(", ");
	}
printf("\n\t};\n\n");

/*
	now do the lowercase table
*/
printf("const uint8_t JASS::ascii::lower_list[] =\n\t{");
for (ch = 0; ch <= 0xFF; ch++)
	{
	if (ch % 16 == 0)
		printf("\n\t");

	/*
		we just output the
	*/
	printf("0x%02X", ::tolower(ch));

	/*
		add a comma unless we're done
	*/
	if (ch != 0xFF)
		printf(", ");
	}
printf("\n\t};\n\n");

/*
	now do the uppercase table
*/
printf("const uint8_t JASS::ascii::upper_list[] =\n\t{");
for (ch = 0; ch <= 0xFF; ch++)
	{
	if (ch % 16 == 0)
		printf("\n\t");

	/*
		we just output the
	*/
	printf("0x%02X", ::toupper(ch));

	/*
		add a comma unless we're done
	*/
	if (ch != 0xFF)
		printf(", ");
	}
printf("\n\t};\n\n");


return 0;
}

