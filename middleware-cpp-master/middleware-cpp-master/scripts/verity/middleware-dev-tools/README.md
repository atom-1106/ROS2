# zzMiddlewareDevTools Usage <!-- omit in toc -->

- [Setup](#setup)
- [Middleware Parameter Service Tester (middleware-parameters-tester)](#middleware-parameter-service-tester-middleware-parameters-tester)
  - [Testing Publishers](#testing-publishers)
  - [Testing Subscribers](#testing-subscribers)
  - [Testing Server Read Requests](#testing-server-read-requests)
  - [Testing Server Write Requests](#testing-server-write-requests)
  - [Notes](#notes)

## Setup
For applications to execute, please run
`export LD_LIBRARY_PATH="/opt/zzMiddlewareDevTools/lib"`

This can be simplified by just running `app_init.sh`.


## Middleware Parameter Service Tester (middleware-parameters-tester)
This is an application used to test middleware publishers, subscribers, client read requests, and client write requests from the Middleware Parameter Service API.
This application will not simulate a server side endpoint for requests.
For additional info, run `./middleware-parameter-tester help`

### Testing Publishers
To test publishers, run the application with the subscriber options.
For example, your application has a publisher for `ParameterA`.
You would need to run this application with a subscriber for the same parameter.

```bash
./middleware-parameter-tester sub --name "ParameterA"
```

The application will log to the terminal all received data forever.
Additionally, you can create multiple subscribers as well.

```bash
./middleware-parameter-tester sub --name "ParameterA" --name "ParameterB"
```


### Testing Subscribers
To test subscribers, run the application with the publisher options.
For example, your application has a subscriber for `ParameterA`.
You would need to run this application with a publisher for the same parameter.

```bash
# All the available publisher data types, application will exit after 5 seconds
./middleware-parameter-tester pub --name "ParameterA" --double_value 100.0
./middleware-parameter-tester pub --name "ParameterA" --string_value Hello
./middleware-parameter-tester pub --name "ParameterA" --binary_value 0010230A18
./middleware-parameter-tester pub --name "ParameterA" --boolean_value true
./middleware-parameter-tester pub --name "ParameterA" --uinteger64_value 100
# This is the same but with the exit delay in ms override option, application will exit after the duration expires
./middleware-parameter-tester pub --name "ParameterA" --double_value 100.0 --delay 100ms
```

The application will publish the value once.

**IMPORTANT:** When the application exits, the subscriber endpoint will run its disconnect behavior.


### Testing Server Read Requests
To test server read requests, run the application with the client read request options.
For example, your application has a server read for `ParameterA`.
You would need to run this application with a client read request for the same parameter.

```bash
./middleware-parameter-tester read --name "ParameterA"
```

The application will wait for the request to finish, logging to the terminal what was received from the server.
Additionally, you can create multiple read requests as well.

```bash
./middleware-parameter-tester read --name "ParameterA" --name "ParameterB"
```


### Testing Server Write Requests
To test server write requests, run the application with the client write request options.
For example, your application has a server write for `ParameterA`.
You would need to run this application with a client write request for the same parameter.

```bash
./middleware-parameter-tester write --name "ParameterA" --double_value 100.0
./middleware-parameter-tester write --name "ParameterA" --string_value Hello
./middleware-parameter-tester write --name "ParameterA" --binary_value 0010230A18 
./middleware-parameter-tester write --name "ParameterA" --boolean_value false 
./middleware-parameter-tester write --name "ParameterA" --uinteger64_value 42
```

The application will wait for the request to finish, logging to the terminal what was received from the server.


### Notes
If you want to run multiple instances, copy the application to the `tmp` directory and link the libraries.


