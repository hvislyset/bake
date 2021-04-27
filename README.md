# Bake

A lightweight version of the Unix utility [make](<https://en.wikipedia.org/wiki/Make_(software)>) with support for project dependencies that are web-based resources.

This is a past project for UWA's Systems Programming unit, refer to the original project specification [here](https://teaching.csse.uwa.edu.au/units/CITS2002/past-projects/p2018/summary.php).

## Setup

Run `make` to generate the executable. You can use the generated bake executable to recompile the project thereafter.

## Usage

Bake can be invoked with the following optional commands. If a target is not specified, bake will run using the default target (which is the first target defined in the Bakefile).

```
Usage: bake [options...] <target>

        -C directoryname,    read the specification file from the specified directory
        -f filename,         instead of reading from Bakefile or bakefile, read from the indicated file
        -i,                  ignore unsuccessful termination of actions
        -n,                  print each action without executing it
        -p,                  print the internal representation of the specification file
        -s,                  execute silently
```

## License

[GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/)
