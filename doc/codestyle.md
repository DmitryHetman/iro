Codestyle   {#codestyle}
=========

iro has strict codestyle and naming conventions that should not be broken when contributing.

1. When naming functions, variables or types prefer camelCase over underscore_naming.
2. All type names begin with an uppercase letter, all variables and functions begin with a lowercase 
letter.
3. Member variables always have a undercore suffix (e.g. id_)
4. Macros are generally not needed (prefer constexpr vars), but if then they should be all uppercase, 
words seperated by undercores and have a IRO_ prefix (e.g. IRO_RANDOM_MACRO)
5. Template parameters are usually just named with one uppercase letter (e.g. T)
6. Functions should be named as short as possible and make use of overloading.
Instead of get and set methods just use 2 overloads for the same function.

````
nytl::vec2i position() const;
void position(const nytl::vec2i& pos);
````

7. All object that have a templated type or are larger than an int should always be passed per
const reference (or referecne/pointer)
8. When using short terms like EGL, GL or KMS in naming, just capitalize the first letter (e.g.
EglContext instead of EGLContext)
9. When using objects of an external backend, prefer prefixing the variables with the backend name.
(e.g. wlDisplay instead of just display)
