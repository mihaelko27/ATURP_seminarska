build:
    mkdir -p build
    g++ main.cpp -O3 -o ./build/a.out

clean:
    rm -rf build

run arg1:
    just build
    ./build/a.out {{arg1}}

test:
    g++ main.cpp -O3 -o ./build/test.out
    ./build/test.out --test


