#function

###Overview

function is a preprocessor in C for C which enables javascript-like anonymous functions. For example, if we have

```C
/* main.c */

#include &lt;stdio.h&gt;
$#

int main ()
{
	void (*fptr)() = $$ (printf, (const char *) "Hello, world!\n");
	(*fptr) ();
}
```

then run

<pre>$ ./function processed.c main.c
$ gcc -o processed processed.c
$ ./processed
Hello, world!
$ </pre>

and we're done.

###Usage

<pre>$$ (func, ...)</pre>

The special syntax function uses is the double dollar "$$" operator; this can't appear anywhere in your code (even in comments) apart from where you're using function. The $$ operator acts like a normal function -- its first argument is the function you want to use (call it func), and the rest are the arguments passed to func. The operator returns a function pointer (the function is of type void and has no arguments) which when called calls func with the arguments you passed it. There are a few restrictions:

 * all arguments must be explicitly cast
 * the operator "$#" must be placed on a line by itself somewhere. This is where the calling of func happens and so all necessary definitions must be done by this point (so above if I put the "$#" line before the inclusion of stdio.h gcc would warn me about implicit declarations of printf, for example).

Another example:

```C
/* input.c */

#include &lt;stdio.h&gt;
#define SUCCESS 1
$#

typedef void (*callback) ();

void try_this (callback win, callback lose)
{
	if (SUCCESS)
		(*win) ();
	else
		(*lose) ();
}

int main ()
{
	try_this ($$(printf, (const char *) "Success!\n"), $$(printf, (const char *) "Failure.\n"));
	exit (0);
}
```

<pre>$ ./function output.c input.c
$ gcc -o output output.c
$ ./output
Success!
$ </pre>

###Build instructions

<pre>$ gcc -c -o vector.o vector.c
$ gcc -c -o function.o function.c
$ gcc -o function function.o vector.o</pre>

