#HOT PI

AKA the best com-PI-ler out there.

#Github reminders

*Don't forget to `git pull` before doing anything in order to avoid merge conflicts. When in doubt, `git pull` before making any changes to the directory (editing files, etc.). Get into the habit of doing git pull as soon as you cd into the directory and before you leave it.*

#Getting up and running

Okay so if you would like your code to be considered as our base:

- First clone this repo
- then `git checkout p4_options`
#- just to make sure everything is up to date do `git rebase master`
- then copy your `csid_cs429_s17_p4` directory into this directory: `cp -r csid_cs429_s17_p4 pi`
- go into your p4 directory and run the command `rm -rf .git`
- then commit with your full human name in the commit message somewhere (i.e "Rushi Shah's p4 submission")
- and push to `p4_options`
- checkout master to get back to the normal master branch