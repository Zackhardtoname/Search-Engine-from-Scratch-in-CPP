[![Crawler Status](https://github.com/sqd/scooble_search/actions/workflows/crawler-check.yml/badge.svg)](https://github.com/sqd/scooble_search/actions/workflows/crawler-check.yml)

### Using the Custom STL:
Make sure to include the full path of the libraries you want to include. For example, in the crawler you may need to include the string library with:
```
#include "../stl/include/String.h"
```

### Style for Everything:
We will go by our conventions for now and then reformat to the class style when we are close to finished.

### TODOS:
Add TODOS wherever we will need to change/update things later! Consider keeping a list of them at the top of each file(that is what we are doing for the crawler) to make sure we don't forget anything. For example, we are adding a TODO to handle all the thrown exceptions from the crawler code once we have multiple threads/workers running it.
