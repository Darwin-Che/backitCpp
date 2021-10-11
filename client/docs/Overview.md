# Overview

The whole data section is maintained in several files under `data` folder. When each command runs, they first mmap the files into memory, and read the data which are laid out in a predefined manner. 

## Status 

1. local folder -> all contents are untracked/local
2. syncless folder -> all contents are untracked/local/sync/remote
3. syncmore folder -> all contents are untracked/local/sync/remote
4. remote folder -> all contents are remote



## Data Structure

`super` stores the description of the other files

`mdt` stores all tracked files, mainly used to navigate among the directories and files.

`fname` stores the filenames when mdt's struct cannot accomodate

`dirmap` stores tables that map filename to index in mdt

## Operations

### `sync file`

```
for folder relative to root:
    if no status found, make local
    check folder's status:
        case remote:
            inform the user, and change it to syncless
        case local:
            inform the user, and change it to syncless
        case syncless:
            do nothing
        case syncmore:
            do nothing

arrive at the last folder:
if no status found for the file, make local
check file's status:
    case remote:
        inform the user, and abort
    case local:
        inform the user, and change it to sync, push it
    case sync:
        inform the user, push it

```

### `sync folder`

```
for folder relative to root:
    if no status found, make local
    check folder's status:
        case remote:
            inform the user, and change it to syncless
        case local:
            inform the user, and change it to syncless
        case syncless:
            do nothing
        case syncmore:
            do nothing

arrive at the folder
if no status found, make it local
check folder's status:
    case remote:
        inform the user, and abort
    case local:
        inform the user to decide syncless/syncmore
    case syncless:
        untracked, local -> local
        sync -> push
        syncless, syncmore -> recursion
    case syncmore:
        untracked -> sync , syncmore
        local -> local
        sync -> push
        syncless, syncmore -> recursion

```

### `status --to .. file`

#### local -> sync
set sync; sync time is MIN

#### local -> remote
not allowed

#### sync -> remote
(push file and) then delete local copy

#### remote -> sync
pull file and set it to sync

#### remote -> local
not allowed

#### sync -> local
set local


### `status --to .. folder`

#### local -> syncless
content stays local

#### local -> syncmore
content all change from local -> sync/syncmore

#### local -> remote
not allowed

#### syncless -> syncmore
