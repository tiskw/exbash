////////////////////////////////////////////////////////////////////////////////////////////////////
/// C++ source file: logo.cxx                                                                    ///
////////////////////////////////////////////////////////////////////////////////////////////////////

// Include the primary header.
#include "logo.hxx"

// Include STL headers.
#include <iostream>

// Include the headers of custom modules.
#include "utils.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
// File-local functions
////////////////////////////////////////////////////////////////////////////////////////////////////

// Unnamed namespace for making classes and functions file-local.
namespace
{
    constexpr char logo_template[] =
    "(C1)EEEEEEEEEEEEEEEEE (C2)                  (C3)BBBBBBBBBBBBBB   (C4)                 (C5)               (C6)hhhhh          (C0)\n"
    "(C1)E:::::::::::::::E (C2)                  (C3)B:::::::::::::B  (C4)                 (C5)               (C6)h:::h          (C0)\n"
    "(C1)EE::::EEEEEEE:::E (C2)xxxxxx     xxxxxx (C3)B::::BBBBBB::::B (C4)   aaaaaaaaaaa   (C5)   sssssssss   (C6)h:::h          (C0)\n"
    "(C1)  E:::E     EEEEE (C2) x::::x   x::::x  (C3)BB:::B     B::::B(C4)   a::::::::::a  (C5) ss:::::::::s  (C6)h:::h          (C0)\n"
    "(C1)  E:::E           (C2)  x::::x x::::x   (C3) B::B     B::::B (C4)   aaaaaaa::::a  (C5)s:::::ssss:::s (C6)h:::h hhhhh    (C0)\n"
    "(C1)  E::::EEEEEEE    (C2)   x::::x::::x    (C3) B::BBBBBB::::B  (C4)          a:::a  (C5) s::::s   sss  (C6)h:::hh::::hhh  (C0)\n"
    "(C1)  E::::::::::E    (C2)    x:::::::x     (C3) B::::::::::BB   (C4)    aaaaaa::::a  (C5)   s::::s      (C6)h::::::::::::h (C0)\n"
    "(C1)  E::::EEEEEEE    (C2)     x:::::x      (C3) B::BBBBBB::::B  (C4)  aa::::::::::a  (C5)    s:::::s    (C6)h:::::hhh::::h (C0)\n"
    "(C1)  E:::E           (C2)    x:::::::x     (C3) B::B     B::::B (C4) a:::aaaa:::::a  (C5)      s:::::s  (C6)h::::h   h::::h(C0)\n"
    "(C1)  E:::E     EEEEE (C2)   x::::x::::x    (C3) B::B     B::::B (C4)a:::a    a::::a  (C5) sss    s::::s (C6)h:::h     h:::h(C0)\n"
    "(C1)EE::::EEEEEE::::E (C2)  x::::x x::::x   (C3)BB:::BBBBBB:::::B(C4) a::::aaaa:::::a (C5)s:::ssss:::::s (C6)h:::h     h:::h(C0)\n"
    "(C1)E:::::::::::::::E (C2) x::::x   x::::x  (C3)B::::::::::::::B (C4)  a::::::::aa:::a(C5) s:::::::::ss  (C6)h:::h     h:::h(C0)\n"
    "(C1)EEEEEEEEEEEEEEEEE (C2)xxxxxx     xxxxxx (C3)BBBBBBBBBBBBBB   (C4)   aaaaaaaa  aaaa(C5)  sssssssss    (C6)hhhhh     hhhhh(C0)";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions
////////////////////////////////////////////////////////////////////////////////////////////////////

int32_t print_exbash_logo(void)
{   // {{{

    // Copy the template string.
    String logo = logo_template;

    // Replace color placeholders.
    logo = replace(logo, "(C0)", "\x1B[m");
    logo = replace(logo, "(C1)", "\x1B[38;2;129;162;190m");
    logo = replace(logo, "(C2)", "\x1B[38;2;240;198;116m");
    logo = replace(logo, "(C3)", "\x1B[38;2;178;148;187m");
    logo = replace(logo, "(C4)", "\x1B[38;2;204;102;102m");
    logo = replace(logo, "(C5)", "\x1B[38;2;181;189;104m");
    logo = replace(logo, "(C6)", "\x1B[38;2;138;190;183m");

    // Print to STDOUT.
    std::cout << logo << std::endl;

    return EXIT_SUCCESS;

}   // }}}

// vim: expandtab tabstop=4 shiftwidth=4 fdm=marker
