contract A {
    uint constant INHERITED = 1;
}
contract C is A {
    uint constant CONST = 2 + A.INHERITED;
    uint[CONST] array;
}
// ----