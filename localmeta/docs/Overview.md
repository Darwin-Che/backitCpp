# Overview

The whole data section is maintained in several files under `.backit` folder at the root of a depot. When each command runs, they first mmap the files into memory, and read the data which are laid out in a predefined manner. 

## Tracking

Local tracks the tree structure of the shared depot. This shared depot is stored plainly under a folder in the server's file system. 

The server doesn't keep track of which files are present at client. 

Need a way to exchange the changes between client and server. 

Current thought : Clearly the exchange must be initiated by the client. At this moment, the server and client might diverged. Since the server side change is already written. The better order is to merge the changes by the order of server then client. 

Now restrict the changes allowed in the server side. Modification is not allowed. Only Add/Rmv is allowed. 

There are four tracked status in local : sync, syncless, syncmore, remote.

Restrict the changes allowed in the local side without connecting to the server. Here the motivation is that the only thing the local wants to do to the server is to overwrite the updated syncfiles (including newly introduced files). 

Question : clearly remote -> sync needs download; do we allow sync -> remote to happen in offline mode?

Answer : No, we want the result to happen immediately, i.e. we want to delete the local copy immediately after the command, but there can be new changes made which are lost if not uploaded to the remote. 

So the changes allowed in offline local side is 

1. untracked -> sync
2. sync -> untracked
3. modify the sync

Any manipulation on the local file system is allowed, and `update` can detect these changes and record them offline. 

What happens when we delete a sync file? The new changes are lost. On the next `update`, the file's tracking status goes from sync -> remote (informing the user), and if we want to actually delete the remote, we have to be online mode and 


Can we simply apply those on the mdt tracking at local? Actually yes, and before the client's changes are pushed to server. When we apply the remote changes, every Add is explained as pure remote (therefore only add the dir if a dir is created). Conflicts might appear because of synced objects are added locally 

## Data Structure

`super` stores the description of the other files

`mdt` stores all tracked files, mainly used to navigate among the directories and files.

`fname` stores the filenames when mdt's struct cannot accomodate

`dirmap` stores tables that map filename to index in mdt

## Operations

Modify the depot at both client and server.


### `update .`

update the tracking info here

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
