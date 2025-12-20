# meta.h

Contact me at johnagrillo@yahoo.com if you find any if this useful.

NOte: The ideas and concepts of meta.h are my own work.  I stared off
with basic ideas of template recursion and the primitive terminal
principle but was quickly overwhelmend by the complexites of template
progamming.  This was completed with many hours of interactions with
Claude AI.

I found the idea of contexpr meta tuples some where in the c++ spec documentation
and worked with that idea.

**Compile-time reflection/serialization/deserialization using constexpr member
pointers and fully compile time code gen.**

meta_h is a compile time ast walker with a visitor pattern. uses std::aply, no run time loops.
All code is generated at compile time using template recursion and the terminal primitive principle.

### Terminal Primitive Principle

If you had reflection this is what it woud like like.

All data structures are trees with two types of nodes:

**Terminal nodes** (primitives - recursion stops):
- int, float, bool, char
- std::string
- Enum types

**Non-terminal nodes** (containers - recurse deeper):
- struct
- std::vector, std::map, std::pair, std::tuple
- std::optional, std::variant

When deserializing:

```cpp
struct Person {
    std::string name;              // Terminal - int, string, etc
    int age;                       // Terminal
    std::vector<std::string> hobbies;  // Non-terminal - recurse into vector
    Address address;               // Non-terminal - recurse into Address struct
};

// Compiler walks the tree:
// Person → name (terminal, stop) 
//        → age (terminal, stop)
//        → hobbies (non-terminal, recurse) 
//           → std::string (terminal, stop) ✓
//        → address (non-terminal, recurse)
//           → street (terminal, stop)
//           → city (terminal, stop)
```

Template recursion naturally terminates at primitives. Works for any nesting depth.

This is how C++26 will do reflection. It's already possible in C++20.



I discovered constexpr member pointers enable compile-time struct introspection.
meta.h was born from that discovery.

C++20 serialization framework. Single header file.   We need to add constexpr tuples to enable it.
In c++ 26 the compiler wil do it.  But for now it gets pretty close with minimal bolier plate.

Define a struct once, serialize/deserialize to YAML, JSON, XML, CSV, 
and could easily be extended to Protobuf automatically.

Pure C++20. No macros. No code generation. Minimal boilerplate.
One header file.

## Simple Example

```cpp
struct Person {
    std::string name;
    int age;
    
    // Compile-time annotations - describes struct for reflection
    static constexpr auto fields = std::make_tuple(
        meta::Field<&Person::name>("name"),     // Annotate: name field
        meta::Field<&Person::age>("age")        // Annotate: age field
    );
};

Person p{"Alice", 28};
std::cout << meta::toYaml(p);
```

Output:
```yaml
name: Alice
age: 28
```

Or JSON:
```json
{"name":"Alice","age":28}
```

Or XML:
```xml
<Person>
  <name>Alice</name>
  <age>28</age>
</Person>
```

## Nested Structures

No special handling needed:

```cpp
struct Address {
    std::string street;
    std::string city;
    
    // Compile-time annotations
    static constexpr auto fields = std::make_tuple(
        meta::Field<&Address::street>("street"),
        meta::Field<&Address::city>("city")
    );
};

struct Person {
    std::string name;
    int age;
    Address address;
    
    // Compile-time annotations - nesting handled automatically
    static constexpr auto fields = std::make_tuple(
        meta::Field<&Person::name>("name"),
        meta::Field<&Person::age>("age"),
        meta::Field<&Person::address>("address")  // Just add the nested field
    );
};

Person p{"John", 30, {"123 Main St", "Boston"}};
std::cout << meta::toYaml(p);
```

Output:
```yaml
name: John
age: 30
address:
  street: 123 Main St
  city: Boston
```

Works for all formats automatically.

## Maps of Maps (Hash of Hash)

Complex nested structures work too:

```cpp
struct Config {
    std::map<std::string, std::map<std::string, std::string>> settings;
    
    // Compile-time annotations - works with any complexity
    static constexpr auto fields = std::make_tuple(
        meta::Field<&Config::settings>("settings")  // One annotation for map of map
    );
};

Config cfg;
cfg.settings["database"]["host"] = "localhost";
cfg.settings["database"]["port"] = "5432";
cfg.settings["cache"]["ttl"] = "3600";

std::cout << meta::toYaml(cfg);
```

Output:
```yaml
settings:
  database:
    host: localhost
    port: "5432"
  cache:
    ttl: "3600"
```

Arbitrary nesting depth. Works for all formats.

## Deserialization

Read from any format, get back typed object:

```cpp
std::string yaml = R"(
name: Bob
age: 35
address:
  street: 456 Oak Ave
  city: New York
)";

auto [person, result] = meta::reifyFromYaml<Person>(yaml);
if (result.valid) {
    std::cout << person->name << " " << person->age << "\n";
}
```

Works for JSON, XML, CSV, and could be extended to Protobuf as well.

## Features

- Multiple formats (YAML, JSON, XML, CSV)
- Nested structures automatic
- Single header file
- Pure C++20 (no macros)
- Validation built-in (BoundedInt, BoundedString, Whitelist)
- No code generation
- Minimal boilerplate
- Compile-time safe
- Zero overhead

## Dependencies

- C++20 compiler (g++ 10+, clang 13+)
- libyaml (for YAML support)

```bash
# macOS
brew install yaml-cpp

# Ubuntu/Debian
sudo apt-get install libyaml-cpp-dev
```

## Quick Start

```bash
# Install libyaml
brew install yaml-cpp          # macOS
sudo apt install libyaml-cpp-dev  # Ubuntu/Debian

# Compile all examples
make

# Run all examples
make run
```

Annotate each field once. Everything else is automatic.

Serialization and deserialization for YAML, JSON, XML, CSV, and can be extended to Protobuf.

## Examples Included

18 examples demonstrating(by ClaudeAI)
- Simple structs and nesting
- Containers (vectors, maps, pairs, tuples)
- Validation (BoundedInt, BoundedString, Whitelist)
- CSV, YAML, XML output
- Enums
- Complex nested structures
- Runtime containers
- Equality testing

All compile instantly. All use the same pattern: annotate fields once.

No more work than BOOST_DESCRIBE or nlohmann. 

## Build

```bash
make
make run
```

That's it. Compiles all examples and runs them.

## How It Works

The framework uses compile-time tuple metadata to describe struct fields. At compile-time, the compiler generates exact deserialization code for each type.

No runtime reflection. No registration. No code generation tools.

Just C++20 templates doing what they're designed to do.

### Constexpr Member Pointers

This framework is built on a pattern that C++26 will standardize for reflection.

**Constexpr member pointers** enable compile-time struct introspection:

```cpp
meta::Field<&Person::name>("name")  // Member pointer at compile-time
```

The compiler knows the exact location and type of each field. It generates 
exact serialization/deserialization code.

### Terminal/Non-Terminal Recursion

All data structures reduce to two categories:

- **Terminal nodes** (primitives): int, string, bool, double - recursion stops
- **Non-terminal nodes** (containers): struct, vector, map - recurse through children

Template recursion naturally terminates when it hits a primitive. This pattern 
works for any nesting depth automatically.

### The Asymmetry Principle

Why this works without code generation tools:

- Data arrives at **runtime** (raw bytes, unknown content)
- Type is known at **compile-time** (template parameter)

This asymmetry is key. The compiler generates exact code for the type it knows 
about, even though it's deserializing runtime data.

Traditional frameworks need code generation because they don't know the type 
at compile-time. meta.h solves it by knowing the type upfront.

### Terminal Primitive Principle

All data structures are trees with two types of nodes:

**Terminal nodes** (primitives - recursion stops):
- int, float, bool, char
- std::string
- Enum types

**Non-terminal nodes** (containers - recurse deeper):
- struct
- std::vector, std::map, std::pair, std::tuple
- std::optional, std::variant

When deserializing:

```cpp
struct Person {
    std::string name;              // Terminal - int, string, etc
    int age;                       // Terminal
    std::vector<std::string> hobbies;  // Non-terminal - recurse into vector
    Address address;               // Non-terminal - recurse into Address struct
};

// Compiler walks the tree:
// Person → name (terminal, stop) 
//        → age (terminal, stop)
//        → hobbies (non-terminal, recurse) 
//           → std::string (terminal, stop) ✓
//        → address (non-terminal, recurse)
//           → street (terminal, stop)
//           → city (terminal, stop)
```

Template recursion naturally terminates at primitives. Works for any nesting depth.

This is why code generation isn't needed - the recursion pattern handles everything.

The real magic: template recursion generates both serialization and deserialization 
code at compile-time.

When you call `meta::toYaml(person)`, the compiler:
1. Sees the Person type
2. Recursively walks through fields tuple
3. For each field: if primitive, generate serialize code; if container, recurse
4. Generates exact, specialized code for Person

When you call `meta::reifyFromYaml<Person>(yaml)`, the compiler:
1. Knows the type is Person (template parameter)
2. Recursively walks through fields tuple
3. For each field: if primitive, generate deserialize code; if container, recurse
4. Generates exact, specialized code for Person

**No runtime dispatch. No reflection. Just compile-time code generation.**

The recursion naturally terminates at primitives (terminal nodes). Container 
types recurse deeper. Any nesting depth works automatically.

This is why you don't need code generation tools. The compiler does it.

### C++26 Alignment

This pattern is how C++26 will do reflection. You can use it now with C++20.

## License

none - free to share

# meta_h
#   m e t a _ h  
 #   m e t a _ h  
 