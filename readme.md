# Coherent Presentation (C++11)

This example demonstrates that you can access information, from the sample metadata, indicating to which unique coherent set any individual sample belongs.

Before this example can be compiled and run, rtiddsgen should be run to generate type support code from the \*.idl file. When performing this step, be sure to set the following:
- Generation: Example Files = "<disable>"
- Generation: Type files = "update"
- Generation: Makefiles = "create"
- Language = "Modern C++ (C++ 11)"

This combination will preserve the publisher and subscriber application source while creating makefiles for your host platform, as well as the type support source files required by the middleware.

## Building the project

On POSIX operating systems such as Linux or MacOS, call `make` with the generated Makefile as an argument.
```
$ make -f makefile_example_x64Darwin17clang9.0
```

## Running the project

Open two shell sessions. Start the publisher in one shell and the subscriber in the second shell.

```
$ ./objs/x64Darwin17clang9.0/example_publisher
```

```
$ ./objs/x64Darwin17clang9.0/example_subscriber
```
