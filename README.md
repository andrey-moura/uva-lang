<p align="center">
    <img src="resources/uva.svg" alt="Alt Text" style="width:200px; height:200px;">
</p>

# The uva programming language

Uva is an interpreted object-oriented multi-purpose programming language

# Availability

Environment | Build
--- | --- |
Ubuntu 20.04 | [![Ubuntu 20.04](https://github.com/andrey-moura/uva-lang/actions/workflows/build-ubuntu-20.04.yml/badge.svg)](https://github.com/andrey-moura/uva-lang/actions/workflows/build-ubuntu-20.04.yml)
Windows Server 2022 | [![Windows Server 2022](https://github.com/andrey-moura/uva-lang/actions/workflows/build-windows-2022.yml/badge.svg)](https://github.com/andrey-moura/uva-lang/actions/workflows/build-windows-2022.yml)

# Building

Linux

```sh
    git clone https://github.com/andrey-moura/uva-lang --recursive
    cd uva-lang
    mkdir build
    cd build/
    cmake ..
    make
```

Optional: Run ```sudo make install``` to install uva

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
