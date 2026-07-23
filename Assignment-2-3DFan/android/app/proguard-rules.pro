# Native methods are referenced by MainActivity and must keep their names.
-keepclasseswithmembernames class * {
    native <methods>;
}
