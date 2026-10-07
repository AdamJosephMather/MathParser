# A Math Parser in C++

Example usage:

```cpp
MathParser mp = MathParser();
auto state = mp.setup();

auto res = mp.RunLine("a = 1-3*cos(2*pi)", state);

if (res.worked) {
  std::cout << res.value << "\n";  // -2
}else{
  std::cout << res.error_msg << "\n";
}

res = mp.RunLine("2*a", state);

if (res.worked) {
  std::cout << res.value << "\n";  // -4
}else{
  std::cout << res.error_msg << "\n";
}
```

The MathParser is designed to work with persistent variables as well as an array of constants and built in math functions. Ex:

```
g         # 9.80665
a = 2*g   # 19.6133
g = a+1   # 20.6133
g         # 20.6133

sqrt(16)  # 4
```
