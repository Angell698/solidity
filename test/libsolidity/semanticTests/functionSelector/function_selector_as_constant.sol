contract B {
    function g() public {}
}

contract C is B {
    bytes4 public constant s2 = B.g.selector;
}


// ----
// s2() -> 0xe2179b8e00000000000000000000000000000000000000000000000000000000
