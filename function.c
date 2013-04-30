/* function.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vector.h"

#define TOK_NONE   0
#define TOK_STRING 1
#define TOK_OPENB  2
#define TOK_CLOSEB 3
#define TOK_COMMA  4

struct TString
{
	int type;
	char *string;
};

union Token
{
	int type;
	struct TString str;
};

char *getstring (FILE *ifile)
{
	int i;
	union Token t;
	char c = fgetc (ifile);
	char *str = malloc(256);
	int counter = 1;
	for (i = 0; (!feof(ifile)); ++ i)
	{
		if (c == '(')
			++ counter;
		else if (c == ')')
			-- counter;
		if (c == ',' && counter <= 1)
			break;
		if (counter <= 0)
			break;
		str[i] = c;
		c = fgetc (ifile);
	}
	str[i] = 0;
	return str;
}

Vector tokenise (FILE *ifile)
{
	Vector result = v_dinit (sizeof(union Token));
	char c;
	union Token t;
	int i;
	char *str;
	int counter = 1;
	while ((!feof(ifile)) && c != '$')
	{
		c = fgetc (ifile);
		if (isspace(c))
			continue;
		else if (c == ')')
		{
			t.type = TOK_CLOSEB;
			//printf(")\n");
			-- counter;
			if (counter <= 0)
				c = '$';
		}
		else if (c == '(')
		{
			t.type = TOK_OPENB;
			//printf("(\n");
			++ counter;
		}
		else if (c == ',')
		{
			t.type = TOK_COMMA;
			//printf(",\n");
		}
		else
		{
			fseek (ifile, -1, SEEK_CUR);
			str = getstring (ifile);
			t.str.type = TOK_STRING;
			t.str.string = str;
			//printf ("%s\n", str);
			fseek (ifile, -1, SEEK_CUR);
		}
		v_push (result, &t);
	}
	return result;
}

void output (FILE *ofile, union Token *t)
{
	switch (t->type)
	{
		case TOK_NONE:
			break;
		case TOK_OPENB:
		{
			fprintf (ofile, " (");
			break;
		}
		case TOK_CLOSEB:
		{
			fprintf (ofile, ") ");
			break;
		}
		case TOK_COMMA:
		{
			fprintf (ofile, ", ");
			break;
		}
		case TOK_STRING:
		{
			fprintf (ofile, "%s", t->str.string);
			break;
		}
	}
}

Vector argify (Vector tokens)
{
	int i;
	union Token *t;
	int flag = 0;
	Vector types = v_dinit (100);
	for (i = 0; i < tokens->len; ++ i)
	{
		t = v_at (tokens, i);
		if (flag)
		{
			flag = 0;
			v_pstr (types, t->str.string);
		}
		if (t->type == TOK_OPENB)
			flag = 1;
	}
	return types;
}

#define PT(str) pt(incfile, FNUM, tokens, types, str)
#define LP(str) lp(incfile, FNUM, tokens, types, str)

void pt (FILE *file, int FNUM, Vector tokens, Vector types, const char *str)
{
	char out[1000];
	char FUNC[50];
	sprintf (FUNC, "FUNCTION%d", FNUM);
	int i, cur, len = strlen(str);
	for (i = 0, cur = 0; i < len; ++ i)
	{
		if (str[i] != '$')
		{
			out[cur] = str[i];
			++ cur;
			continue;
		}
		switch (str[i+1])
		{
			case 'F':
			{
				strcpy ((char*)(out+cur), FUNC);
				cur += strlen(FUNC);
				break;
			}
			case 'A':
			{
				union Token *t = v_at (tokens, 0);
				strcpy ((char*)(out+cur), t->str.string);
				cur += strlen (t->str.string);
				break;
			}
		}
		++ i;
	}
	out[cur] = 0;
	//fprintf (stderr, "%s", out);
	fprintf (file, "%s", out);
}

void lp (FILE *file, int FNUM, Vector tokens, Vector types, const char *str)
{
	int i;
	char out[1000];
	for (i = 0; i < types->len; ++ i)
	{
		int j, len = strlen(str), k;
		char NUM[50]; sprintf(NUM, "%d", i);
		for (j = 0, k = 0; j < len; ++ j)
		{
			if (str[j] != '$')
			{
				out[k] = str[j];
				++ k;
				continue;
			}
			switch (str[j+1])
			{
				case 'N':
				{
					strcpy ((char*)(out+k), NUM);
					k += strlen (NUM);
					break;
				}
				case 'T':
				{
					strcpy ((char*)(out+k), v_at (types, i));
					k += strlen (v_at (types, i));
					break;
				}
				case 'C':
				{
					if (i == (types->len - 1))
						break;
					out[k] = ',';
					++ k;
					break;
				}
			}
			++ j;
		}
		out[k] = 0;
		pt (file, FNUM, tokens, types, out);
	}
}

int main (int argc, char **argv)
{
	if (argc != 3)
		exit (1);
	FILE *ifile = fopen (argv[2], "rb");
	FILE *incfile = tmpfile ();
	FILE *tofile = tmpfile ();
	FILE *ofile = fopen (argv[1], "wb");
	char c = ' ';
	int FNUM = 0, i;
	fprintf(incfile, "#include <malloc.h>\n");
	while (!feof(ifile))
	{
		while (!feof(ifile))
		{
			c = fgetc (ifile);
			while ((!feof(ifile)) && c != '$')
			{
				fputc (c, tofile);
				c = fgetc (ifile);
			}
			if (feof (ifile))
				goto DONE;
			c = fgetc (ifile);
			if (c == '.')
				break;
			else if (c == '$')
			{
				fprintf (tofile, "FUNCTION%dcallback", ++FNUM);
				continue;
			}
			fputc ('$', tofile);
			fseek (ifile, -1, SEEK_CUR);
		}
		if (feof (ifile))
			break;
		do
			c = fgetc (ifile);
		while ((!feof(ifile)) && isspace(c));
		if (feof (ifile))
			break;
		if (c != '(')
		{
			fprintf (stderr, "function: expected '('\n");
			exit (1);
		}
		fprintf (tofile, "FUNCTION%d (", FNUM);
		Vector tokens = tokenise(ifile);
		for (i = 2; i < tokens->len; ++ i)
		{
			output (tofile, v_at(tokens, i));
		}
		if (tokens->len <= 2)
			fprintf (tofile, ")");
		Vector types = argify (tokens);
		/*printf ("\nSTART\n");
		for (i = 0; i < types->len; ++ i)
		{
			printf ("%s\n", v_at(types, i));
		}
		printf ("END\n");*/
		PT("struct $Fdata\n{\n\t");
		LP("$T var$N; ");
		PT("\n};\nvoid $Fcallback (void *data)\n{\n\tstruct $Fdata *F = data;\n\t$A (");
		LP("F->var$N$C");
		PT(");\n\tfree(F);\n}\nvoid *$F(");
		LP("$T var$N$C");
		PT(")\n{\n\tstruct $Fdata *F = malloc(sizeof(*F));\n");
		LP("\tF->var$N = var$N;\n");
		PT("\treturn F;\n}\n\n");
	}
DONE:
	fseek (tofile, 0, SEEK_SET);
	while (!feof(tofile))
	{
		c = fgetc (tofile);
		while ((!feof(tofile)) && c != '$')
		{
			fputc (c, ofile);
			c = fgetc (tofile);
		}
		if (feof (tofile))
			exit (0);
		c = fgetc (tofile);
		if (c == '#')
			break;
		fputc ('$', tofile);
		fseek (ifile, -1, SEEK_CUR);
	}
	fseek (incfile, 0, SEEK_SET);
	c = fgetc (incfile);
	while (!feof(incfile))
	{
		fputc (c, ofile);
		c = fgetc (incfile);
	}
	c = fgetc (tofile);
	while (!feof(tofile))
	{
		fputc (c, ofile);
		c = fgetc (tofile);
	}
	exit (0);
}
