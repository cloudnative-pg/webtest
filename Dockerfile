FROM ubuntu:noble AS builder
RUN apt-get -y update && apt-get install -y postgresql-client build-essential clang libpq-dev
COPY . /app
WORKDIR /app
RUN make

FROM ubuntu:noble
LABEL quay.expires-after=1d
RUN apt-get -y update && apt-get install -y postgresql-client && rm -rf /var/lib/apt/lists/*
COPY --from=builder /app/http_test /app/http_test
CMD ["/app/http_test"]

