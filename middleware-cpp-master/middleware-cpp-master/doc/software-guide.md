# Gen 7 Software Middleware Guide <!-- omit in toc -->

- [Introduction](#introduction)
- [Concepts](#concepts)
  - [Data Owner](#data-owner)
  - [Data Client](#data-client)
  - [Asynchronous Clients and Owners](#asynchronous-clients-and-owners)
  - [Read .vs Subscribe](#read-vs-subscribe)
    - [Read](#read)
    - [Subscribe](#subscribe)
  - [Write .vs Publish](#write-vs-publish)
    - [Write](#write)
    - [Publish](#publish)
  - [Read/Write Asynchrony](#readwrite-asynchrony)
  - [Read/Write Asynchrony Assurances](#readwrite-asynchrony-assurances)
  - [Configuration](#configuration)
  - [Middleware Logging](#middleware-logging)
- [Known Limitations](#known-limitations)
  - [Fixed Endpoint Thread Pool Size](#fixed-endpoint-thread-pool-size)
- [Middleware: Parameter Service API](#middleware-parameter-service-api)
  - [Onboard Static Configuration](#onboard-static-configuration)
  - [Service Configuration Builder API](#service-configuration-builder-api)
    - [Create](#create)
    - [AddPublish](#addpublish)
    - [AddSubscribe](#addsubscribe)
    - [AddClientRead](#addclientread)
    - [AddClientWrite](#addclientwrite)
    - [AddServerWrite](#addserverwrite)
    - [AddServerRead](#addserverread)
  - [General Usage](#general-usage)
    - [Parameter Service Configuration Builder](#parameter-service-configuration-builder)
    - [Parameter Service Creation](#parameter-service-creation)
    - [Owner Publish](#owner-publish)
    - [Client Read](#client-read)
    - [Client Write](#client-write)
    - [Service Destruction](#service-destruction)
    - [Owner Availability](#owner-availability)
  - [Testing](#testing)
  - [Common Mistakes](#common-mistakes)
    - [Reusing Domain Endpoint](#reusing-domain-endpoint)
- [Middleware: Diagnostics Service API](#middleware-diagnostics-service-api)
  - [Diagnostics Service Creation](#diagnostics-service-creation)
  - [Code Consumer (Client Side)](#code-consumer-client-side)
  - [Code Data Structure](#code-data-structure)
  - [Additional Information](#additional-information)
  - [Code Server (Owner Side)](#code-server-owner-side)
  - [Thread Safety](#thread-safety)
  - [Diagnostics Service Example](#diagnostics-service-example)

## Introduction
These are configurable shared libraries that handles data transfers between users via a provided API.
The internal communication is handled by the Middleware layer. The users can be classified by two types:

## Concepts

### Data Owner
This is the owner of the data being transferred.
Owners primarily access or maintain the raw and cached data.
Owners of data are the only users allowed to "Publish" data to others.
The owner also controls the frequency of data updates and is responsible for handling any data requests from clients.

### Data Client
This is the user that requests or subscribes to data changes.
They can perform read or write requests and subscribe to data changes.
They are not allowed to publish data owned by others.
They are still allowed to write data due to the concept that clients that perform a write request are relinquishing ownership of the data back to the owner.

### Asynchronous Clients and Owners
The built in behavior for clients and owners is to behave in a way that will not block either side.
Actions are queued and sequenced in their own instances.
Anything the client requests, will be returned to them by the owner by the passed in callback at a later time.

### Read .vs Subscribe
These are classified as different actions taken by Data Clients.

#### Read
This is a transactional request triggered by the client.
The client requests the action to the owner, then the owner responds at a later time.

#### Subscribe
The client listens for data broadcasts from the owner.
This is not transactional.
If the publisher does not exist, then the client will continue to subscribe and listen.
All data received by client through a subscription, can be assured that the data received came from the sourced owner.

### Write .vs Publish
These are classified as separate actions allowed only on certain sides of the data.

#### Write

This is a transactional request triggered by the client.
The client requests the action to the owner, then the owner responds at a later time.
Owners do not need to write their own data across the wire.

#### Publish

The data owner can publish data changes to any client who is subscribed.
This is not transactional.
If a publish occurs with no clients subscribed, then the publisher does not care and continues its behavior regardless.

### Read/Write Asynchrony
To prevent wasted resources, a thread is not spun up for every read or write transaction. For long-running transactions, one's server callback should return `std::nullopt` and call Reply on the owning IParameterService once it is complete.
This may be done from any thread, so may involve patterns such as:

- starting a thread for each request
- one worker thread looping to fulfill stored requests that are now complete due to data being made available elsewhere
- one worker thread which, after not being done with a request in under some timeout, is set to exit after Reply-ing and replaced

Reply must be called within a timeout if `std::nullopt` was returned. This is currently `10 seconds`, but will be configurable in the future.
If that timeout has elapsed, the client receives a timeout and any future Reply is dropped.

### Read/Write Asynchrony Assurances
Transactions may be handled concurrently, but are initially received by the server in the order they were created. Callbacks for the transaction of some data (parameter, diagnostics additional information, etc.) are also called in the client in the order they were created (by the client).

### Configuration
The middleware shared libraries configure the allowed connections, endpoints, and the data to be handled.
This is to provide the users less headaches when it comes to connecting to others.

### Middleware Logging
Logging to stream has been integrated into the APIs.
There are *4* levels of logging:

1. LEVEL_ONE
   - Errors Only
2. LEVEL_TWO
   - Warnings + LEVEL_ONE
3. LEVEL_THREE
   - Additional Information + LEVEL_TWO
4. LEVEL_FOUR
   - Debug Information + LEVEL_THREE

Users of the APIs will be given the ability to set the logging target and level via their creation factories.
The setting must be called once before using the factories.
The logger does validate its arguments so be cautious on when and what you set to the logger.

```cpp
  // LEVEL_ONE
  ::middleware::parameter::Factory::SetLogLevel(std::cout, 1);
  // LEVEL_TWO
  ::middleware::parameter::Factory::SetLogLevel(std::cout, 2);
  // LEVEL_THREE
  ::middleware::parameter::Factory::SetLogLevel(std::cout, 3);
  // LEVEL_FOUR
  ::middleware::parameter::Factory::SetLogLevel(std::cout, 4);
```

## Known Limitations

### Fixed Endpoint Thread Pool Size
Each client and server read/write endpoint (Parameter Service and Diagnostics Service alike) dispatches its asynchronous transactions onto an internal thread pool that is **fixed at 5 threads per endpoint instance** and is not currently configurable.
When all 5 threads are occupied and a new request arrives, the **oldest in-flight request is dropped** so the new one can be consumed. Internally this surfaces as a DDS-RPC `BrokenPipelineError`, but the API does not propagate that detail; the caller's completion callback is simply invoked with a `FAILURE` status. To avoid losing in-flight work, throttle outbound request rates and keep callback work short (defer long-running work via `std::nullopt` + `Reply` on the server side, or onto your own worker on the client side).

## Middleware: Parameter Service API
This is a Middleware API that handles parameter data between two endpoints.
The breakup in content follows a certain understanding.
As a user of the Parameter Service API, all the server configuration building is for your own server that you own and control. If your software is a data owner, then you will be expected to create a server for others to connect to for data.
If you do not own data, then you do not need to call "AddServerWrite" or "AddServerRead".
Having no actions means that your internal server object in the Parameter Service will not be used.
Also as a data owner, calling "AddPublish" will only work if defined parameter in the static table or onboard static connections configuration file determines that the parameter identity passed matches the owner identity passed into "Create". There are no limitations on the amount of client read, client write, subscriber, and publisher objects you create in the service.

Both the client-side and server-side read/write endpoints dispatch their asynchronous work onto an internal thread pool. The pool is created per endpoint instance and is **fixed at a size of 5 threads**; it is not currently configurable. This means each created client read, client write, server read, and server write instance has up to 5 in-flight transactions being processed concurrently. Plan callback work accordingly: if a callback blocks for an extended period it consumes one of those 5 threads, so long-running work should be deferred (server side: return `std::nullopt` and call `Reply` later; client side: dispatch onto your own worker).

### Onboard Static Configuration
The Parameter Service resolves domain endpoints and the parameters each endpoint owns from a static lookup table. On a target device, this table **must be provided** by the onboard configuration file:

```
/opt/appdata/Middleware/PARAMETER_SERVICE_TEST_MODE.xml
```

Despite the historical `_TEST_MODE` suffix in its name, this file is a **runtime requirement** for any application that uses client read, client write, server read, or server write actions. The library reads it once on startup; if it is missing, the in-library default lookup table is used instead, and any identity not present there will fail validation when the service is created.

> **Publish/Subscribe-only services do not need this file.** If your application only calls `AddPublish` and/or `AddSubscribe` (no `AddClientRead`/`AddClientWrite`/`AddServerRead`/`AddServerWrite`), the onboard XML is not consulted - publishers and subscribers locate each other purely via the parameter identity string and do not require a domain entry.

The XML root is a `domains` node containing one `domain` per server endpoint. Each `domain` requires:

- `name` - the server identity string. This must match the value passed to `IConfigBuilder::Create` by the owner of those parameters, and to all client `AddSubscribe`/`AddClientRead`/`AddClientWrite` calls that target that owner.
- `id` - a 4-byte unsigned integer domain identifier; it must be unique across all entries.

Each `domain` then contains one `parameter` child per parameter that endpoint owns:

- `name` - the parameter identity used in `AddPublish`/`AddSubscribe`/`AddServerRead`/`AddServerWrite`/`AddClientRead`/`AddClientWrite`.
- `value_type` - the parameter's payload type. Allowed values: `double`, `string`, `binary`, `bool`, `unsigned`.

A parameter must appear under the domain that owns it; client-side actions only validate that the named parameter exists somewhere in the table, while server-side actions (`AddPublish`, `AddServerRead`, `AddServerWrite`) require the parameter to be listed under the server's own `domain`.

> **Client-side fallback:** If a parameter configured via `AddClientRead` or `AddClientWrite` cannot be located in any domain in the resolved lookup table, the service emits a warning and silently defaults that parameter's owner to the **`DataLinkEngine`** server endpoint - even when no `DataLinkEngine` domain exists in the loaded table. This means a typo or missing entry in the configuration file will not fail service creation; the client will simply attempt to communicate with `DataLinkEngine` for that parameter and the request will time out (or never connect) at runtime. Verify the file's contents carefully when client reads/writes do not behave as expected.

Example:

```xml
<domains>
    <!-- Data Link Engine owns these parameters -->
    <domain name="DataLinkEngine" id="1">
        <parameter name="Parameter_0" value_type="double" />
        <parameter name="Parameter_1" value_type="string" />
        <parameter name="Parameter_2" value_type="binary" />
        <parameter name="Parameter_3" value_type="bool" />
        <parameter name="Parameter_4" value_type="unsigned" />
    </domain>
    <!-- Some Blue Key owns these parameters -->
    <domain name="SomeBlueKey" id="5">
        <parameter name="Parameter_5" value_type="double" />
        <parameter name="Parameter_6" value_type="string" />
        <parameter name="Parameter_7" value_type="binary" />
        <parameter name="Parameter_8" value_type="bool" />
        <parameter name="Parameter_9" value_type="unsigned" />
    </domain>
</domains>
```

### Service Configuration Builder API
The library is configured by passing a configuration builder/taker object into its Service factory.
The configuration builder/taker object has functionality that will allow users to add content into the objects internal configuration structure, see interface `IConfigBuilder.h`.
The service does define domains and connections internally via a static lookup table or an existing onboard static connections configuration file.
If the onboard file exists, then it will be used as the lookup table.
If it does not exist, it will use the default provided in the shared library.
The parameter and domain identities passed into the configuration must match what is in the internal lookup table, otherwise connections will fail.

#### Create
This is a static creation function that will return an interface to the builder object.
It requires a server identity string value.
If intending to use read/write as a client or server, the identity must match a domain string in the static table or onboard static connections configuration file.

> **Note:** If this service will act as a server (i.e. you will call `AddServerWrite` or `AddServerRead`), the identity string passed here must exactly match the `name` attribute of the `domain` entry in the [Onboard Static Configuration](#onboard-static-configuration) XML that owns the parameters being served.

```cpp
    auto spConfigBuilder = ::middleware::parameter::IConfigBuilder::Create("DataLinkEngine");
```

#### AddPublish
This adds a parameter that the server will be publishing.
It is added to the internal service configuration.
It requires a parameter identity, a unit, and an initial value to be published on creation.
The initial value will be immediately sent right before exiting it's constructor.
```cpp
    auto spConfigBuilder = ::middleware::parameter::IConfigBuilder::Create("MyApplication");
    middleware::parameter::Data initial_value{};
    initial_value.mutable_value()->set_string_value("Mossville")
    spConfigBuilder->AddPublish("Location", {}, std::move(initial_value))
```

#### AddSubscribe
This adds a parameter that the client will subscribe to for data publishes.
It is added to the internal service configuration.
`AddSubscribe` has two overloads, both of which take an `on_subscription` callback and an `on_disconnect` callback:

1. **Recommended (metadata overload):** `AddSubscribe(parameter_id, on_subscription, on_disconnect)`
   - The `on_subscription` callback signature is `void(Identity const& id, Data const& data, Metadata const& metadata)`.
   - `Metadata` (defined in `ParameterServiceDefines.h`) carries owner-provided contextual information about the published value (currently the `unit`). Future fields will be added to `Metadata` rather than to the function signature, so this overload is the preferred form for new code.
2. **Deprecated:** `AddSubscribe(parameter_id, unit, on_subscription, on_disconnect)`
   - The `on_subscription` callback signature is `void(Identity const& id, Data const& data)` and the unit is fixed at configuration time.
   - Retained for backward compatibility; new code should prefer the metadata overload.

For both overloads:

- The `on_subscription` callback returns no value. It is invoked every time a publisher publishes data and when a subscriber connects to an existing publisher.
- The `on_disconnect` callback returns no value and takes no arguments. It is invoked when the publisher disconnects from the subscriber and after the last `on_subscription` invocation has finished.

```cpp
    auto spConfigBuilder = ::middleware::parameter::IConfigBuilder::Create("MyApplication");

    // Recommended: metadata overload
    auto on_subscription = [](::middleware::parameter::Identity const& id,
                              ::middleware::parameter::Data const&     data,
                              ::middleware::parameter::Metadata const& metadata)
    {
        // Inspect data and metadata.unit
    };
    auto on_disconnect = [] { /*Do this*/ };
    spConfigBuilder->AddSubscribe("GPS", std::move(on_subscription), std::move(on_disconnect));

    // Deprecated: unit overload
    auto legacy_on_subscription = [](::middleware::parameter::Identity const& id,
                                     ::middleware::parameter::Data const&     data) { /*Do this*/ };
    spConfigBuilder->AddSubscribe("GPS", {}, std::move(legacy_on_subscription), std::move(on_disconnect));
```


#### AddClientRead
This adds a parameter that the client wants to read from the data owner.
It is added to the internal service configuration.
It requires a parameter identity and unit.
The parameter name used must match exactly to the defined parameter in the static table or onboard static connections configuration file.
```cpp
    auto spConfigBuilder = ::middleware::parameter::IConfigBuilder::Create("MyApplication");
    spConfigBuilder->AddClientRead("Timezone", {});
```

#### AddClientWrite
This adds a parameter that the client wants to write to the data owner.
It is added to the internal service configuration.
It requires a parameter identity and unit.
The parameter name used must match exactly to the defined parameter in the static table or onboard static connections configuration file.
```cpp
    auto spConfigBuilder = ::middleware::parameter::IConfigBuilder::Create("MyApplication");
    spConfigBuilder->AddClientWrite("Hours Worked", {});
```

#### AddServerWrite
This adds an action that the server will execute on the linked parameter identity when its a write action.
It is added to the internal service configuration.
It requires a parameter identity and action callback.
The parameter name used must match exactly to the defined parameter in the static table or onboard static connections configuration file.
The callback takes an unchangeable transaction key, unchangeable parameter identity, and a value as arguments. It should return the value provided to it or `std::nullopt` if the action was deferred. In the latter case, your IParameterService's Reply should be called later with the key and value provided in order to mark the action as complete.
The server has `10 seconds` from the moment the request is received to formulate a response (either by returning a value from the callback or by calling Reply after returning `std::nullopt`). If this timeout elapses, the request is dropped and any subsequent Reply for that key is ignored.
```cpp
    auto spConfigBuilder = ::middleware::parameter::IConfigBuilder::Create("MyApplication");
    auto on_write = [](auto /*key*/, auto const &id, auto const &data) -> auto {
          middleware::parameter::Data output{};
          // Do something with data
          return output
        }
    spConfigBuilder->AddServerWrite("Fuel Level", std::move(on_write));
```

#### AddServerRead
This adds an action that the server will execute on the linked parameter identity when its a read action.
It is added to the internal service configuration.
It requires a parameter identity and action callback.
The parameter name used must match exactly to the defined parameter in the static table or onboard static connections configuration file.
The callback takes an unchangeable transaction key and an unchangeable parameter identity as arguments. It should return the requested value or `std::nullopt` if the action was deferred. In the latter case, your IParameterService's Reply should be called later with the key provided and requested value.
The server has `10 seconds` from the moment the request is received to formulate a response (either by returning a value from the callback or by calling Reply after returning `std::nullopt`). If this timeout elapses, the request is dropped and any subsequent Reply for that key is ignored.
```cpp
    auto spConfigBuilder = ::middleware::parameter::IConfigBuilder::Create("MyApplication");
    auto on_read = [](auto /*key*/, auto const &id) -> auto {
          middleware::parameter::Data output{};
          // Do something with data
          return output
        }
    spConfigBuilder->AddServerRead("My Zone", on_read)
```




### General Usage

#### Parameter Service Configuration Builder
Creating the Parameter Service configuration is done via the configuration builder object.
To get a builder object, use its own associated factory.
The builder provides functionality to add publishers, subscribers, server actions, read clients, and write clients.
Example:

```cpp
    auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("DataLinkEngine");
    middleware::parameter::Data some_owned{};
    middleware::parameter::Data default_value{};

    // Server Publishers
    spConfigBuilder->AddPublish("Engine Speed", {}, std::move(some_owned))
        .AddPublish("Bucket Height", {}, default_value)
        .AddPublish("Speed Limit", {}, default_value);
    // Server Writes
      auto on_write = [](auto /*key*/, auto const &id, auto const &data) -> auto {
          middleware::parameter::Data output{};
          // Do something with data
          return output
        }
    spConfigBuilder->AddServerWrite("Product ID", std::move(on_write));

    // Server Reads
      auto on_read = [](auto /*key*/, auto const &id) -> auto {
          middleware::parameter::Data output{};
          // Do something with data
          return output
        }
    spConfigBuilder->AddServerRead("Product ID", on_read)
        .AddServerRead("Machine Serial Number", on_read)
        .AddServerRead("Software Part Number", [](auto const &id, auto &data){ /*Do something else */});

    // Client Subscriptions
    auto on_subscription = [](auto const &id, auto const &data, auto const &metadata){ /*Do this*/ };
    auto on_disconnect = [] { /*Do this*/ };
    spConfigBuilder->AddSubscribe("Brightness", std::move(on_subscription), std::move(on_disconnect));
    // Client Write
    spConfigBuilder->AddClientWrite("Battery Level", {});
    // Client Read
    spConfigBuilder->AddClientRead("Language", {});


    ::middleware::parameter::Factory::SetLogLevel(std::cout, 1);
    auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfigBuilder));
```

#### Parameter Service Creation
The creation of the service is done via its factory.
Users of the service beforehand must create a Parameter Service configuration using the configuration builder. Example:

```cpp
  // Build configuration here....
  auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("DataLinkEngine");
  //
  // Use the service here...
  ::middleware::parameter::Factory::SetLogLevel(std::cout, 1);
  auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfigBuilder));
```


#### Owner Publish
Once the service is created, users will be able to access their owned publisher objects by reference from the service.
Users are required to remember and use the correct identities they used in the configuration.
Publisher object references have only one function called "Publish" that takes a "::cat::middleware::parameter::Data" protobuf message.

- ::cat::middleware::parameter::Data
  - Protobuf message from "middleware_parameter.proto"

```cpp
        auto &rEngineSpeed = spService->RetrievePublisherInstance("Engine Speed");
        ::middleware::parameter::Data data{};
        data.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
        data.mutable_value()->set_double_value(22.9);
        rEngineSpeed.Publish(data);
```


#### Client Read
Once the service is created, users will be able to access the reader objects by reference from the service.
Users are required to remember and use the correct identities they used in the configuration.
Read object references have only one function called "Read" it takes an on action complete callback.
The function returns a boolean that tells users if there was an internal connection issue.
If the server does not respond within `10 seconds`, the request times out and the callback is invoked with a failure status.


```cpp
// Read On Complete Callback
std::function<void(::middleware::parameter::IRead::ReadStatus const &,  ::cat::middleware::parameter::Parameter const &)>;
```

- ::middleware::parameter::IRead::ReadStatus
  - Structure from "IReadParameter.h"
- ::cat::middleware::parameter::Parameter
  - Protobuf message from "middleware_parameter.proto"

```cpp
        auto &rLanguage = spService->RetrieveReadInstance("Language");
        auto handler = [](auto const &status, auto const &parameter)
        {
            // Handle data
        };
        if(rLanguage.Read(handler))
        {
            // Successful request
        }
```

#### Client Write
Once the service is created, users will be able to access the writer objects by reference from the service.
Users are required to remember and use the correct identities they used in the configuration.
Write object references have only one function called "Write" it takes an on action complete callback.
The function returns a boolean that tells users if there was an internal connection issue.
If the server does not respond within `10 seconds`, the request times out and the callback is invoked with a failure status.


```cpp
// Read On Complete Callback
std::function<void(::middleware::parameter::IWrite::WriteStatus const &,  ::cat::middleware::parameter::Parameter const &)>;
```

- ::middleware::parameter::IWrite::WriteStatus
  - Structure from "IWriteParameter.h"
- ::cat::middleware::parameter::Parameter
  - Protobuf message from "middleware_parameter.proto"

```cpp
        auto &rBatteryLevel = spService->RetrieveWriteInstance("Battery Level");
        ::middleware::parameter::Data data{};
        data.set_quality(::cat::middleware::parameter::QUALITY_GOOD);
        data.mutable_value()->set_double_value(5.0);

        auto handler = [](auto const &status, auto const &parameter)
        {
            // Handle data
        };
        if(rBatteryLevel.Write(data, handler))
        {
            // Successful request
        }
```

#### Service Destruction
The Parameter Service is able to destruct and tear down all of its internal objects.
There is no special handling needed by the user.
The service manages the complete lifetime of all configured objects.

#### Owner Availability
There is no built in behavior in the Parameter Service for users that will tell them whether the owners of the data being requested is available.
This was decided by many to not be implemented.
As a tip, there is a way to do it with the Parameter Service.
Basically, choose a read or write object reference and just call its function.
The passed callback will let you know if there is an owner available.
There are better ways, but this is a basic way done in testing.

```cpp
void MyWaitForServer(::middleware::parameter::IRead &reader)
{
    std::mutex mutex{};
    std::condition_variable condition{};

    bool ready = false;
    while (!ready)
    {
        auto handler = [&mutex, &condition, &ready](auto const &status, auto const &parameter)
        {
            if (status.status == ::middleware::parameter::Status::SUCCESS)
            {
                // Owner available
                std::unique_lock<std::mutex> lock{mutex};
                ready = true;
                condition.notify_all();
            }
            else
            {
                // Owner not available
            }
        };
        reader.Read(handler);
        {
            std::unique_lock<std::mutex> lock{mutex};
            // Retry every 2 seconds
            condition.wait_for(lock, std::chrono::seconds(2), [&ready]
                               { return ready; });
        }
    }
    std::this_thread::sleep_for(5s);
}
int main(int argc, char ..argv)
{
    // Build configuration here....
    auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("DataLinkEngine");
    spConfigBuilder->AddClientRead("Language", {});

    // Use Service
    auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfigBuilder));
    auto &rLanguage = spService->RetrieveReadInstance("Language");
    MyWaitForServer(rLanguage);
}
```

### Testing
For users to try out the Parameter Service, there has been overrides and a test application implemented. These test apps and xml configurations will change or go away in time and are not expected to be moving with production.

During testing, the same onboard configuration file (see [Onboard Static Configuration](#onboard-static-configuration)) is used to override the in-library default lookup table; populate it with whatever domains and parameters your test scenario needs.

There is a testing application called `middleware-parameters-tester` that can be used to test the Parameters API.
For more information, see [zzMiddlewareDevTools README.md](../scripts/verity/middleware-dev-tools/README.md)


### Common Mistakes

#### Reusing Domain Endpoint
When creating a Parameter Service object you are creating an endpoint.
The domain ID and name *must be different* than another Parameter Service object.

**DO NOT DO THIS!!**

Application Alpha
```cpp
  // Build configuration here....
  auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("DataLinkEngine");
  //
  // Use the service here...
  ::middleware::parameter::Factory::SetLogLevel(std::cout, 1);
  auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfigBuilder));
```

Application Bravo
```cpp
  // Build configuration here....
  auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("DataLinkEngine");
  //
  // Use the service here...
  ::middleware::parameter::Factory::SetLogLevel(std::cout, 1);
  auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfigBuilder));
```

In the above example, "Application Bravo" will overwrite the endpoint made in `Application Alpha`.
The names passed into `middleware::parameter::IConfigBuilder::Create` must be different because they are two different endpoints.

**BETTER**

Application Alpha
```cpp
  // Build configuration here....
  auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("DataLinkEngine.Alpha");
  //
  // Use the service here...
  ::middleware::parameter::Factory::SetLogLevel(std::cout, 1);
  auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfigBuilder));
```

Application Bravo
```cpp
  // Build configuration here....
  auto spConfigBuilder = middleware::parameter::IConfigBuilder::Create("DataLinkEngine.Bravo");
  //
  // Use the service here...
  ::middleware::parameter::Factory::SetLogLevel(std::cout, 1);
  auto spService = ::middleware::parameter::Factory::CreateParameterService(std::move(spConfigBuilder));
```

## Middleware: Diagnostics Service API
The Diagnostics Service API distributes a list of active diagnostic codes from a single owner (the **Data Link Engine**) to any number of consumers, and supports request/response retrieval of additional information for individual codes.
Unlike the Parameter Service, the diagnostics endpoint, domain ID, and connection names are fixed by the library; consumers do not provide a configuration builder.

Like the Parameter Service, both the client (`ICodeClient`) and server (`ICodeServer`) endpoints dispatch their asynchronous work onto an internal thread pool. The pool is created per endpoint instance and is **fixed at a size of 5 threads**; it is not currently configurable. Long-running callback work should be deferred (server side: return `std::nullopt` from the additional-information callback and call `Reply` later; client side: dispatch onto your own worker) so the pool is not starved.

To receive diagnostic codes, recipients should:

- Create an `IService` with `Factory::CreateDiagnosticService()`, defined in `CodesFactory.h`
- Create a `CodeConsumer` with callbacks - see `CodesStructures.h`
- Create an `ICodeClient` with the diagnostic service's `CreateCodeClient(CodeConsumer&&)`, defined in `IDiagnosticService.h`

There will only ever be one diagnostics server - **Data Link Engine** - and if it is not found for initial connection after **10s**, the disconnect callback will be automatically called.
Upon successful connection the current list of codes will be provided via the data callback. When a code is added, removed, or modified, the data callback will be called again with an updated list. Determining what was added, removed, and/or modified may be done by comparing the `code_key` values across successive callback invocations.

### Diagnostics Service Creation
The service is created via its factory. Any number of services can be created within a process, and a single service can host any number of `ICodeClient` instances.

```cpp
    auto spCodeService = ::middleware::diagnostics::Factory::CreateDiagnosticService();
```

Logging for the diagnostics API uses the same level system as the parameter API and is configured via the parameter factory:

```cpp
    ::middleware::parameter::Factory::SetLogLevel(std::cout, 1);
```

### Code Consumer (Client Side)
A `CodeConsumer` (defined in `CodesStructures.h`) is a small struct of two callbacks that are moved into the client on creation:

```cpp
struct CodeConsumer
{
    // All callbacks are passed into entities to own. MUST BE THREAD SAFE!
    std::function<void(::cat::middleware::diagnostics::Codes const& codes)> callback;
    std::function<void()>                                                   disconnect;
};
```

- `callback` is invoked on initial connection with the current list of codes, and again every time the owner publishes an updated list (additions, removals, or modifications).
- `disconnect` is invoked when the owner disappears, or when the initial 10-second connection window elapses without locating the owner.

Once the consumer is built, create the client from the service:

```cpp
    ::middleware::diagnostics::CodeConsumer consumer{std::move(on_subscription), std::move(on_disconnect)};
    auto spCodeClient = spCodeService->CreateCodeClient(std::move(consumer));
```

The returned `std::unique_ptr<ICodeClient>` owns the subscription; destroying it stops further callbacks.

### Code Data Structure
The `Codes` payload delivered to the consumer's `callback` is the protobuf message defined in `middleware_diagnostics.proto`. Each `Code` entry contains:

- `code_key` - a unique 64-bit identifier for the code; use this to detect adds/removes/modifies across successive callbacks.
- `network_address` - identifies the source of the code (`self` or a `j1939_address` with address and CAN port).
- `information` - one of `diagnostic`, `event`, or `public_dtc` payloads, each carrying identifiers, lamp statuses (where applicable), and a `CodeStatus` (`ACTIVE_ONLY`, `ACTIVE_AND_LOGGED`, `LOGGED_ONLY`, `PREVIOUSLY_ACTIVE`).

> The `update_reason` field on `Code` is deprecated and should not be used by new code. Track changes by diffing `code_key` sets between callbacks instead.

### Additional Information
Additional information may be requested for a specific code using your `ICodeClient`'s `RequestAdditionalInformation`:

```cpp
virtual void RequestAdditionalInformation(
    uint64_t                                                          codeKey,
    std::vector<::middleware::diagnostics::AdditionalInformationGroup> const& groups,
    OnComplete&&                                                      additionalInformationCallback
) = 0;
```

- `codeKey` - the `code_key` of the code whose details are being requested.
- `groups` - one or more `AdditionalInformationGroup` values (currently only `Group2` is defined) selecting which information sets to retrieve.
- `additionalInformationCallback` - invoked once with the populated `::cat::middleware::diagnostics::AdditionalInformation` reply.

Like the Parameter Service, requests are subject to a `10 second` timeout; if the owner does not respond in time, the request is dropped. The callback is invoked on the diagnostics endpoint's internal thread pool (fixed pool size of 5 per instance), so it must be thread safe relative to your application state.

### Code Server (Owner Side)
Applications that own diagnostic codes (the Data Link Engine) create an `ICodeServer` instead of (or in addition to) an `ICodeClient` from the same `IService`:

```cpp
virtual std::unique_ptr<ICodeServer> CreateCodeServer(
    ::cat::middleware::diagnostics::Codes const& current,
    ::middleware::diagnostics::ICodeServer::OnRequest&&  requestAdditionalInformation
) = 0;
```

- `current` - the initial list of codes to publish to subscribers immediately on creation.
- `requestAdditionalInformation` - a callback of signature `std::optional<AdditionalInformation>(uint_least16_t key, AdditionalInformation const& request)` invoked when a client requests additional information. Return the populated reply, or `std::nullopt` to defer; in the deferred case, call `ICodeServer::Reply(key, data)` later (within `10 seconds`) to complete the transaction.

When the owned code list changes, publish the updated list to all subscribers:

```cpp
    spCodeServer->Publish(updatedCodes);
```

Only one diagnostics server should be created in the system (the Data Link Engine).

### Thread Safety
All consumer and server callbacks are invoked from internal middleware threads. Callbacks **must be thread safe**, must not block for extended periods (long-running work should be deferred via `std::nullopt` + `Reply` on the server side, or dispatched to your own worker on the client side), and must not destroy the owning client/server from within the callback.

### Diagnostics Service Example
```cpp
    std::unique_ptr<::middleware::diagnostics::ICodeClient> spCodeClient;
    auto on_subscription = [spCodeClient&](auto const &data) {
        if(data.codes_size() != 0)
        {
            // Handle data
            if(data.codes(0).code_key() == 0x01)
            {
                spCodeClient->RequestAdditionalInformation(
                    data.codes(0).code_key(),
                    {::middleware::diagnostics::AdditionalInformationGroup::Group2},
                    [](auto& additionalInformation) {
                        // Handle additional information
                    }
                );
            }
        }
    };
    auto on_disconnect = []() {
        // No diagnostics info, but any number could be active. . . panic?
    };

    auto spCodeService = ::middleware::diagnostics::Factory::CreateDiagnosticService();
    ::middleware::diagnostics::CodeConsumer consumer = {std::move(on_subscription), std::move(on_disconnect)};
    spCodeClient = spCodeService->CreateCodeClient(std::move(consumer));
```
