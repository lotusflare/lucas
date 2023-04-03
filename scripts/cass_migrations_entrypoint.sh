#!/bin/bash
set -e
set -x
/usr/local/bin/cqlsh cassandra2 9042 -p cassandra -u cassandra -f /app/integration/migrations/*