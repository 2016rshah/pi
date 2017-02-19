#HOT PI

AKA the best com-PI-ler out there.

#Github reminders

*Don't forget to `git pull` before doing anything in order to avoid merge conflicts. When in doubt, `git pull` before making any changes to the directory (editing files, etc.). Get into the habit of doing git pull as soon as you cd into the directory and before you leave it.*

#How to do stuff:

Begin to do stuff:

- First clone this repo using `git clone https://github.com/2016rshah/pi.git` and enter the created directory.
- Use `git checkout -b <your branch name>` to create a new branch and switch to it.
- You can now make commits to this branch without modifying `master`.

After someone else does something, you may want to incorporate that into your branch. To do so:

- Use `git checkout master` to switch to the master branch
- Use `git pull` to receive a local copy of the changes.
- Use `git checkout <your branch name>` to switch back to your branch
- Use `git rebase master` while on your branch to apply the new changes to your branch. You may have to resolve some conflicts.

Once you have done some things that work:

- Make sure that you have incorporated the latest version of `master` into your branch.
- Switch to the master branch using `git checkout master`.
- Add your changes to master by using `git merge master`.
- Use `git push` to push the changes to the repository.
- If you are going to add more changes, switch back to your branch.

#Guidelines:

##General
- `master` should only contain functioning code

##Style 
If anyone cares to maintain the style of my original code:
- Indent with 4 spaces
- `{` on same line
- newline before `}` always
- no newline between `}` and `else`
- space between relevant keywords (`if`, `while`) and `(`
- no space between function name and `(`
- variables, structs, and unions use `_` separated lowercase
- function names are in camelCase with the initial letter in lowercase

##Documentation
- Tokenization
  - The input is converted into a list of tokens. 
  - To add a new token, create a new entry in the `token_type` enum and add a conditional case inside `getToken`. If the token is a keyword, the additional case should be inside the `islower` case.
- Variable Namespace
  - The variable namespace is handled by tries.
  - Global and local namespaces have different tries. The global namespace root is pointed to by `global_root_ptr`. Local namespaces are discarded after each function is parsed.
  - `var_num` is a variable regarding the state of the variable. It is 0 if the variable is never referenced (the node was created because one of its children was referenced). For a global variable, it is 1 if it is referenced. In the local namespace, it is the parameter number (first argument is has `var_num` 1). 
  - If variables need to be associated with additional information (types?), that information should probably be added to `trie_node`.
- Expression Evaluation
  - `expression` causes the result of the expression evaluation to be placed in %rax and maintains the values of all other registers.
  - `e4` places its result in %r15 and may modify %r12, %r13, and %r14.
  - `e3` places its result in %r14 and may modify %r12 and %r13.
  - `e2` places its result in %r13 and may modify %r12.
  - `e1` places its result in %r12.
- Function Calls
  - Parameters are located on the top of the stack in reverse order before the function is called (parameter 1 is at %rsp, parameter 2 is at %rsp + 8, and so on before the function is called).
  - At the beginning of each function call, the original value of %rbp will be stored, and %rbp will be set to the address of the old %rbp (the address after the return value; if parameter 7 exists it will be located at %rbp + 16). %rbp is restored at the end of the function call.
