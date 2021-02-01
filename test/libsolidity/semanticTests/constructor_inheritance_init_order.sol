contract A {
    uint x;
    constructor() {
        x = 42;
    }
    function f() public returns(uint256) {
        return x;
    }
}
contract B is A {
    uint public y = f();
}
// ====
// compileToEwasm: also
// compileViaYul: true
// ----
// constructor() ->
// gas ir: 230700
// gas irOptimized: 152439
// y() -> 42
