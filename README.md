#function

###Overview

function is a preprocessor in C for C which enables javascript-like anonymous functions. For example, if we have as our main.c

<pre>#include <stdio.h>
$#

int main ()
{
	void (*fptr)() = $$ (printf, (const char *) "Hello, world!\n");
	(*fptr) ();
}</pre>

then run

<pre>$ ./function processed.c main.c
$ gcc processed.c</pre>

then we will have a binary which prints hello world. Here "processed.c" is the output of function.

###Usage

<pre>$$ (func, ...)</pre>

The special syntax function uses is the double dollar "$$" operator; this can't appear anywhere in your code (even in comments) apart from where you're using function. The $$ operator acts like a normal function -- its first argument is the function you want to use (call it func), and the rest are the arguments passed to func. The operator returns a function pointer (the function is of type void and has no arguments) which when called calls func with the arguments you passed it. There are a few restrictions:

 * all arguments must be explicitly cast
 * the operator "$#" must be placed on a line by itself somewhere. This is where the calling of func happens and so all necessary definitions must be done by this point (so above if I put the "$#" line before the inclusion of stdio.h gcc would warn me about implicit declarations of printf, for example).

Another example:

<pre>#include <stdio.h>
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
}</pre>

This will output "Success!\n" and exit.

