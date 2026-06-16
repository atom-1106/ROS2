# Mapper

A parameter mapping application that subscribes to input parameters and republishes them under a different name.

## Inputs

Subscribes to the following parameters:
- `A` - sent by tests
- `machine.fuel_level_1` - sent by Jaws

## Outputs

Republishes received parameter data as:
- `B`

The application continuously monitors the input parameters and republishes any updates to the output parameter.
