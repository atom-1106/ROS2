# gRPC Fallback Plan

## Things to know
- The gRPC original work code is back in the middleware-cpp project.
- It is currently disabled because it will not compile.
- The gRPC code is based on a variation of Middleware Framework that has changed but the core concept code is still there. 
- Domains in Fast DDS .vs gRPC
  - For Fast DDS, domain info was given in the form of a data owner name, topic name, and domain ID while gRPC was a data owner name, ip, and port.
  - Before it was removed, gRPC was only supporting client requests and had not supported simulated pub/sub for a long time. So that code is scatter across personal spaced repos. Pub/sub on gRPC was originally just an open stream that the server published data to on-change with the client side call remains open until server lost or cancelled. An example of this can be seen with [ListFeatures](https://github.com/grpc/grpc/blob/master/examples/cpp/route_guide/route_guide_callback_client.cc)
- The middleware-framework.proto does not reflect the proto used at the time of gRPC removal, it is a re-capture of the request + pub/sub stream behavior from early prototyping days. So the existing gRPC code does not match the proto file.
- The grpc-a is still in external/ but the hardcoded paths to grpc-a/include and grpc-a/lib have been removed from middleware-cpp cmake root file. It will need re-added.
- The overall cmake variable names have changed since gRPC went away so those will need updated in the grpc source code cmakes.

