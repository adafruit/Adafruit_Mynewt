# Upgrading Projects

### Switch apache-mynewt-core versions

- Check the version you with to use for the core library in the
`repos/apache-mynewt-core/repository.yml` list.
- Open the `project.yml` file in your project root folder and then specify the
right version number for your core library and save (ex. 'vers: 0-dev').
- Upgrade your project via `newt upgrade`
