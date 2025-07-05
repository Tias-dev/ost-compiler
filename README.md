# ost-compiler
Compiler of OST high-level programing language of Turing's machine algorithmic model

## Warning about headed recursion
This OST compiler support only tailed recursion because headed recursion needs realization run-time stack for storing returning states for recursion calls. This needs splitting line to 2 parts(by odds and even tiles for example) for storing program data and for storing recursion stack. To be honest i don't really know how to realize it in terms of Machine Turing algorithmic model.

If you use not tailed recursion in program, no errors will be produced for this recursion usage but program will not work as you may imagine by other programming languages.

It can be demonstrated by following simple example.

Imagine you have alphabet A = { \_, 1, 0} and you should in the word before cursor shift left all zeros and ones by 1 tile before you meet lambda(\_).
Recursion realization in C language may be following
```c
void foo(const char * line, size_t* cursor) {
  if(line[*cursor] == '0') { // if statement exists in OST
    *cursor = *cursor - 1; // Shift MT pointer left aka <
    foo(line, cursor); // Not tailed recursion!
    line[*cursor] = '0'; // Setting 0 in current place, aka q,0,0,q1 ; q,1,0,q1 ; q,_,0,q1
  }
  else if(line[*cursor] == '1') {
    *cursor = *cursor - 1; // Shift MT pointer left aka <
    foo(line, *cursor); // Not tailed recursion!
    line[*cursor] = '1'; // Setting 0 in current place, aka q,0,1,q1 ; q,1,1,q1 ; q,_,1,q1
  }
}

int main(int argc, const char * argw[]) {
  size_t cursor = atoll(argw[1]);
  const char * line = argw[2];

  cursor = cursor - 1; // Shift MT pointer left aka < to first letter of the word
  foo(line, &cursor);
  while(line[cursor] != '_') cursor = cursor + 1; // Shift MT pointer right aka >
}
```

This program on input [\_1010\_101\_] will produce output [\_1010101\_] as we want

This program may be written in OST language as follows:
```
MT MAIN;
BEGIN
  ALPHABET: 0,1;

  MT FOO;
  BEGIN
    ALPHABET: 0,1;
    IF
      0 ? l ; FOO ; a(0)
      1 ? l ; FOO ; a(1)
    FI;
  END FOO;

  l; FOO;
  DO () != _ ? r; OD;
END MAIN
```
This program on input [\_1010\_101\_] will produce output [\_1010\_101\_] which is nothing

This happen via no recursion stack in ost exists by default so if mt FOO starts from state `qsFOO` and ends by `qeFOO` then any recursion call will step to `qsFOO` state, but when any of recursions will successfully ends mt FOO i.e. it will point on lambda(\_) then it will step to `qeFOO` state that will ends all recursions at all and continue executing from where mt FOO was called at the first time(i.e. from mt MAIN)

It may become clear to understood if we remember that recursion stack is run-time feature of programming language and while compiling ost program we don't know how much recursion steps will be needed.
