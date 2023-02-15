[![Build](https://github.com/lotusflare/lucas/actions/workflows/build.yml/badge.svg?branch=master)](https://github.com/lotusflare/lucas/actions/workflows/build.yml)

# Lucas

_Lucas_ is a Lua library which wraps the [DataStax Cassandra C/C++ driver](https://github.com/datastax/cpp-driver) using the Lua C API.

## Usage

See `integration/tests` for examples.

## Contributing

- Follow the [conventional commit message](https://www.conventionalcommits.org) specification
- Use integration tests to demonstrate usage and prevent regressions
- Contribute with docs
- Open an issue

## Scripts

`format.sh` checks the code has been formatted.
- `-f` auto fixes formatting errors.

`start.sh` brings up the testing & development environment.
- `-c` removes existing containers first (clears database).
- `-b` force rebuild of the containers first.
- `-t` run the tests after environment is up.
