<p align="center">
    <img src="resources/uva.svg" alt="Alt Text" style="width:200px; height:200px;">
</p>

# The uva programming language

Uva is an interpreted object-oriented multi-purpose programming language

# Availability

Environment | Build
--- | --- |
Ubuntu 20.04 | [![Ubuntu 20.04](https://github.com/andrey-moura/uva-lang/actions/workflows/build-ubuntu-20.04.yml/badge.svg)](https://github.com/andrey-moura/uva-lang/actions/workflows/build-ubuntu-20.04.yml)

# Building

### Linux

```sh
    git clone https://github.com/andrey-moura/uva-lang --recursive
    cd uva-lang
    mkdir build
    cd build/
    cmake ..
    make
```

Optional: Run ```sudo make install``` to install uva.exe

## Windows

### From VS Code Or Visual Studio
Simply open the project with it and hit build.

### From Developer Command Prompt for VS

```sh
    git clone https://github.com/andrey-moura/uva-lang --recursive
    cd uva-lang
    mkdir build
    cd build/
    cmake ..
    MSBuild.exe ALL_BUILD.vcxproj
```

Optional: Run (as an administrator) ```MSBuild.exe INSTALL.vcxproj``` to install uva

# Samples

If you want to run a sample, try:

```sh
    uva samples/hellow_world.uva
```

This file has the content:

```typescript
    class Application
    {
        function run()
        {
            puts("Hello, World!");
        }
    }
```

The result is:

```
    Hello, World!
```
