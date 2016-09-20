# Upgrading Projects

### Switch apache-mynewt-core versions

- Check the version you with to use for the core library in the
`repos/apache-mynewt-core/repository.yml` list.
- Open the `project.yml` file in your project root folder and then specify the
right version number for your core library and save (ex. 'vers: 0-dev').
- Upgrade your project via `newt upgrade`

# Updating `newt` and `newtmgr`

To update the `newt` and `newtmgr` tools:

- Go to `$GOPATH/src/mynewt.apache.org/newt/`
- Run `git pull` to get the latest source code for the tools
- Switch branches if necessary (ex. `git checkout develop`)
- Go into the `newtmgr` sub-folder
- Run `go install`
- Repeat the process from the `newt` and `newtvm` folders

This will cause the tools in $GOPATH/bin to be rebuilt, which you can see
by running `ls -l $GOPATH/bin`
