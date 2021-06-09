# Simple HTTP stress testing

This small C++ software has only one final scope: to have a simple backend to
be used to test the speed of PostgreSQL workloads when a new connection is
established per every request.


## Configuration

Environment variables are your friends:

* `DATABASE_URL` is used as a connection string (libpq is being used to
  communicate with the PostgreSQL server);

* `SQL_QUERY` is the SQL query that will be executed when accessing the `/tx`
  endpoint.

The endpoint `/.well-known/check` will always return "ok".

## Other considerations

As previously said, a new connection is established per request, without any
connection pool. Use pgbouncer if you need something like that.

## Docker image

A simple Docker image containing this utility can be found at
[quay.io](https://quay.io/repository/leonardoce/webtest).

## Thanks

https://github.com/yhirose/cpp-httplib is a great tool, really.
