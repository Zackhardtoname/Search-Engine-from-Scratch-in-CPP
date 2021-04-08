# Crawler Build

After setting up a new machine, to build the crawlers you must follow these steps in order. NOTE: Make sure you run steps 2-4 in the `scooble_search/bin` directory.

### 1) Clone the repo
Run `git clone https://github.com/sqd/scooble_search.git` or `git clone git@github.com:sqd/scooble_search.git` if you have ssh keys setup.

### 2) Install the necessary dependencies and setup file types
`./install-dependencies`

### 3) Build the code executables
Run `./build` and if you ever want to remove the executables or clean the workspace, run `./clean`

### 4) Run the crawlers
`./run-crawlers`

## Additional handling steps:

### Pause the crawlers
`./pause-crawlers`

### Continue/resume the crawlers
`./continue-crawlers`

### Shut down the crawlers and reset everything
`./shut-down-crawlers`

### Initialize the URL seed list
Run `./initialize-seed-list`. This should only be down on one machine one time when deploying.
