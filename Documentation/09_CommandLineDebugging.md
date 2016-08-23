# Command Line Debugging Tips

The following tips and tricks can be used when debugging from the command line.

## Grep'ing Source Code

The `grep` tool allows you to efficiently find references to strings in the
current folder or recursively, such as looking for every reference to a
specific function name or variable in your codebase.

### Grep recursively for a partial string

To perform a recursive search for a **partial string** enter:

```
$ grep -rn "./" -e "SEARCH_PATTERN"
```

- `r` means recursive
- `n` means show line numbers
- `e` is the string to search for (`SEARCH_PATTERN` in this case)

This will return a reference to any instance starting with `SEARCH_PATTERN`,
including the specific line number.

> NOTE: This search is **case-sensitive**. Adding the `i` flag will make
the search case insensitive.

### Grep recursively for an exact string

To perform a recursive search for an **exact match** of an entire string enter:

```
$ grep -rnw "./" -e "SEARCH_PATTERN"
```

- `w` means whole word

This will return a reference to any instances of `SEARCH_PATTERN`.

### Grep recursively for a string ignoring the case

If you wish the search to be **case insensitive** you can also add the `-i`
argument, as shown below:

```
$ grep -rni "./" -e "SeArCh_PaTtErN"
```

- `i` means case insensitive (default is case sensitive searching)

This would return 'Search_Pattern' or 'SEARCH_PATTERN' as valid matches.

### Grep recursively with specific file types

If you wish to restrict your search to a specific file type, you can use the
`--include` flag as follows:

```
$ grep --include=\*.{c,h} -rnw "./" -e "SEARCH_PATTERN"
```

This will only search files ending in .c or .h

You can also exclude certain file types with the `--exclude` flag:

```
$ grep --exclude=*.o -rnw "./" -e "SEARCH_PATTERN"
```

This will exclude all files ending in .o from the search.
