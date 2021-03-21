[![Crawler Status](https://github.com/sqd/scooble_search/actions/workflows/crawler-check.yml/badge.svg)](https://github.com/sqd/scooble_search/actions/workflows/crawler-check.yml)

### How to make updates to the code repository:
1. Create a new branch with `git checkout -b <new-branch-name>`
2. Add and commit your changes.
3. Right before pushing you changes, run `git merge master` on you dev branch to make sure it is updated. Resolve and commit any merge comflicts that arise.
4. Push your branch with `git push --set-upstream origin <new-branch-name>`
5. Go to gitub and click the 'New pull request' button next to your branch name on the repository.
6. Assign whoever you want to review it, and wait for one/two code reviews before merging into the master branch(this may involve several rounds of code review and additional changes).
7. Once the branch is merged into master by a reviewer, delete the remote and local branch you used for development.

### Using the Custom STL:
Make sure to include the full path of the libraries you want to include. For example, in the crawler you may need to include the string library with:
```
#include "../stl/include/String.h"
```

### Style for Everything:
Make sure that you format your changes according to the style guide with [this tool](https://github.com/sqd/eecs440-format).
In short:

To format a few files, run: `path-to-clang-format-440 --style=EECS440 -i file1.cpp file2.h`

To format a whole directory (this could be slow): `find path_to_you_directory -name '*.cpp' -o -name '*.o' | xargs path-to-clang-format-440 --style=EECS440 -i`

Note: Add this to your `~/.bashrc` if you want to be able to run `formatter [filename]` to format any file:
```
formatter ()
{
    ./clang-format-440 -style=EECS440 -i "$1" "$1"
}
```

### TODOS:
Add TODOS wherever we will need to change/update things later! Consider keeping a list of them at the top of each file(that is what we are doing for the crawler) to make sure we don't forget anything. For example, we are adding a TODO to handle all the thrown exceptions from the crawler code once we have multiple threads/workers running it.
