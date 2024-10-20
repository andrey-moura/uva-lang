# The uva programming language

Uva is an interpreted object-oriented multi-purpose programming language

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