#if defined(_MSC_VER)
    // #pragma message("!!! This function or variable may be unsafe")
    // #pragma message("!!! Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified")
    #pragma warning(disable:5045) // - warning C5045: Compiler will insert Spectre mitigation for memory load if /Qspectre switch specified
#endif
