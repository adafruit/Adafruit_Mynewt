# Mynewt Project Structure

Mynewt projects are made up of the following components:

## Project(s)

A project is the base directory of your codebase, and is defined by the
presence of a file name `project.yml`.

Projects will normally contain one or more packages and targets, as described
below, and at least one dependency on `apache-mynewt-core`.

Project dependencies are stored in the `repos` sub-folder.

A sample project.yml file can be seen below:

```
project.name: "my_project"

project.repositories:
    - apache-mynewt-core

# Use github's distribution mechanism for core ASF libraries.
# This provides mirroring automatically for us.
#
repository.apache-mynewt-core:
    type: github
    vers: 0-latest
    user: apache
    repo: incubator-mynewt-core
```

The `project.repositories` field contains all remote repositories that this
project depends upon, including the version required for the dependency.

The version details an information to retrieve these repositories is defined
elsewhere, for example in the `repository.apache-mynewt-core` entry above which
points to the core mynewt codebase, which will be downloaded from the specified
github repo.

## Package(s)

A package is a collection of logical items for the mynewt OS, defined by the
presence of a `pkg.yml` file, and can be one of the following pkg.types:

- **app** - An application, stored in the `apps` folder
- **libs** - A library, stored in the `repos` folder
- **compiler** - Compiler definition, stores in the `compiler` folder
- **target** - A build target, stored in the `targets` folder

A sample pkg.yml file for a `blinky` application can be seen below:

```
pkg.name: apps/blinky
pkg.type: app
pkg.description: Basic example application which blinks an LED.
pkg.author: "Apache Mynewt <dev@mynewt.incubator.apache.org>"
pkg.homepage: "http://mynewt.apache.org/"
pkg.keywords:

pkg.deps:
    - "@apache-mynewt-core/libs/os"
    - "@apache-mynewt-core/hw/hal"
    - "@apache-mynewt-core/libs/console/full"
```

Packages can rely upon other packages, and the list of packages in `pkg.deps`
will be included in the project (including header and source files, and any
included definitions).

## Target(s)

A target is a collection of parameters that must be passed to `newt` to produce
a binary output, and behaves similarly to make targets (ex. `make bootlaoder`,
`make tester`, etc.)

Anything defined in the target cascades down to all dependencies.

Targets are Packages, and are stored in the `targets/` sub-folder.

A target package can also define flags for the compiler, such as the following:

```
pkg.cflags:
    - "-DSTATS_NAME_ENABLE"
```
