package = "lucas"
version = "dev-1"
source = {
   url = "git+https://github.com/lotusflare/lucas.git"
}
description = {
   detailed = "_Lucas_ is a Lua library which wraps the [DataStax Cassandra C/C++ driver](https://github.com/datastax/cpp-driver) using the Lua C API.",
   homepage = "https://github.com/lotusflare/lucas",
}
build = {
   type = "cmake",
   modules = {
      lucas = {
         sources = "src/library.c"
      },
   }
}
