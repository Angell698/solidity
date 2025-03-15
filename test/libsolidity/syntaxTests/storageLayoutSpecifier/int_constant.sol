int constant x = -42;
contract C layout at x {}
// ----
// TypeError 6753: (43-44): The base slot of the storage layout evaluates to -42, which is outside the range of type uint256.
