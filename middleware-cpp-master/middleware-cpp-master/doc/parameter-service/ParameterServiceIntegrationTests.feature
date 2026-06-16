# Copyright (C) Caterpillar Inc. All Rights Reserved.

Feature: Middleware Parameter Service Integration Tests
  The automated integration test samples to demonstrate the functionality of
  Middleware Parameter Service API on GitLab Pipeline builds.
  Not a direct interpretation of the software tests but the scenarios are the expectation. 

  Background:
    Given I have an application called "Jones" using Middleware Parameter Service with parameters
      | Parameter | Value     | Relation      |
      | State     | Illinois  | Owned         |
      | Town      | Mossville | Owned         |
      | Team      | none      | Subscription  |
      | Genre     | none      | Subscription  |
      | Color     | none      | Client::Write |
      | Movie     | none      | Client::Read  |
      | Date      | none      | Client::Read  |
    And I have an application called "Mike" using Middleware Parameter Service with parameters
      | Parameter | Value | Relation      |
      | Team      | Bears | Owned         |
      | Color     | Brown | Owned         |
      | Genre     | none  | Subscription  |
      | Town      | none  | Subscription  |
      | State     | none  | Client::Read  |
      | Movie     | none  | Client::Write |
      | Date      | none  | Client::Read  |
    And I have an application called "Freddy" using Middleware Parameter Service with parameters
      | Parameter | Value     | Relation      |
      | Movie     | Insidious | Owned         |
      | Genre     | Horror    | Owned         |
      | Color     | none      | Subscription  |
      | State     | none      | Subscription  |
      | Team      | none      | Client::Read  |
      | Town      | none      | Client::Write |
      | Date      | none      | Client::Read  |
    And I have an application called "Clock" using Middleware Parameter Service with parameters
      | Parameter | Value     | Relation |
      | Date      | 1-10-2025 | Owned    |


  Scenario Outline: Publisher can send data of 4MB or less successfully
    Given application "Jones" is running
    And application "Mike" is running
    When application "Jones" publishes a protobuf message with a total size of <size> <label> for Parameter "Town"
    Then application B will receive a subscription callback for Parameter "Town"
    Examples:
      | size | label |
      | 255  | Bytes |
      | 10   | KB    |
      | 2    | MB    |
      | 4    | MB    |


  Scenario: Publisher cannot send data over 4MB
    Given application "Jones" is running
    And application "Mike" is running
    When application "Jones" publishes a protobuf message with a total size of 4000001 Bytes for Parameter "Town"
    Then application "Jones" will receive a publish failure
    And application "Mike" will not receive a subscription callback for Parameter "Any"


  Scenario: Publisher disconnect sets parameter quality QUALITY_BAD
    Given application "Jones" is running
    And application "Mike" is running
    When application "Jones" is stopped
    Then application B will receive a subscription callback for Parameter "Town" with QUALITY_BAD
    And application B will receive a disconnect callback for Parameter "Town"


  Scenario Outline: Client calls to a server that is not available returns SERVER_NOT_AVAILABLE
    Given application "Mike" is running
    When application B makes a client <call> request for Parameter "<name>"
    Then application B will received an on_complete callback with SERVER_NOT_AVAILABLE
    Examples:
      | name  | call  |
      | State | read  |
      | Movie | write |


  Scenario: Client read request to a server that is available returns the requested parameters value
    Given application "Jones" is running
    And application "Mike" is running
    When application "Mike" makes a client read request for Parameter "State"
    Then application "Mike" will receive an on_complete callback success with the parameter's value of "Illinois"


  Scenario: Client write request to a server that is available returns the requested parameters written value
    Given application "Jones" is running
    And application "Mike" is running
    When application "Mike" makes a client write request for Parameter "Movie" with value "Chucky"
    Then application "Mike" will receive an on_complete callback success with the parameter's value of "Chucky"


  Scenario: Client requests to multiple servers will be successful
    Given application "Jones" is running
    And application "Mike" is running
    And application "Freddy" is running
    And application "Clock" is running
    And application "Mike" makes a client read request for Parameter "State"
    And application "Mike" makes a client write request for Parameter "Movie" with value "Megan"
    When application "Mike" makes a client read request for Parameter "Date"
    Then application "Mike" will receive an on_complete callback success with the parameter's value of "Illinois"
    And application "Mike" will receive an on_complete callback success with the parameter's value of "Megan"
    And application "Mike" will receive an on_complete callback success with the parameter's value of "1-10-2025"


  Scenario: Server receiving multiple requests from multiple clients will be successful
    Given application "Jones" is running
    And application "Mike" is running
    And application "Freddy" is running
    And application "Clock" is running
    And application "Jones" makes a client read request for Parameter "Date"
    And application "Mike" makes a client read request for Parameter "Date"
    When application "Freddy" makes a client read request for Parameter "Date"
    Then application "Jones" will receive an on_complete callback success with the parameter's value of "1-10-2025"
    And application "Mike" will receive an on_complete callback success with the parameter's value of "1-10-2025"
    And application "Freddy" will receive an on_complete callback success with the parameter's value of "1-10-2025"
