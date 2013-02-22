#function

###Overview

function is a preprocessor in C for C which enables javascript-like anonymous functions. For example, if we have

```C
/* main.c */

#include <stdio.h>
$#

int main ()
{
	$$ ($. (printf, (const char *) "Hello, world!\n"));
}
```

function is run as follows:

<pre>$ ./function processed.c main.c # processed.c is the output
$ gcc -o processed processed.c # compile it
$ ./processed
Hello, world!
$ </pre>

###Usage

<pre>$ ./function output_file.c input_file.c</pre>

<pre>global dumping ground: $#
function pointer: $$
function argument: $. (callback, ...)</pre>

There are three special syntax things which function uses. The first, "$#", is the dumping ground for definitions which function uses. It must be placed after the definition of the callback function. The second, "$$", denotes the function pointer which cann be called. Lastly, "$." is the argument which must be passed to "$$". "$." is a function which takes as its first argument a callback and the rest of its arguments are parameters to be passed to the callback function. For example:

```C
void mycallback (int arg1, int arg2, char *arg3)
{
	printf ("Arguments: %d %d %s\n", arg1, arg2, arg3);
}
$#

void myfunction ()
{
	void (*func) (void *) = $$;
	void *arg = $. (mycallback, (int) 10, (int) -200, (char *) "Hello World!");
	func (arg);
}
```

is a valid snippet. Calling ```myfunction()``` will, predictably, echo "Arguments: 10 -200 Hello World\n".

Some things to note:

 * "$#" must appear before the other things in the file, and must appear only once;
 * "$$" must appear in the file before "$.";
 * "$$" and "$." must be paired; there must be the same number of "$$"s and "$."s in the file;
 * all arguments to "$." *must be explicitly cast* as in the example, spacing doesn't matter but brackets do;
 * "$$" and "$." may be used as many times as you like.

Sorry for the numerous restrictions; the casting one is especially annoying but function has no way of determining the types of the arguments without explicit casts.

Another example:

```C
/* input.c */

#include <stdio.h>
#define SUCCESS 1
$#

typedef void (*callback) ();

void try_this (callback win, void *winarg, callback lose, void *losearg)
{
	if (SUCCESS)
		(*win) (winarg);
	else
		(*lose) (losearg);
}

int main ()
{
	try_this ($$, $. (printf, (const char *) "Success!\n"), $$, $. (printf, (const char *) "Failure.\n"));
	exit (0);
}
```

<pre>$ ./function output.c input.c
$ gcc -o output output.c
$ ./output
Success!
$ </pre>

###Build instructions

Very simple:

<pre>$ gcc -o function function.c vector.c</pre>

