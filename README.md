[![Crawler Status](https://github.com/sqd/scooble_search/actions/workflows/crawler-check.yml/badge.svg)](https://github.com/sqd/scooble_search/actions/workflows/crawler-check.yml)

### How to make updates to the code repository:
1. Create a new branch with ```git checkout -b <new-branch-name>```
2. Add and commit your changes.
3. Right before pushing you changes, run ```get merge master``` on you dev branch to make sure it is updated. Resolve and commit any merge comflicts that arise.
4. Push your branch with ```git push --set-upstream origin <new-branch-name>
5. Go to gitub and click the 'New pull request' button next to your branch name on the repository.
6. Assign whoever you want to review it, and wait for two code reviews before merging into the master branch(this may involve several rounds of code review and additional changes).
7. Once the branch is merged into master by a reviewer, delete the remote and local branch you used for development.

### Style for Everything:
We will go by our conventions for now and then reformat to the class style when we are close to finished.

### TODOS:
Add TODOS wherever we will need to change/update things later! Consider keeping a list of them at the top of each file(that is what we are doing for the crawler) to make sure we don't forget anything. For example, we are adding a TODO to handle all the thrown exceptions from the crawler code once we have multiple threads/workers running it.
