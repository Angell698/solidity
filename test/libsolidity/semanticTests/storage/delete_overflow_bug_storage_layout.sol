contract C layout at 2**256 - 2 {
    uint256 a;

    function getArray() internal returns (uint256[10][1] storage x) {
        assembly {
            x.slot := a.slot
        }
    }

    function f() public returns (uint256[10] memory) {
        uint256[10][1] storage x = getArray();
        for (uint i = 0; i < 10; i++)
            x[0][i] = i;
        delete x[0];
        return x[0];
    }
}
// ----
// f() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 181930
// gas legacy: 184143
// gas legacyOptimized: 181900
