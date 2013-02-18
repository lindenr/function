function is a preprocessor in C for C which enables javascript-like anonymous functions. For example:

<pre>void (*fptr)() = $$ (printf, (const char *) "Hello, world!\n");
(*fptr) ();</pre>

would become something ugly after preprocessing and require an include somewhere of the output header. Ta-da.

