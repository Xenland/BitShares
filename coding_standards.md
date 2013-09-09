General Coding Standards
================
  - 3 spaces instead of tabs
  - all c++ headers end in .hpp
  - all c headers end in .h
  - pragma once instead of header guards
  - use private implementation classes for long-lived objects

Example of using private implementation: 

    namespace detail { class_name_impl; }
    class class_name
    {
       public:
          class_name();
          ~class_name();

       private:
          std::unique_ptr<detail::class_name_impl> my;
    };

  - note the formatting of the class, braces, indentions, etc.
  - avoid abreviations as much as possible
  - const std::string& x   (not const string &x)
  - char* y                (not char *y)
  - always use {} for conditionals and loops.
 
Here is an example of proper usage of braces:

      while( true )
      {
      }
      
      try {
      
      }
      catch ( ... ) 
      {
      }
      
      switch( x )
      {
         case a:
            break;
         case b:
         {
            int local = 0;
            break;
         }
         default:
            break;
      }

GUI Coding Standards
--------------------
The following list includes is an example of high-level standard.

  - follow the Qt conventions 
  - ClassName
  - methodName
  - _member_variable
  - local_varaible
  - ClassName.hpp 
  - ClassName.cpp
  - No namespaces unless creating a GUI library




Library Coding Standards
------------------------
All classes within a library are namespaced.

  - follow stdlib and boost naming convention (generally)
  - class_name
  - set_method_name
  - _member_variable
  - local_variable

