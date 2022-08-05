/*
Copyright The CloudNativePG Contributors

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "httplib.h"
#include <chrono>
#include <ctime>
#include <iostream>
#include <libpq-fe.h>

void tx(const httplib::Request &req, httplib::Response &res);
void ok(const httplib::Request &req, httplib::Response &res);
void logger(const httplib::Request &req, const httplib::Response &res);
std::string get_configuration(const std::string &env_name,
                              const std::string &default_value = "");

/**
 * The database URL to be used as a libpq connection string
 */
const std::string database_url = get_configuration("DATABASE_URL");
const std::string sql_query = get_configuration("SQL_QUERY", "SELECT 1");

/**
 * Everything starts from here
 */
int main() {
  // Initial logging
  std::cout << "Database URL: " << std::quoted(database_url) << std::endl;
  std::cout << "SQL query: " << std::quoted(sql_query) << std::endl;
  std::cout.flush();

  // Starting HTTP server
  httplib::Server svr;
  svr.Get("/tx", tx);
  svr.Get("/.well-known/check", ok);
  svr.listen("0.0.0.0", 8080);

  return 0;
}

/**
 * This is a context class that to be used inside a request handler. It will
 * log the request handling time
 */
struct RequestHandlerLogger {
  const httplib::Request &_request;
  const httplib::Response &_response;

  const std::chrono::system_clock::time_point _startTime;

  RequestHandlerLogger(const httplib::Request &req,
                       const httplib::Response &res);
  ~RequestHandlerLogger();
};

/**
 * Start a context, keeping track of when the request started
 */
RequestHandlerLogger::RequestHandlerLogger(const httplib::Request &req,
                                           const httplib::Response &res)
    : _request(req), _response(res), _startTime(std::chrono::system_clock::now()) {}

/**
 * Stop the context, emitting a log message
 */
RequestHandlerLogger::~RequestHandlerLogger() {
  using namespace std;

  const auto now = std::chrono::system_clock::now();
  const auto nowAsTimeT = std::chrono::system_clock::to_time_t(now);
  const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                         now.time_since_epoch()) %
                     1000;

  const auto timeOccurred =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime) %
      1000;

  cout << std::put_time(std::localtime(&nowAsTimeT), "%a %b %d %Y %T") << '.'
       << std::setfill('0') << std::setw(3) << nowMs.count() << " "
       << _request.remote_addr << " " << _request.path << " - " << _response.status << " - "
       << timeOccurred.count() << "ms" << endl;
  cout.flush();
}

/**
 * The test transaction on PostgreSQL
 */
void tx(const httplib::Request &req, httplib::Response &res) {
  RequestHandlerLogger _logger(req, res);
  PGconn *conn = PQconnectdb(database_url.c_str());

  if (PQstatus(conn) != CONNECTION_OK) {
    res.set_content("Connection error", "text/plain");
    res.status = 500;
    std::cout << PQerrorMessage(conn) << std::endl;
    std::cout.flush();
  } else {
    PGresult *pgres = PQexec(conn, sql_query.c_str());
    if (PQresultStatus(pgres) != PGRES_COMMAND_OK &&
        PQresultStatus(pgres) != PGRES_TUPLES_OK) {
      res.set_content("Query error", "text/plain");
      res.status = 500;
      std::cout << PQresultErrorMessage(pgres) << std::endl;
      std::cout.flush();
    } else {
      res.status = 200;
      res.set_content("Ok!", "text/plain");
    }
    PQclear(pgres);
  }

  PQfinish(conn);
}

/**
 * The test transaction on PostgreSQL
 */
void ok(const httplib::Request &req, httplib::Response &res) {
  RequestHandlerLogger _logger(req, res);
  res.status = 200;
  res.set_content("Ok!", "text/plain");
}

/**
 * Get a configuration parameter or returns the default value
 */
std::string get_configuration(const std::string &env_name,
                              const std::string &default_value) {
  auto value = std::getenv(env_name.c_str());
  if (value == NULL) {
    return default_value;
  } else {
    return value;
  }
}
