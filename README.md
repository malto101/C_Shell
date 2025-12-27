# Basic Lifeline of a Shell

a shell does 3 things.

- initialize: read and execute its configuration files.
- interpret: reads cmds from stdin (interactively or from a script) and executes them.
- terminate: after its cmds are executed, the shell executes any shutdown routines and exits.

## Initialization

for this tutsm it will be simple, there wont be any config files and there wont be any shutdown cmds, we will just have a looping func anf then terminate when the user wants to exit.

## Basic loop of the shell

a simple way to handle cmds is with 3 steps

- read: read the input from stdin.
- parse: break the input into cmds and args.
- execute: run the cmds.

1. read input

reading input from stdin is not simple in C. as we dont know how much text will be typed into thier shell, you cant't simply allocate a block and hope they dont exceed it. Instead, you will need to start with a block and if they do exceed it, reallocate with more space. this can also be done using getline().

2. parsing the line

In our current implementation, we wont be handling quoting and backslashes excaping in our cmd line. for now we will just split the line into tokens based on whitespace. later we will add more complex parsing.

This means **echo "this message"** would not call echo with a single argument this message, but rather it would call echo with two arguments: **this** and **message**.

At the start of **dsh_split_line** we begin tokenizing by calling strtok. it wreturns a pointer to the first token. what strtok() actually does is return the points to within the string you give it and place \0 bytes at the end of each token, at which point we null terminate the list of tokens

3. executing cmds

There are two ways to start a new procedd in UNIX, 

the first one(which almost doesnt count) is by being **init**. when a unix computer boots up, the kernel starts the **init** process, which is the ancestor of all other processes. it is started with a special syscall and not with fork() and exec().

the second way is to use the **fork()** and **exec()** syscalls. **fork()** creates a copy of the current process, while **exec()** replaces the current process image with a new process image.

there are many variants of **exec()**, but they all do the same thing: load a binary into memory and run it. in our shell, we will be using **execvp()**, which is a variant that searches for the binary in the directories listed in the PATH environment variable.


To-Do

1. add support for double quotes and back slash escaping in the cmd line parser.(completed)
2. add support for new line(completed)
3. add support for cursor movement(completed)
4. add support for history(completed)
5. add support for auto complete