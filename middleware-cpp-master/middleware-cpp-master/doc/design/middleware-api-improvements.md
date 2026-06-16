# Middleware API Improvements <!-- omit in toc -->

See [Middleware API Issues](./middleware-api-issues.md) for a discussion of pain points.

- [Motivation](#motivation)
- [1. Template: \<Improvement Title\>](#1-template-improvement-title)
  - [1.1. Description](#11-description)
  - [1.2. Customer Code Changes](#12-customer-code-changes)
  - [1.3. Backwards Compatibility Possibilities](#13-backwards-compatibility-possibilities)
  - [1.4. Backwards Compatibility Decision](#14-backwards-compatibility-decision)
  - [1.5. Automatic Refactoring When Backwards Compatibility is Not Achieved \<Remove if BC is Achieved\>](#15-automatic-refactoring-when-backwards-compatibility-is-not-achieved-remove-if-bc-is-achieved)
- [2. Fine-grained Logging Control](#2-fine-grained-logging-control)
  - [2.1. Description](#21-description)
  - [2.2. Customer Code Changes](#22-customer-code-changes)
  - [2.3. Backwards Compatibility Possibilities](#23-backwards-compatibility-possibilities)
  - [2.4. Backwards Compatibility Decision](#24-backwards-compatibility-decision)

## Motivation

Now that we and our customers have been using the Middleware 1.0.0 API extensively, we have observed multiple areas where it could be improved for clarity, usability, and maintainability.

This API is our constant "20-year" API so all changes we make to it must be
1. A true improvement for the users
2. Very deliberate where all backwards compatibility breaks (either in the API or on the wire) are clearly understood, documented, and approved before moving forward with the changes

>>> [!caution]
All breaking changes must be clearly communicated to our users and, in fact, over-communicated. If there is a breaking API change that is not backwards compatible, it would be a good idea to provide a tool that would automatically refactor the user's code to make it compliant with the new API.
>>>

## 1. Template: \<Improvement Title\>
All improvements should follow this template:
### 1.1. Description
- *Subject Matter Expert*: 
- *Backwards Compatible*: Yes/No
- *Scripted Refactoring*: Yes/No/NA
- *Approved By*: 
- *Approval Date*:
- *Pain Description Link*: [\<Issues Section Name\>](./middleware-api-issues.md#heading-goes-here)
- *Story Link*:

Description of the change with code snippets.

Justification for the improvement for the client.

### 1.2. Customer Code Changes
Show before and after code snippets showing exactly what customers will need to do to utilize this improvement.

### 1.3. Backwards Compatibility Possibilities
Describe possible ways this change could be made backwards-compatible.

### 1.4. Backwards Compatibility Decision
Describe the decision about how it will be made backwards compatible or why it will not be backwards compatible.

### 1.5. Automatic Refactoring When Backwards Compatibility is Not Achieved \<Remove if BC is Achieved\>
If this improvement is not backwards compatible, will automatic refactoring via scripting be implemented? How will it work?
If it will not be implemented, why not?

## 2. Fine-grained Logging Control

### 2.1. Description
- *Subject Matter Expert*: Steve Schrock (@schros1)
- *Backwards Compatible*: Yes
- *Scripted Refactoring*: NA
- *Approved By*: Roger Owdom (@owdomrd)
- *Approval Date*: 3/13/2026
- *Pain Description Link*: [Logging](./middleware-api-issues.md#logging)
- *Story Link*: [32767](https://dev.azure.com/cat-ciss/G7/_workitems/edit/32767)

We should allow the client to register a logging callback that will receive the string to log as well as a severity level enum. This mechanism follows well-established industry patterns like [SQLite](https://sqlite.org/errlog.html). This will give the client fine-grained control over log message formatting and distribution so the client can easily integrate Middleware log messages into their own logging framework.

Example usage:
```cpp
void MyLogger(middleware::LogLevel logLevel, std::string const& message)
{
  if (logLevel >= middleware::LogLevel::Warning)
  {
    Log("Middleware: " + message);
  }
}

middleware::RegisterLoggingCallback([](middleware::LogLevel logLevel, std::string const& message) { MyLogger(logLevel, message); });
```

### 2.2. Customer Code Changes
Remove all calls to `middleware::parameter::Factory::SetLogLevel`. Instead implement a callback and pass it to `middleware::RegisterLoggingCallback`.

### 2.3. Backwards Compatibility Possibilities
We can keep the `middleware::parameter::Factory::SetLogLevel`. It will call `middleware::RegisterLoggingCallback` with its own internal callback that utilizes the `std::ostream` the caller passes into that old method.

### 2.4. Backwards Compatibility Decision
We will maintain backwards compatibility, but add comments to the old method that it is deprecated and point to the new API.
