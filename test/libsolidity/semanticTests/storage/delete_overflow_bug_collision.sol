contract C {
    uint256 y = 42;

    function getArray() internal returns (uint256[10][1] storage x) {
        assembly {
            x.slot := sub(0, 1)
        }
    }

    function f() public returns (uint256[10] memory) {
        uint256[10][1] storage x = getArray();
        for (uint i = 0; i < 10; i++)
            x[0][i] = i;
        delete x[0];
        return x[0];
    }

    function g() public view returns (uint256) {
        return y;
    }
}

// ----
// g() -> 42
// f() -> 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// gas irOptimized: 168243
// gas legacy: 170463
// gas legacyOptimized: 168219
// g() -> 0
