Basic Workflow
==============

1. Fork:
  - github: _Top right_ on https://github/clchab/hab_arduino

2. clone
  - github: ignore
  - github client: click clone to desktop
  - cli: `git clone https://github/<github name>/<fork>`

3. create branch: _optional_
  - github: create branch
  - github client: create branch
  - cli: `git checkout -b <branch>`

4. add changes
  - github: ignore
  - github client: ignore
  - cli: `git add <file>`

5. commit:  Please make good descriptive comments
  - github: should be automatic
  - github client: make a commit
  - cli: `git commit -m "<descriptive comment>"`

7. Merge branch with master on fork: skip if no branches were made
  - github: under pull requets
  - github client: branch > manage and merge
  - cli: `git checkout master && git merge <branch>`

8. update with clchab/hab_arduino
  - github: make a pull request
  - github client: see cli or github
  - cli `git pull upstream master`

9. push to github
  - github: ignore
  - github client: sync
  - cli: 'git push origin master`

10. merge with clchab
  - github: send a pull request

11. update with clchab/hab_arduino
  - github: make a pull request
  - github client: see cli
  - cli: `git pull upstream  master`

12. goto 3

## Git Documentation and tutorials:

  - http://git-scm.com/documentation
  - https://www.atlassian.com/git/tutorials/setting-up-a-repository
