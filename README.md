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

- Use `git fetch` to receive a local copy of the changes.
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

##Style (if anyone cares to maintain the style of my original code)
- Indent with 4 spaces
- `{` on same line
- newline before `}` always
- no newline between `}` and `else`
- space between relevant keywords (`if`, `while`) and `(`
- no space between function name and `(`
- variables, structs, and unions use `_` separated lowercase
- function names are in camelCase with the initial letter in lowercase