# syscfg

MyNewt uses the **syscfg.yml** file in the target's root folder to set certain
config values that are defined in other packages used by the target.

## Setting syscfg values with the `newt` tool

You can set syscfg values from the command line with the following command:

```
# Increase mbuf count to 16 for the specified target
$ newt target set <target-name> syscfg=MSYS_1_BLOCK_COUNT=16
```

## See all current syscfg settings for target

Running the following command will display a list of all syscfg values
defined by your target:

```
$ newt target config show <target-name>
```
