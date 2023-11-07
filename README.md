Tested with sqlite 3.44.0 and 3.42.0

Output:

    ./go.sh
    sqlite_version() = 3.44.0
    
    **** Query results with no index: ****
    name = apple
    
    name = orange
    
    name = grape
    
    name = watermelon
    
    **** Query results with index: ****
    Unknown subtype: 0
    repro: repro.c:57: hasSeedsFunc: Assertion `0' failed.
    Aborted (core dumped)
