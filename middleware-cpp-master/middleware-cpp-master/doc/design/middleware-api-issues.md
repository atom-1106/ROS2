# Middleware API Issues <!-- omit in toc -->

- [Motivation](#motivation)
- [Current Issues/Limitations](#current-issueslimitations)
  - [Lack of documentation in public interface headers](#lack-of-documentation-in-public-interface-headers)
  - [Unit parameter confusion](#unit-parameter-confusion)
    - [Publisher](#publisher)
    - [Subscriber](#subscriber)
    - [Read](#read)
    - [Write](#write)
  - [General interface confusion](#general-interface-confusion)
    - [Callback types and names](#callback-types-and-names)
    - [`IConfigBuilder` method names](#iconfigbuilder-method-names)
  - [Published Units on the Wire and in the API](#published-units-on-the-wire-and-in-the-api)
  - [Data Type Requirements](#data-type-requirements)
  - [Read/Write Cancelation is Impossible](#readwrite-cancelation-is-impossible)
  - [Overall Thread Count](#overall-thread-count)
    - [Read/Write server thread pool](#readwrite-server-thread-pool)
  - [Streaming read/write API](#streaming-readwrite-api)
  - [Read/Write XML](#readwrite-xml)
  - [Security](#security)
  - [Design Notes](#design-notes)
  - [Bugs / Stability](#bugs--stability)
  - [Diagnostic Proto](#diagnostic-proto)
  - [Diagnostics Interface](#diagnostics-interface)
  - [Protobuf serialization](#protobuf-serialization)
  - [Event One-Shot](#event-one-shot)
  - [QoS settings aren't optimal (maybe?)](#qos-settings-arent-optimal-maybe)
  - [Read/Write Responses](#readwrite-responses)
  - [Not every paramter publish is received](#not-every-paramter-publish-is-received)
  - [Logging](#logging)

## Motivation

Now that we and our customers have been using the Middleware 1.0.0 API extensively, we have observed multiple areas where it could be improved for clarity, usability, and maintainability.

This API is our constant "20-year" API so all changes we make to it must be
1. A true improvement for the users
2. Very deliberate where all backwards compatibility breaks (either in the API or on the wire) are clearly understood, documented, and approved before moving forward with the changes

>>> [!caution]
All breaking changes must be clearly communicated to our users and, in fact, over-communicated. If there is a breaking API change that is not backwards compatible, it would be a good idea to provide a tool that would automatically refactor the user's code to make it compliant with the new API.
>>>

## Current Issues/Limitations

### Lack of documentation in public interface headers
Self-explanatory. The interface headers used by clients should clearly document the interface and how to use it. Additional, more in-depth documentation is good, but documentation within the code is more important -- it is more likely to be found by the users and more likely to be kept up-to-date over time by the developers (as opposed to the software guide).

### Unit parameter confusion

#### Publisher
```c++
IConfigBuilder:
	virtual IConfigBuilder& AddPublish(Identity&& parameter_id, Unit unit, Data&& initial_value) = 0

IPublishParameter (via IBasic):
  virtual ::middleware::parameter::Unit Unit()         = 0;
```

The publisher *must* provide the `Unit`.

>>> [!important]
There is still a question whether the `Unit` must be provided at creation/discovery time or if it should be allowed to be set later. This is addressed later in this document.
>>>

#### Subscriber
```c++
IConfigBuilder:
	virtual IConfigBuilder& AddPublish(Identity&& parameter_id, Unit unit, Data&& initial_value)

	virtual IConfigBuilder&
	AddSubscribe(Identity&& parameter_id, Unit unit, OnSubscription&& callback, OnDisconnect&& disconnect) = 0;
	virtual IConfigBuilder&
	AddSubscribe(Identity&& parameter_id, OnSubscriptionWithMetadata&& callback, OnDisconnect&& disconnect) = 0;
```

There is absolutely no reason for the `Unit` to be passed into `AddSubscribe()`. (That overload is actually deprecated today.)


#### Read
```c++
IConfigBuilder:
	virtual IConfigBuilder& AddClientRead(Identity&& parameter_id, Unit unit)
```

The client should not be specifying the `Unit`. The server will return the `Unit` in the response with the value.

>>> [!important]
Is this true?
>>>

#### Write
```c++
	virtual IConfigBuilder& AddClientWrite(Identity&& parameter_id, Unit unit)
```

>>> [!important]
Should the client send the `Unit` to the server? This would allow the server to reject if it is not in the expected `Unit`. However that would then *require* the client to provide the `Unit` -- it would not be optional at all.
>>>

Ideally the clients should be able to query the unit expected by the server for writes.

### General interface confusion
1. Interface names are unclear (like `IPublish`). Interface names are much clearer when they are nouns.
2. The clients must `#include <IPublishParameter.h>` when they should really be "namespaced" like `#include <middleware/parameter/IPublishParameter.h>`. This makes it obvious where the includes are coming from (see `boost` for examples of this).
3. Interfaces are hard to find in headers because they don't live in headers named after the interface (`IPublish` is in `IPublishParameter.h`).
    - Perhaps the files were named this way because they were not properly namespaced using directories (see prior point)
4. Some interfaces inherit from other interfaces (See `IBasic` base class). This makes it hard for the user to clearly understand the full public interface.
5. The interfaces are noisy -- some are in `middleware::parameter` but still unnecessarily call out this same namespace for return and parameter types. Ex: `IBasic`'s `::middleware::parameter::Unit Unit()`.
6. The `key` used in the async methods has led to quite a bit of confusion, especially around the difference between it and the diagnostics `codeKey`. Perhaps it should be called a `handle` instead of a `key` since that term makes it clear it is an abstraction for something, in this case an asynchronous operation.
7. The `ICodeClient::OnComplete` callback takes a writable reference to `::cat::middleware::diagnostics::AdditionalInformation& additionalInformation`. This should be `const`.

Examples:
| Header | Interface | Better would be |
|--|--|--|
| `IPublishParameter.h` | `IPublish` | `<middleware/parameter/IPublisher.h> - IPublisher` |
| `IBasicParameter.h` | `IBasic` | Simply eliminate this |
| `IReadParameter.h` | `IRead` | `<middleware/parameter/IReader.h> - IReader` |
| `IWriteParameter.h` | `IWrite` | `<middleware/parameter/IWriter.h> - IWriter` |

#### Callback types and names
The callback types and names should clearly describe the event occurred (past tense) to be clear about the reason they would be called. In `IConfigBuilder` I suggest changes like:
| Old | New |
|--|--|
| `OnSubscriptionWithMetadata callback` | `OnParameterReceived onParameterReceived` |
| `OnDisconnect disconnect` | `OnDisconnected onDisconnected` |
| `OnWriteAction callback` | `OnWriteRequested onWriteRequested` |

#### `IConfigBuilder` method names
The method names like `AddSubscribe` are confusing because 'Add' is a verb, but it is unclear what is being added. What is the noun that is being added? I suggest changes like:
| Old | New |
|--|--|
| `AddPublish` | `AddPublisher` |
| `AddSubscribe` | `AddSubscriber` |
| `AddClientRead` | `AddReadClient` |
| `AddClientWrite` | `AddWriteClient` |
| `AddServerRead` | `AddReadServer` |
| `AddServerWrite` | `AddWriteServer` |

Plus `OnSubscription()` and `AddSubscribe()` are deprecated now.

### Published Units on the Wire and in the API
Here is the protobuf describing the `Parameter` that is sent on the wire every time a parameter value is published.

```protobuf
message Data {
    Value value     = 1;
    Quality quality = 2;
}
message Info {
    string identity = 1;
    uint32 unit     = 2;
}

message Parameter {
    Info info = 1;
    Data data = 2;
}
```

The `IPublish` interface takes only `Data` -- the `unit` is only able to be changed at creation time in `IConfigBuilder`'s `AddPublish()` method. Thus it is unnecessary to send the unit every time.

We could store the Unit in the DataWriter's [UserDataQosPolicy](https://fast-dds.docs.eprosima.com/en/latest/fastdds/dds_layer/core/policy/standardQosPolicies.html#userdataqospolicy). The Middleware Reader would retrieve the parameter Unit from this field after discovery.
  - If the Read Parameter response message contains the `Unit`, then the Published Parameter message should probably contain `Unit` as well for consistency.

Tricky for mapper -- it doesn't necessarily know the unit until the value is published, so it can't create the participant until later. Maybe we should allow it to be defined as None and changed later? Could potentially be part of a middleware layer config.

### Data Type Requirements
Should the client be able to query the data type? What if the Write Server only takes binary and someone sends a boolean?

### Read/Write Cancelation is Impossible
There is no way for Middleware server to tell the server implementation to cancel a Read/Write.
- The server implementation should provide callback that will be called if there is a timeout, the client goes away, etc. This will allow the implementation to free up any resources used by the active read/write. 

### Overall Thread Count
There are many, many threads spawned. We must understand them.

- From testing, DDS does create a lot of threads already. For what exact reason is not entirely known.

#### Read/Write server thread pool
Each read/write server has a thread pool of 5 threads.
>>> [!important] Things to figure out
- Are all 5 threads created at startup or are they created as-needed?
- We are limited to 5 simultaneous read/writes.. what happens when a 6th one comes in?
>>>

### Streaming read/write API
RPC Feeds open up a channel between two endpoints that will not close until the endpoints disconnect or the client calls Finish
   - Pro:
     - Potentially streamlines a reply later system from a wait for 1 response for (X) time to keep the channel open for that request until the server side writes a response that is viable for the client to use.
   - Con:
     - Not exactly how RPC feeds are to be used. Their expectation of usage is for the server to send multiple values to a client for a single request. Our usage would be send NoData until timeout or server fulfills the Reply later call.

### Read/Write XML
Currently the Read/Write Middleware client must know the names of the Parameters and the servers that host them. This is configured as follows:
1. `/opt/appdata/Middleware/PARAMETER_SERVICE_TEST_MODE.xml` configuration file
2. If that file does not exist, there is an internal hard-coded list that will be used.
3. If the parameter is not defined either place, the default server "DataLinkEngine" is used
   1. This was required for PGN On-Request parameters where the client performs a read of the middleware parameter ID which is named in the data link config

Previous ideas:
  - Update the API configuration to take endpoint connection information
  - Pre-defined C++ configuration in the form of a struct built as a library for the application to load.

>>> [!important]
- What can we do to get rid of this file?
- Maybe allow the client to specify the server in the API?
>>>

### Security
At some point each application will need a signed security XML which defines the parameters it can publish and subscribe to.

- Knowledge so far:
  - Security plugin is enabled and configured at the DomainParticipant QOS level. That level is unchanged between standard pub/sub and DDS RPC.
  - When determining topic targets for the Permissions and Governance Security files, we do not exactly know what the final RPC topic names are going to be because with RPC we do not have access to the low level DDS entities like Topic.

>>> [!important]
  - We may need support from eProsima around security plugin setup with Fast DDS RPC due to not much documentation around it.
>>>

### Design Notes
- Client side callbacks will finish after `10 seconds` of no reply from the server.
- Server side callbacks will finish after `10 seconds` of no value assigned via the `API:Reply(...)` function call from the server side user.
- Client RPC system manages requests via `asio thread pool` with a size of `5 threads`.
- Server RPC system manages requests via `internal DDS scheduler thread pool` with a size of `5 threads`.
- Client side receives an RpcFuture from the RPC server entity.
- RPC Request Handler from the RpcServer defines finish of the request as `when exiting the IDL RPC function`.

### Bugs / Stability
Order of start up matters. Sleeps. Etc.

### Diagnostic Proto
Embedded into the Middleware. It should be its own proto library.

### Diagnostics Interface
This is a one-off. Could it be done in Read/Writes.

### Protobuf serialization
Would it have been easier to use json or eProsima's built-in serialization?

### Event One-Shot
HEDC wants to be able to fire events. Also needed for jog dial. Don't want history maintained.

### QoS settings aren't optimal (maybe?)
See Jonathan's email.

### Read/Write Responses
Would be nice to provide more data link info like "retry later". It is unclear how to provide this info to the client.

### Not every paramter publish is received

### Logging
There is only a limited ability to control logging, it is not discoverable, and it requires the use of an opaque integer log level.

```cpp
middleware::parameter
class Factory {
  static void SetLogLevel(std::ostream& out, uint8_t const level);
}
```

