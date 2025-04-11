contract C {
    function getArray() internal returns (uint256[10][1] storage x) {
        assembly {
            x.slot := sub(0, 1)
        }
    }
    function f() public returns (uint256[10] memory) {
        uint256[10][1] storage x = getArray();
        for (uint i = 0; i < 10; i++)
            x[0][i] = i;
        x[0] = [11, 12, 13];
        return x[0];
    }
}
// ----
// f() -> 11, 12, 13, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 198025
// gas legacy: 200268
// gas legacyOptimized: 198058
