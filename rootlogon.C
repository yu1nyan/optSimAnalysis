

int loadMacro( const char *macro )
{
    printf("Load macro '%s'.\n", macro);
    const int ret = gROOT->LoadMacro( macro );
    if( ret != 0 ) {
        fprintf(stderr, "Error:\t Failed to load library '%s' : ret = %d\n", macro, ret);
        exit(1);
    }
    return 0;
}

int rootlogon()
{
    loadMacro("./optSimAnalysis.cc");
    printf("optSimAnalysis.cc is loaded.\n");
    return 0;
}
