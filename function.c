/* function.c */

#include <stdio.h>
#include <stdlib.h>

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

int main (int argc, char **argv)
{
	if (argc != 3)
		exit (1);
	FILE *ifile = fopen (argv[2], "rb");
	FILE *tfile = tmpfile ();
	FILE *cfile = tmpfile ();
	FILE *ofile = fopen (argv[1], "wb");
	char c = ' ';
	int FNUM = 0, i;
	fprintf (cfile, "#include <string.h>\n#include <stdint.h>\n#include <stdlib.h>\ntypedef void (*TYPFUNC) ();\n");
	while (!feof(ifile))
	{
		while (!feof(ifile))
		{
			c = fgetc (ifile);
			while ((!feof(ifile)) && c != '$')
			{
				fputc (c, tfile);
				c = fgetc (ifile);
			}
			if (feof (ifile))
				break;
			if (fgetc (ifile) == '$')
				break;
			fputc ('$', tfile);
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
			fprintf (stderr, "function: extected '('\n");
			exit (1);
		}
		fprintf (tfile, "FUNCTION%d (", ++FNUM);
		Vector tokens = tokenise(ifile);
		for (i = 2; i < tokens->len; ++ i)
		{
			output (tfile, v_at(tokens, i));
		}
		Vector types = argify (tokens);
/*		printf ("\nSTART\n");
		for (i = 0; i < types->len; ++ i)
		{
			printf ("%s\n", v_at(types, i));
		}
		printf ("END\n");*/
		fprintf (cfile, "void *FUNCTION%ddata;\nvoid FUNCTION%dcallback ()\n{\n\t", FNUM, FNUM);
		for (i = 0; i < types->len; ++ i)
			fprintf (cfile, "%s var%d; ", v_at (types, i), i);
		fprintf (cfile, "\n\tuintptr_t i = (uintptr_t)FUNCTION%ddata;\n", FNUM);
		for (i = 0; i < types->len; ++ i)
			fprintf (cfile, "\tmemcpy (&var%d, (void*)i, sizeof(var%d)); i += sizeof(var%d);\n", i, i, i);
		fprintf (cfile, "\tfree (FUNCTION%ddata);\n\t", FNUM);
		output (cfile, v_at (tokens, 0));
		fprintf (cfile, "(");
		for (i = 0; i < types->len; ++ i)
			fprintf (cfile, "%svar%d", (i>0)?", ":"", i);
		fprintf (cfile, ");\n}\n");
		fprintf (cfile, "TYPFUNC FUNCTION%d (", FNUM);
		for (i = 0; i < types->len; ++ i)
			fprintf (cfile, "%s%s var%d", (i>0)?", ":"", v_at (types, i), i);
		fprintf (cfile, ")\n{\n");
		fprintf (cfile, "\tFUNCTION%ddata = malloc (", FNUM);
		for (i = 0; i < types->len; ++ i)
			fprintf (cfile, "%ssizeof (var%d)", (i>0)?" + ":"", i);
		fprintf (cfile, ");\n");
		fprintf (cfile, "\tuintptr_t i = (uintptr_t)FUNCTION%ddata;\n", FNUM);
		for (i = 0; i < types->len; ++ i)
			fprintf (cfile, "\tmemcpy ((void*)i, &var%d, sizeof(var%d)); i += sizeof(var%d);\n", i, i, i);
		fprintf (cfile, "\treturn &FUNCTION%dcallback;\n}\n\n", FNUM);
	}
	fseek (tfile, 0, SEEK_SET);
	while (!feof(tfile))
	{
		c = fgetc (tfile);
		while ((!feof(tfile)) && c != '$')
		{
			fputc (c, ofile);
			c = fgetc (tfile);
		}
		if (feof (tfile))
			exit (0);
		if (fgetc (tfile) == '#')
			break;
		fputc ('$', tfile);
		fseek (ifile, -1, SEEK_CUR);
	}
	fseek (cfile, 0, SEEK_SET);
	c = fgetc (cfile);
	while (!feof(cfile))
	{
		fputc (c, ofile);
		c = fgetc (cfile);
	}
	c = fgetc (tfile);
	while (!feof(tfile))
	{
		fputc (c, ofile);
		c = fgetc (tfile);
	}
	exit (0);
}
