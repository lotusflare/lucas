# Lucas

_Lucas_ is a Lua library which wraps the [DataStax Cassandra C/C++ driver](https://github.com/datastax/cpp-driver) using the Lua C API.

## Usage

See `integration/tests` for examples.

## Scripts

`format.sh` will check the code has been formatted.
- `-f` auto fixes formatting errors.

`run.sh` will build and test the code.

- `-c` removes existing containers first (clears database).
- `-b` force rebuild of the containers.
- `-t` run the tests.
