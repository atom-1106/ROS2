# Copyright (C) Caterpillar Inc. All Rights Reserved.

Feature: Middleware Parameter Service
  This is the "as-is" high level requirements of the Middleware Parameter Service API. 
  These requirements are a general agreement at the time of prototyping and are in no way 
  solidified or have had in depth client review. All requirements listed are subject to change.

  # ParameterService
  Scenario: Middleware Parameter Server does not initialize if the configured read and write actions are empty
    Given the Middleware Parameter Service configuration field "server_write_parameters" is empty
      And the Middleware Parameter Service configuration field "server_read_parameters" is empty
     When the Middleware Parameter Service is created with the configuration
     Then the Parameter Server will not be initialized

  Scenario: Middleware Parameter Server starts on Service construction with only Write Parameters
    Given the Middleware Parameter Service configuration field "server_write_parameters" is not empty
      And the Middleware Parameter Service configuration field "server_read_parameters" is empty
     When the Middleware Parameter Service is created with the configuration
     Then the Parameter Server will start

  Scenario: Middleware Parameter Server starts on Service construction with only Read Parameters
    Given the Middleware Parameter Service configuration field "server_write_parameters" is empty
      And the Middleware Parameter Service configuration field "server_read_parameters" is not empty
     When the Middleware Parameter Service is created with the configuration
     Then the Parameter Server will start

  Scenario: Middleware Parameter Server starts on Service construction with both Read and Write Parameters
    Given the Middleware Parameter Service configuration field "server_write_parameters" is not empty
      And the Middleware Parameter Service configuration field "server_read_parameters" is not empty
     When the Middleware Parameter Service is created with the configuration
     Then the Parameter Server will start

  Scenario: Middleware Parameter Publisher throws when the identity can not be found
    Given the Middleware Parameter Service has been created with a configuration
     When the Middleware Parameter Service call "RetrievePublisherInstance" is passed an identity not in the configuration
     Then the call will throw

  Scenario: Middleware Parameter Read throws when the identity can not be found
    Given the Middleware Parameter Service has been created with a configuration
     When the Middleware Parameter Service call "RetrieveReadInstance" is passed an identity not in the configuration
     Then the call will throw

  Scenario: Middleware Parameter Write throws when the identity can not be found
    Given the Middleware Parameter Service has been created with a configuration
     When the Middleware Parameter Service call "RetrieveWriteInstance" is passed an identity not in the configuration
     Then the call will throw

  # ParameterServer
  Scenario: Middleware Parameter Server initializes when passed a configuration
    Given the Middleware Parameter Server has been created with a configuration
     When the Middleware Parameter Server initializes
     Then each write parameter entry will be cached by identity with their action callback
      And each read parameter entry will be cached by identity with their action callback

  Scenario: Middleware Parameter Server uses the cached On Read callback for data when Read Request is received
    Given the Middleware Parameter Server is running
     When a client Read Request is received
     Then the Middleware Parameter Server will call the correct On Read callback for the requested data
      And will assign the data read to the response

  Scenario: Middleware Parameter Server uses the cached On Write callback for data when Write Request is received
    Given the Middleware Parameter Server is running
     When a client Write Request is received
     Then the Middleware Parameter Server will call the correct On Write callback for the requested data
      And will assign the data written to the response

  Scenario: Middleware Parameter Server returns the request with NOT_FOUND when the requested identity callback is not found
    Given the Middleware Parameter Server is running
     When a client Request is received for an identity not in the cached callback actions
     Then the Middleware Parameter Server will assign the original request to the data with status NOT_FOUND

  Scenario: Middleware Parameter Server returns the request with TIMEOUT_ERROR when the requested identity takes longer than 5 seconds to process
    Given the Middleware Parameter Server is running
      And a client Request is received for an owned identity
     When the Middleware Parameter Server takes longer than 5 seconds to process the request
     Then the Middleware Parameter Server will assign the original request to the data with status TIMEOUT_ERROR

  # PublishParameter
  Scenario: Middleware Parameter Publisher publishes initial data on construction
    Given the Middleware Parameter Service has been created with a configuration
     When a Publisher is constructed
     Then the configured initial data is published

  Scenario: Middleware Parameter Publisher logs publishing failures
    Given the Middleware Parameter Service is created with usable publishers
     When a retrieved publisher calls "Publish" that fails internally
     Then the Publisher will log a message

  # SubscriberParameter
  Scenario: Middleware Parameter Subscriber starts subscription on construction
    Given the Middleware Parameter Service has been created with a configuration
     When a Subscriber is constructed
     Then the subscription callback is registered
      And the disconnect callback is registered

  Scenario: Middleware Parameter Subscriber assigns QUALITY_BAD, calls the subscription callback, and calls configured disconnect callback when disconnected from the Publisher
    Given the Middleware Parameter Service is created with usable subscribers
     When a Publisher disconnects from the Subscriber
     Then the current cached data is assigned QUALITY_BAD
      And the subscription callback is called
      And the client configured disconnect callback is called

  # ReadParameter
  # [Note] Errors have not been discussed or agreed to yet, so errors are at developer discretion
  Scenario: Middleware Parameter Read callback is called with error data on Read failure
    Given the Middleware Parameter Service is created with usable readers
     When a retrieved reader calls "Read" that fails
     Then the data quality is assigned QUALITY_NOT_RECEIVED
      And the on_complete callback is called with data reflecting the error

  Scenario: Middleware Parameter Read receives a response with an unexpected identity
    Given the Middleware Parameter Service is created with usable readers
      And a retrieved reader has called "Read"
     When a response is received with the incorrect identity
     Then the data quality is assigned QUALITY_NOT_RECEIVED
      And the on_complete callback is called with data reflecting the error

  Scenario: Middleware Parameter Read receives a response with data that can not be parsed
    Given the Middleware Parameter Service is created with usable readers
      And a retrieved reader has called "Read"
     When a response is received with data that can not be parsed into protobuf message
     Then the data quality is assigned QUALITY_BAD
      And the on_complete callback is called with data reflecting the error

  Scenario: Middleware Parameter Read receives a response with valid data
    Given the Middleware Parameter Service is created with usable readers
      And a retrieved reader has called "Read"
     When a response is received with the valid data
     Then the on_complete callback is called with the responding data

  # WriteParameter
  # [Note] Errors have not been discussed or agreed to yet, so errors are at developer discretion
  Scenario: Middleware Parameter Write callback is called with error data on Read failure
    Given the Middleware Parameter Service is created with usable writers
     When a retrieved writer calls "Write" that fails
     Then the data quality is assigned QUALITY_NOT_RECEIVED
     And the on_complete callback is called with data reflecting the error

  Scenario: Middleware Parameter Write receives a response with an unexpected identity
    Given the Middleware Parameter Service is created with usable writers
      And a retrieved writer has called "Write"
     When a response is received with the incorrect identity
     Then the data quality is assigned QUALITY_NOT_RECEIVED
      And the on_complete callback is called with data reflecting the error

  Scenario: Middleware Parameter Write receives a response with data that can not be parsed
    Given the Middleware Parameter Service is created with usable writers
      And a retrieved writer has called "Write"
     When a response is received with data that can not be parsed into protobuf message
     Then the data quality is assigned QUALITY_BAD
      And the on_complete callback is called with data reflecting the error

  Scenario: Middleware Parameter Write receives a response with valid data
    Given the Middleware Parameter Service is created with usable writers
      And a retrieved writer has called "Write"
     When a response is received with the valid data
     Then the on_complete callback is called with the responding data