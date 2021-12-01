# Starter

Create a data structure for directory entry in the remote. - transfer the entries into this in-memory structure from either internet connection / disk read. (We can even supply this from a local directory at this stage)

Then we can have a collection of such entries

Consider How to merge this collection with current working directory? 

First, consider displaying with `ls` command. 

( Then display on the pClui (curse.h) lib we had. )

The key is the remote files must have proper mtime to denote the last sync with local.

# Data format when tranfering over internet (byte streams)

Always use the `reporel` when transferring the fname
Let the program figure out `repoabs`, and then concatenate to get `fsabs`.


# Directory list

```
{time_64}{fname}\n
```

repeatedly.

# Files

## Paths 

```
{num_files_64}

Each file

{fname}\n
```

## Content

```
{num_files_64}

Each file

{fname}\n
{time_64}{filesz_64}{content}

```

